#include "asyncnet.h"
#include "asyncsys.h"
#include "msgserver.h"
#include "session.h"
#include "rpcfunc.h"
#include "proto/msg.pb.h"
#include "log.h"

using namespace google::protobuf;
using namespace ashan;

using trpc = rpc<1, Request, Response>;
void test_rpc()
{
	auto srv = msgserver::make("127.0.0.1:9001");
	trpc::add_method(
		srv,
		[](auto s, auto req, auto res)
		{
			res->set_response("helloworld");
			return 0;
		});
	asyncnet::get().wait();
}

void test_rpccc()
{

	auto cli = msgsession::make("127.0.0.1:9001");
	Request req;
	req.set_request("hello");
	trpc::query(
		cli, req,
		[](auto status, auto res)
		{
			LOG_INF("%s", res->response().c_str());
		});

	asyncnet::get().wait();
}