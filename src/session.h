#ifndef __SESSION_H__
#define __SESSION_H__
#include <memory>
#include <mutex>
#include <list>
#include "client.h"
#include "object.h"
#include "boolean.h"

namespace ashan
{
	class session : public object<std::string, session>
	{
		std::shared_ptr<client> m_client;
		std::list<std::string> m_list;
		int m_notified = FALSE;
		void __notifysend();
		void __send();

	public:
		std::shared_ptr<session> set_client(std::shared_ptr<client> _cli);
		void send(std::string &&);
		void send(const char *buf, int size) { return send(std::string(buf, size)); }
	};
}
#endif