#include <string.h>
#include "ws.h"
#include "wsmsg/wsmsg.h"

namespace ashan
{
	std::string WS::make(u32 mask, int cmd, int error, const char *buf, u32 s)
	{
		std::string r;
		auto _size = sizeof(WS) + s;
		r.resize(sizeof(WSMSG) + WSMSG::c_mask_size(mask) + WSMSG::c_payload_size(_size) + _size);
		WSMSG *_msg = (WSMSG *)r.data();
		_msg->set_size(_size);
		_msg->set_mask(mask);

		WS *_ws = (WS *)_msg->data();
		_ws->cmd = cmd;
		_ws->error = error;
		if (s > 0)
		{
			memcpy(_ws->buffer, buf, s);
		}
		return r;
	}
	std::string WS::make(int cmd, int error, const char *buf, u32 s)
	{
		std::string r;
		auto _size = sizeof(WS) + s;
		r.resize(sizeof(WSMSG) + WSMSG::c_payload_size(_size) + _size);
		WSMSG *_msg = (WSMSG *)r.data();
		_msg->set_size(_size);

		WS *_ws = (WS *)_msg->data();
		_ws->cmd = cmd;
		_ws->error = error;
		if (s > 0)
		{
			memcpy(_ws->buffer, buf, s);
		}
		return r;
	}
}