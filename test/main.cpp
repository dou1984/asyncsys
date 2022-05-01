#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <memory>
#include <chrono>
#include <atomic>
#include "asys.h"
#include "rpctest.h"
#include "wstest.h"
#include "jsontest.h"

#define CHECK_MSG(_MSG, _BUF, _SIZE)         \
	ashan::MSG *_MSG = (ashan::MSG *)(_BUF); \
	if ((int)_MSG->message_size() > _SIZE)   \
		return 0;

using a3cli = ashan::client;
using sql_results = const ashan::mysqlcli::sql_results;
using rds_results = const ashan::rediscli::rds_results;
using msg_results = const ashan::msgsession::msg_results;
using sess = ashan::session;
using MSG = ashan::MSG;

auto cli_dispatch = [](auto c, auto buf, auto size) -> int
{
	CHECK_MSG(_msg, buf, size);
	if (_msg->cmd == 1)
	{
		c->send(ashan::MSG::make(1, "helloworld", 10));
		LOG_DBG("send msg %d %d", _msg->cmd, _msg->len);
	}
	return _msg->message_size();
};
auto msg_dispatch = [](ashan::client *cli, const char *buf, int size) -> int
{
	CHECK_MSG(_msg, buf, size);
	LOG_DBG("size:%d recv:%d %d", size, _msg->cmd, _msg->len);
	if (_msg->cmd == 0)
	{
		std::string id(_msg->data(), _msg->size());
		auto s = sess::get_pool().get(id);
		if (s == nullptr)
		{
			s = sess::make();
			sess::get_pool().insert(id, s);
		}

		s->set_client(cli);

		cli->dispatch = [=](auto c, auto buf, auto size) -> int
		{
			CHECK_MSG(_msg, buf, size);
			if (_msg->cmd == 1)
			{
				s->send(buf, _msg->message_size());
			}
			return _msg->message_size();
		};
	}
	return _msg->message_size();
};

auto msg_cli = [](ashan::asyncnet *_net, int fd)
{
	static std::atomic_int g_id = 0;

	auto cli = a3cli::make(fd)->init();
	cli->dispatch = cli_dispatch;
	std::string id = std::to_string(g_id++);
	LOG_DBG("connect success %s", id.c_str());

	cli->send(ashan::MSG::make(0, id.c_str(), id.size()));
	cli->send(ashan::MSG::make(1, "helloworld", 10));
};
auto msg_srv = [](ashan::asyncnet *_net, int fd)
{
	auto cli = a3cli::make(fd)->init();
	cli->dispatch = msg_dispatch;
};
auto com_dispatch = [](ashan::client *cli, const char *buf, int size) -> int
{
	cli->send(buf, size);
	return 0;
};
auto srv_online = [](ashan::asyncnet *_net, int fd)
{
	auto cli = a3cli::make(fd)->init();
	cli->dispatch = com_dispatch;
};
auto cli_online = [](ashan::asyncnet *_net, int fd)
{
	auto cli = a3cli::make(fd)->init();
	cli->dispatch = com_dispatch;
	cli->send("helloworld", 10);
};

int test_timer()
{
	auto fd = ashan::asyncnet::get().set_timer(
		1000, 1000,
		[&](auto _net, auto _fd)
		{
			printf("ontimer\n");
			static int i = 0;
			if (++i == 3)
			{
				_net->stop_timer(_fd);
				_net->notify(
					[](auto, auto)
					{ printf("timer stop\n"); });
			}
		});

	ashan::asyncnet::get().wait();
	return 0;
}

auto g_rds()
{
	static ashan::connectpool<ashan::rediscli> rds_pool("127.0.0.1:6379");
	return rds_pool.obj();
}
int test_redis()
{
	auto cli = g_rds();

	auto f = [](rds_results *r)
	{
		if (r->last_error == 0)
		{
			LOG_DBG("%s => %s", r->query.c_str(), r->str);
		}
		else
		{
			LOG_DBG("%s => %s", r->query.c_str(), r->strerror());
		}
	};
	auto f2 = [](rds_results *r)
	{
		if (r->last_error == 0)
		{
			for (auto i = 0; i < r->num_rows;)
			{
				auto a = i++;
				auto b = i++;
				LOG_DBG("%s => %s %s", r->query.c_str(), r->rows[a]->str, r->rows[b]->str);
			}
		}
		else
		{
			LOG_DBG("%s => %s", r->query.c_str(), r->strerror());
		}
	};
	cli->query("getxx aaaa", f);
	cli->query("get aaaa", f);
	cli->query("get abcd", f);
	cli->query("hgetall hb", f2);
	cli->query("set bbbb 1", f);
	cli->query("del bbbb", [](auto r)
			   { LOG_DBG("del %d", r->num_rows); });
	ashan::asyncnet::get().wait();
	return 0;
}

void rdscb(rds_results *r)
{
	if (r->last_error == 0)
	{
		LOG_DBG("query:%s %s", r->query.c_str(), r->str);
	}
	g_rds()->query("get aaaa", rdscb);
}
void test_repeatredis()
{
	g_rds()->query("get aaaa", rdscb);

	ashan::asyncnet::get().wait();
}
void rdshshow(rds_results *r)
{
	if (r->last_error == 0)
	{
		LOG_DBG("query:%s", r->query.c_str());
		for (int i = 0; i < r->num_rows;)
		{
			auto a = i++;
			auto b = i++;
			LOG_DBG("\t%s\t%s", r->rows[a]->str, r->rows[b]->str);
		}
		g_rds()->query("hgetall hhhh", rdshshow);
	}
}
void test_repeathash()
{
	g_rds()->query("hgetall hhhh", rdshshow);
	ashan::asyncnet::get().wait();
}

void test_httpserver()
{
	auto srv = ashan::httpserver::make(":8080");
	srv->add_method(
		"/echo",
		[](auto s, auto &req)
		{
			for (auto it : req.m_header)
			{
				LOG_DBG("%s %s", it.first.c_str(), it.second.c_str());
			}
			LOG_DBG("%s", req.m_body.c_str());

			ashan::httpreponse rep;
			rep.set_error(200);
			rep.set_body(R"({"a":2})");
			auto _lst = rep.serialize();
			for (auto &it : _lst)
			{
				s->send(std::move(it));
			}
		});
	ashan::asyncnet::get().wait();
}

auto g_sql()
{
	static auto _cli = ashan::mysqlcli::make("127.0.0.1:3306");
	return _cli;
}
int test_mysql()
{

	g_sql()->query("select * from t_userinfo;", [](auto r)
				   { LOG_DBG("%s %s %s", r->rows[0], r->rows[1], r->rows[2]); });
	g_sql()->query("select * from t_userinfo;", [](auto r)
				   { LOG_DBG("%s %s %s", r->rows[0], r->rows[1], r->rows[2]); });
	g_sql()->query("update t_userinfo set `mobile`='133333' where `id`=1;", [](auto r)
				   { LOG_DBG("%d %s", r->last_error, r->last_msg); });
	g_sql()->query("insert into t_userinfo (`userid`,`password`,`mobile`) values ('abccc','fffff','000000');",
				   [](auto r)
				   {
					   LOG_DBG("%d %s", r->last_error, r->last_msg);
				   });
	g_sql()->query("select from t_userinfo;", [](auto r)
				   { LOG_DBG("%d %s", r->last_error, r->last_msg); });
	ashan::asyncnet::get().wait();
	return 0;
}

void tquery(sql_results *r)
{
	if (r->last_error == 0)
	{
		auto rows = r->rows;
		if (rows)
		{
			LOG_DBG("%s %s %s", rows[0], rows[1], rows[2]);
		}
		g_sql()->query("select * from t_userinfo;", tquery);
	}
}

int test_mysqlstop()
{
	g_sql()->query("select * from t_userinfo;", tquery);
	ashan::asyncnet::get().wait();
	return 0;
}
auto pool = ashan::connectpool<ashan::rpccli>::make("127.0.0.1:9000");
void test_rpccb(msg_results *r)
{
	if (r->last_error == INVALID)
	{
		LOG_ERR("%d", r->last_error);
		r->client->query(MSG::make(1, "helloworld", 10), test_rpccb);
	}
	else
	{
		LOG_DBG("recv %s", r->data);
		{
			auto rpccli = pool->obj();
			rpccli->query(MSG::make(1, "helloworld", 10), test_rpccb);
		}
	}
}
void test_rpccli()
{
	std::list<std::thread> _lst;
	for (int i = 0; i < 4; i++)
	{
		std::thread t(
			[]()
			{
				{
					auto rpccli = pool->obj();
					rpccli->query(MSG::make(1, "helloworld", 10), test_rpccb);
				}
				ashan::asyncnet::get().wait();
			});
		t.detach();
		_lst.emplace_back(std::move(t));
	}
	ashan::asyncnet::get().wait();
}
void test_rpcsrv()
{
	auto srv = ashan::msgserver<int>::make("127.0.0.1:9000");
	srv->dispatch = ashan::mydispatch<MSG>::server;

	srv->add_method(
		1,
		[](auto s, auto buf, auto size)
		{
			LOG_DBG("recv %s", buf);
			s->send(MSG::make(1, buf, size));
		});

	ashan::asyncnet::get().wait();
}
struct vec
{
	int x;
	int y;
	int z;
};

int test_event()
{
	ashan::myevent<vec>::onevent(
		"echo",
		[](auto t)
		{
			std::cout << t->x << t->y << t->z << std::endl;
		});
	ashan::myevent<vec>::emit(
		"echo",
		{
			.x = 1,
			.y = 2,
			.z = 3,
		});

	ashan::asyncnet::get().wait();
	return 0;
}
int test_kafka_consume()
{
	auto c = ashan::kafkaconsume::make("broker");

	std::thread t(
		[=]()
		{
			c->pull(
				"echo",
				[](auto r)
				{
					if (r->last_msg == nullptr)
					{
					}
				});
		});
	t.join();

	return 0;
}
int test_kafka_produce()
{
	auto c = ashan::kafkaproduce::make("broker");
	c->push("echo", "helloworld");
	return 0;
}
int test_regex()
{
	using namespace ashan;
	mysys::get().splite(
		"lobby@0.0.0.0:9090;:9099", ";",
		[](auto &s)
		{
			std::string name = "gateway";
			std::string ip = "0.0.0.0";
			std::string port = "0";
			if (mysys::get().regex_search(R"((\w+)@(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))", s, name, ip, port) || mysys::get().regex_search(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+))", s, ip, port) || mysys::get().regex_search(R"(:(\d+))", s, port))
			{
				LOG_DBG("%s %s %s %s", s.c_str(), name.c_str(), ip.c_str(), port.c_str());
			}
		});

	return 0;
}
int main(int argc, char **argv)
{
	ashan::mysys::get().ignore_signal();

	// set_log_level(e_log_level_error);
	set_log_level(e_log_level_core);

	ashan::asyncsys sys;
	if (strcmp(argv[1], "server") == 0)
	{
		sys.start("127.0.0.1:9999", srv_online);
		ashan::asyncnet::get().wait();
	}
	else if (strcmp(argv[1], "client") == 0)
	{
		sys.connect("127.0.0.1:9999", cli_online);
		ashan::asyncnet::get().wait();
	}
	else if (strcmp(argv[1], "msgserver") == 0)
	{
		sys.start("127.0.0.1:10000", msg_srv);
		ashan::asyncnet::get().wait();
	}
	else if (strcmp(argv[1], "msgclient") == 0)
	{
		auto f = [&]()
		{
			sys.connect("127.0.0.1:10000", msg_cli);
			ashan::asyncnet::get().wait();
		};
		std::list<std::thread> _lst;
		for (int i = 0; i < 4; i++)
		{
			std::thread t(f);
			t.detach();
			_lst.emplace_back(std::move(t));
		}
		ashan::asyncnet::get().wait();
	}
	else if (strcmp(argv[1], "mysql") == 0)
	{
		test_mysql();
	}
	else if (strcmp(argv[1], "repeatmysql") == 0)
	{
		test_mysqlstop();
	}
	else if (strcmp(argv[1], "redis") == 0)
	{
		test_redis();
	}
	else if (strcmp(argv[1], "repeatredis") == 0)
	{
		test_repeatredis();
	}
	else if (strcmp(argv[1], "repeathash") == 0)
	{
		test_repeathash();
	}
	else if (strcmp(argv[1], "httpserver") == 0)
	{
		test_httpserver();
	}
	else if (strcmp(argv[1], "msgclient") == 0)
	{
		test_rpccli();
	}
	else if (strcmp(argv[1], "msgserver") == 0)
	{
		test_rpcsrv();
	}
	else if (strcmp(argv[1], "rpc") == 0)
	{
		test_rpc();
	}
	else if (strcmp(argv[1], "rpccc") == 0)
	{
		test_rpccc();
	}
	else if (strcmp(argv[1], "event") == 0)
	{
		test_event();
	}
	else if (strcmp(argv[1], "kafkac") == 0)
	{
		test_kafka_consume();
	}
	else if (strcmp(argv[1], "kafkap") == 0)
	{
		test_kafka_produce();
	}
	else if (strcmp(argv[1], "ws") == 0)
	{
		test_ws();
	}
	else if (strcmp(argv[1], "wssrv") == 0)
	{
		test_wssrv();
	}
	else if (strcmp(argv[1], "wscli") == 0)
	{
		test_wscli();
	}
	else if (strcmp(argv[1], "json") == 0)
	{
		test_json();
	}
	else
	{
		test_regex();
	}
	return 0;
}
