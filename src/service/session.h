#ifndef __SESSION_H__
#define __SESSION_H__
#include <memory>
#include <mutex>
#include <list>
#include <functional>
#include "../object.h"
#include "../boolean.h"
#include "../invalid.h"
#include "../net/heardbeat.h"
#include "client.h"

namespace ashan
{
	class session final : public heartbeat<session>, public object<std::string, session>
	{
		std::shared_ptr<client> m_client;
		std::list<std::string> m_list;
		int m_notified = FALSE;
		void __notifysend();
		void __send();

	public:		
		std::shared_ptr<session> set_client(client *_cli);
		void send(std::string &&);
		void send(const char *buf, int size) { return send(std::string(buf, size)); }
		void send(std::list<std::string> &&_lst);
		bool valid() const { return ::valid(m_client); }
		void stop() { m_client = nullptr; }	
	};
}
#endif