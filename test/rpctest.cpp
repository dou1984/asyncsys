#include "asys.h"
#include "msg/msg.h"
#include "msg/rpc.h"
#include "msg/rpccli.h"
#include "http/httpserver.h"
#include "mysql/mysqlcli.h"
#include "proto/msg.pb.h"

using namespace google::protobuf;
using namespace ashan;
using trpc = rpc<1, Request, Response>;

void test_rpc()
{
	auto srv = msgserver<int>::make("127.0.0.1:9001");
	srv->dispatch = mydispatch<MSG>::server;

	trpc::add_method(
		srv,
		[](auto s, auto req, auto res)
		{
			LOG_DBG("recv trpc %s", req->request().c_str());
			res->set_response("helloworld response");
			return 0;
		});
	asyncnet::get().wait();
}

void test_rpccc()
{
	auto cli = rpccli::make("127.0.0.1:9001");
	Request req;
	req.set_request("hello request");
	trpc::query(
		cli, req,
		[](auto status, auto res)
		{
			LOG_INF("%s", res->response().c_str());
		});

	asyncnet::get().wait();
}

void test_rpchttp()
{
	auto mysqlpool = connectpool<mysqlcli>::make("@127.0.0.1:3306");
	auto srv = httpserver::make("127.0.0.1:8080");

	srv->add_method(
		"/select",
		[=](auto s, auto req)
		{
			auto sql = mysqlpool->obj();
			sql->query(
				"select * from t_userinfo where userid=ashan",
				[=](auto r)
				{
					if (r->last_error != 0)
					{
						auto res = httpreponse::make();
						res->set_error(httpreponse::error_bad_request);
						s->send(res->serialize());
					}
				});
		});
	asyncnet::get().wait();
}