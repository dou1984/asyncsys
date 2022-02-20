#ifndef __MYSQLCLI_H__
#define __MYSQLCLI_H__
#include <string>
#include <list>
#include <functional>
#include <mysql/mysql.h>
#include "cliquery.h"
#include "clistate.h"
#include "sp.h"

namespace ashan
{
	class mysqlcli final : public sp<mysqlcli>
	{
	public:
		struct my_results
		{
			int last_error = 0;				// last error
			int num_rows = 0;				// affect rows num
			const char *last_msg = nullptr; // last error string
			std::string query;				// query string
			MYSQL_ROW rows = 0;				// query results
		};
		using mysqlcb = std::function<void(const my_results *)>;
		using mysqlquery = cliquery<mysqlcb>;

		mysqlcli(const std::string &url);
		~mysqlcli();
		int query(std::string &&, mysqlcb &&);
		bool valid() const { return m_state == e_clistate_connected; }

	private:
		int __tryconnect();
		int __onquery();
		int __onqueryerr(int);
		int __onstore();
		int __checkerror(int status);
		void __ontimer();
		void __onrecv();
		void __onconnect(asyncnet *, int);
		void __onclose(asyncnet *, int);
		void __init();
		void __retry();
		void __clear();

	private:
		MYSQL *m_mysql;
		std::list<mysqlquery> m_list;
		std::string m_user = "debian-sys-maint";
		std::string m_password = "eZpPxmwEE8lkWaQI";
		std::string m_db = "test";
		std::string m_ip = "127.0.0.1";
		int m_port = 3306;

		int m_fd = INVALID;
		int m_timer = INVALID;
		int m_timeout = 1000;
		int m_state = e_clistate_closed;
	};
}

#endif