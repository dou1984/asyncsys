#include <string.h>
#include "msg.h"

namespace ashan
{
	static void __set(MSG *_msg, int cmd, u32 s, int err)
	{
		_msg->len = s;
		_msg->cmd = cmd;
		_msg->error = err;
	}
	std::string MSG::make(int cmd, const char *buf, u32 s)
	{
		std::string t;
		t.resize(sizeof(MSG) + s);
		MSG *_msg = (MSG *)t.data();
		__set(_msg, cmd, s, 0);
		if (s > 0)
		{
			memcpy(_msg->buffer, buf, s);
		}
		return t;
	}
	std::string MSG::make(int cmd, int err, const char *buf, u32 s)
	{
		std::string t;
		t.resize(sizeof(MSG) + s);
		MSG *_msg = (MSG *)t.data();
		__set(_msg, cmd, s, err);
		if (s > 0)
		{
			memcpy(_msg->buffer, buf, s);
		}
		return t;
	}
	std::string MSG::make(int cmd, int err)
	{
		std::string t;
		t.resize(sizeof(MSG));
		MSG *_msg = (MSG *)t.data();
		__set(_msg, cmd, 0, err);
		return t;
	}
}