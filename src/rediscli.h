#ifndef __REDISCLI_H__
#define __REDISCLI_H__
#include <string>
#include <functional>
#include <list>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include "cliquery.h"
#include "clistate.h"
#include "sp.h"
#include "invalid.h"

namespace ashan
{
	class rediscli final : public sp<rediscli>
	{
	public:
		struct rds_results final
		{
			int num_rows = 0;
			const char *last_error = 0;
			std::string query;
			union
			{
				const char *str;
				struct redisReply **rows;
			};
		};
		using rediscb = std::function<void(const rds_results *)>;
		using redisquery = cliquery<rediscb>;

	public:
		rediscli(const std::string &addr);
		~rediscli();
		int query(std::string &&addr, rediscb &&);
		bool valid() const { return m_state == e_clistate_connected; }

	private:
		void __onquery();
		void __onconnect();
		void __onsend(redisAsyncContext *);
		void __onrecv(redisAsyncContext *);
		void __attach(redisAsyncContext *);
		void __oncallback(redisReply *);
		void __init();
		void __retry();
		void __clear();
		static auto __get(const redisAsyncContext *ac) { return (rediscli *)ac->ev.data; }
		static void __addread(void *privdata);
		static void __delread(void *privdata);
		static void __addwrite(void *privdata);
		static void __delwrite(void *privdata);
		static void __clearup(void *privdata);
		static void __settimeout(void *privdata, struct timeval tv);
		static void __connected(const redisAsyncContext *c, int status);
		static void __disconnected(const redisAsyncContext *c, int status);
		friend void __callback(redisAsyncContext *c, void *r, void *privdata);

	private:
		std::list<redisquery> m_list;
		std::string m_ip = "127.0.0.1";
		std::string m_auth;
		int m_port = 6379;

		int m_fd = INVALID;
		int m_state = 0;
		int m_events = 0;
		int m_timer = INVALID;
		int m_timeout = 1000;
		redisAsyncContext *m_context = nullptr;
	};
}

#endif