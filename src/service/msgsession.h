#ifndef __MSGSESSION_H__
#define __MSGSESSION_H__
#include <list>
#include <mutex>
#include "../cliquery.h"
#include "../sp.h"
#include "../log.h"
#include "../mythrow.h"
#include "../net/heardbeat.h"
#include "../myresults.h"
#include "client.h"

namespace ashan
{
	class msgsession : public heartbeat<msgsession>
	{
	public:
		struct msg_results : myresults
		{			
			int len = 0;
			const char *data = 0;
			msgsession *client = 0;
			void __set(int e, int l, const char *d, msgsession *c);
		};
		using rpccb = std::function<void(const msg_results *)>;
		using rpcquery = cliquery<rpccb>;
		using hb = heartbeat<msgsession>;
		msgsession(const std::string &url);
		virtual ~msgsession();
		void query(std::string &&_req, rpccb &&_cb);
		bool valid() const { return ::valid(m_client); }
		int send(std::string &&_req);
		int send(const char *, int);
		void send() { __send(); }
		void stop() { m_client = nullptr; }
		std::function<int(msgsession *, const char *buf, int size)> dispatch;
		void on_dispatch(msgsession *_cli, int _error, const char *_data, int _size);

	private:
		enum e_state
		{
			e_state_waiting,
			e_state_query,
		};
		std::list<rpcquery> m_list;
		std::shared_ptr<client> m_client;
		std::string m_url;
		int m_state = e_state_waiting;
		int m_timeout = 1000;
		int m_timer = INVALID;

		void __onconnect(int fd);
		void __onclose();
		void __send();
		void __init();
		void __retry();
		void __retry_timeout();
	};
}

#endif