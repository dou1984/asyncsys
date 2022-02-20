#ifndef __RPC_SESSION_H__
#define __RPC_SESSION_H__
#include <list>
#include "client.h"
#include "msg.h"
#include "sp.h"
#include "cliquery.h"

namespace ashan
{
	class msgsession final : public sp<msgsession>
	{
	public:
		struct rpc_results
		{
			int last_error = 0;
			int len = 0;
			const char *data = 0;
			std::shared_ptr<msgsession> cli;
		};
		using rpccb = std::function<void(const rpc_results *)>;
		using rpcquery = cliquery<rpccb>;
		msgsession(const std::string &url);
		virtual ~msgsession();
		int query(std::string &&_req, rpccb &&_cb);
		bool valid() const { return ::valid(m_client); }

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
		int __dispatch(const char *buf, int size);
		void __onconnect(int fd);
		void __onclose();
		void __send();
		void __init();
		void __retry();
	};	
}

#endif