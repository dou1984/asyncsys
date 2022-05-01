#ifndef __MSGSERVER_H__
#define __MSGSERVER_H__
#include <memory>
#include <string>
#include <unordered_map>
#include "../mythrow.h"
#include "../sp.h"
#include "../mytype.h"
#include "../log.h"
#include "../net/heardbeat.h"
#include "client.h"
#include "session.h"

namespace ashan
{
	template <class _ID>
	struct msgserver : sp<msgserver<_ID>>
	{
		using rpccb = std::function<void(session *, const char *, int)>;
		std::unordered_map<_ID, rpccb> m_router;
		std::string m_url;
		std::function<int(msgserver<_ID> *, session *, const char *buf, int size)> dispatch;

		msgserver(const std::string &url);
		void add_method(const _ID &cmd, rpccb &&_callback);
		void on_dispatch(session *s, const _ID &_method, const char *_data, int _size);
	};
}
#include "msgserver.inl"
#endif