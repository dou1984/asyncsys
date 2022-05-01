#ifndef __PBDISPATCH_H__
#define __PBDISPATCH_H__
#include <string.h>
#include "../service/msgserver.h"
#include "../service/msgdispatch.h"
#include "../joint.h"
#include "pbmsg.h"
#include "proto/pbmsg.pb.h"

namespace ashan
{
	struct pbdispatch
	{
		static int server(msgserver<std::string> *_this, session *s, const char *buf, int size)
		{
			PBMSG *_msg = (PBMSG *)(buf);
			if ((int)_msg->message_size() > size)
			{
				return 0;
			}
			s->tick();
			if (_msg->size() > 0)
			{
				MsgBase _base;
				_base.ParseFromArray(_msg->data(), _msg->size());
				_this->on_dispatch(s, _base.method(), (const char *)&_base, 0);
			}
			return (int)_msg->message_size();
		}
		static int client(msgsession *_this, const char *buf, int size)
		{
			PBMSG *_msg = (PBMSG *)(buf);
			if ((int)_msg->message_size() > size)
			{
				return 0;
			}
			_this->tick();
			if (_msg->size() > 0)
			{
				MsgBase _base;
				_base.ParseFromArray(_msg->data(), _msg->size());
				msgdispatch<std::string>::get().on_dispatch(
					_this, _base.method(), _base.error(), (const char *)&_base, 0);
			}
			return (int)_msg->message_size();
		}
		static int clientex(msgsession *_this, const char *buf, int size)
		{
			PBMSG *_msg = (PBMSG *)(buf);
			if ((int)_msg->message_size() > size)
			{
				return 0;
			}
			_this->tick();
			if (_msg->size() > 0)
			{
				MsgBase _base;
				_base.ParseFromArray(_msg->data(), _msg->size());
				msgdispatch<std::string>::get().on_dispatchex(
					_this, _base.method(), _base.error(), (const char *)&_base, 0);
			}
			return (int)_msg->message_size();
		}
	};
}
#endif