#ifndef __MYDISPATCH_H__
#define __MYDISPATCH_H__
#include "../service/msgserver.h"
#include "../service/msgsession.h"
#include "../service/session.h"
#include "../service/msgdispatch.h"

namespace ashan
{
	template <class _MSG>
	struct mydispatch
	{
		static int server(msgserver<int> *_this, session *s, const char *buf, int size)
		{
			_MSG *_msg = (_MSG *)(buf);
			if ((int)_msg->message_size() > size)
			{
				return 0;
			}
			s->tick();
			if (_msg->size() > 0)
			{
				_this->on_dispatch(s, _msg->message_method(), _msg->data(), _msg->size());
			}
			return (int)_msg->message_size();
		}
		static int client(msgsession *_this, const char *buf, int size)
		{
			_MSG *_msg = (_MSG *)(buf);
			if ((int)_msg->message_size() > size)
			{
				return 0;
			}
			_this->tick();
			msgdispatch<int>::get().on_dispatch(_this, _msg->message_method(), _msg->last_error(), _msg->data(), _msg->size());
			return (int)_msg->message_size();
		}
	};
}
#endif