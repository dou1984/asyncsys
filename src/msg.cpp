#include <string.h>
#include "msg.h"

namespace ashan
{
	std::string MSG::make(int c, const char *buf, int s)
	{
		std::string t;
		t.resize(sizeof(MSG) + s);
		MSG *_msg = (MSG *)t.data();
		_msg->len = s;
		_msg->cmd = c;
		_msg->error = 0;
		if (s > 0)
		{
			memcpy(_msg->data, buf, s);
		}
		return t;
	}
	std::string MSG::make(int c, int err)
	{
		std::string t;
		t.resize(sizeof(MSG));
		MSG *_msg = (MSG *)t.data();
		_msg->len = 0;
		_msg->cmd = c;
		_msg->error = err;
		return t;
	}
}