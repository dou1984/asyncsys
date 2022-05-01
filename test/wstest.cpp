#include <assert.h>
#include "wstest.h"
#include "asys.h"
#include "ws.h"
#include "msg/rpc.h"

using namespace ashan;

struct WSCLI : msgsession, sp<WSCLI>
{
	WSCLI(const std::string &url) : msgsession(url)
	{
		msgsession::dispatch = [](msgsession *_this, const char *buf, int size) -> int
		{
			WSMSG *_msg = (WSMSG *)(buf);
			if ((int)_msg->message_size() > size)
			{
				return 0;
			}
			_this->tick();
			WS *_ws = (WS *)_msg->data();
			LOG_DBG("dispatch cmd:%d %d", _ws->cmd, size);
			msgdispatch<int>::get().on_dispatch(_this, _ws->cmd, _ws->error, _ws->buffer, _msg->size() - sizeof(WS));
			return _msg->message_size();
		};
	}
};

int test_ws()
{
	auto s = WSMSG::make(0xabcdef00, "helloworld", 11);

	auto _msg = (WSMSG *)s.data();

	assert(_msg->get_mask() == 0xabcdef00);
	printf("mask:%x head:%d msg:%lld size:%lld %s\n", _msg->get_mask(), _msg->head_size(), _msg->message_size(), _msg->size(), _msg->data());

	return 0;
}

int test_wssrv()
{
	auto srv = ashan::msgserver<int>::make("127.0.0.1:9001");
	srv->dispatch = [](msgserver<int> *_this, session *s, const char *buf, int size) -> int
	{
		WSMSG *_msg = (WSMSG *)(buf);
		if ((int)_msg->message_size() > size)
		{
			return 0;
		}
		WS *_ws = (WS *)_msg->data();
		LOG_DBG("dispatch cmd:%d %d", _ws->cmd, size);
		_this->on_dispatch(s, _ws->cmd, _ws->buffer, _msg->size() - sizeof(WS));
		return _msg->message_size();
	};

	srv->add_method(
		1,
		[](auto s, auto buf, auto size)
		{
			LOG_DBG("get method:%d %d %s", 1, size, buf);
			s->send(WS::make(1, 0, "sayhiws", 8));
		});
	asyncnet::get().wait();
	return 0;
}

int test_wscli()
{
	auto c = WSCLI::make("127.0.0.1:9001");

	c->query(WS::make(1, 0, "helloworld", 11),
			 [](auto r)
			 {
				 if (r->last_error == 0)
				 {
					 LOG_DBG("get msg %d %s", r->len, r->data);
				 }
			 });
	asyncnet::get().wait();
	return 0;
}