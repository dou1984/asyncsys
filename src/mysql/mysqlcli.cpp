#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <sys/epoll.h>
#include "../log.h"
#include "../net/asyncnet.h"
#include "../net/asyncsys.h"
#include "mysqlcli.h"

namespace ashan
{
	int mysql_get_socket_descriptor(MYSQL *mysql)
	{
		if (mysql && mysql->net.vio)
		{
			return *(int *)(mysql->net.vio);
		}
		return INVALID;
	}
	bool is_network_error(uint errorno)
	{
		return errorno == CR_CONNECTION_ERROR || errorno == CR_CONN_HOST_ERROR ||
			   errorno == CR_SERVER_GONE_ERROR || errorno == CR_SERVER_LOST ||
			   errorno == ER_CON_COUNT_ERROR || errorno == ER_SERVER_SHUTDOWN ||
			   errorno == ER_NET_READ_INTERRUPTED ||
			   errorno == ER_NET_WRITE_INTERRUPTED;
	}
	mysqlcli::mysqlcli(const std::string &dst)
	{
		m_mysql = mysql_init(0);
		if (m_mysql == nullptr)
		{
			LOG_ERR("mysql_init error %d", errno);
			throw("mysql_init error");
		}
		std::string url;
		if (!asyncsys::spliturl(dst, m_user, m_password, url))
		{
			LOG_ERR("mysql spliturl %s", dst.c_str());
		}
		if (!asyncsys::splitaddr(url, m_ip, m_port))
		{
			throw("mysql splitaddr error");
		}
		__init();
	}
	mysqlcli::~mysqlcli()
	{
		if (m_timer != INVALID)
		{
			asyncnet::get().stop_timer(m_timer);
		}
		mysql_close(m_mysql);
	}
	void mysqlcli::query(std::string &&_sql, mysqlcb &&_cb)
	{
		auto it = m_list.emplace_back(std::move(_sql), std::move(_cb));
		if (m_list.size() == 1 && m_state == e_clistate_connected)
		{
			__onquery();
		}
	}
	int mysqlcli::__tryconnect()
	{
		auto status = mysql_real_connect_nonblocking(
			m_mysql, m_ip.c_str(), m_user.c_str(), m_password.c_str(), m_db.c_str(), m_port, nullptr, 0);
		LOG_CORE("mysql_real_connect_nonblocking fd:%d status:%d addr:%s:%d %s %s %s",
				 mysql_get_socket_descriptor(m_mysql), status,
				 m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_db.c_str());
		m_state = status == NET_ASYNC_COMPLETE ? e_clistate_connected : e_clistate_connecting;
		return status;
	}
	int mysqlcli::__onquery()
	{
		if (auto it = m_list.begin(); it != m_list.end())
		{
			auto &_sql = it->m_query;
			auto status = mysql_real_query_nonblocking(m_mysql, _sql.c_str(), _sql.size());
			LOG_CORE("query async(%d):%s %s", status, _sql.c_str(), mysql_error(m_mysql));
			return status;
		}
		return NET_ASYNC_COMPLETE_NO_MORE_RESULTS;
	}
	int mysqlcli::__onqueryerr(int status)
	{
		if (status == NET_ASYNC_ERROR)
		{
			if (auto it = m_list.begin(); it != m_list.end())
			{
				sql_results r;
				r.last_error = mysql_errno(m_mysql);
				r.last_msg = mysql_error(m_mysql);
				LOG_DBG("mysql error:%d:%s", r.last_error, r.last_msg);
				auto _callback = std::move(it->m_callback);
				if (_callback)
				{
					_callback(&r);
				}
				m_list.pop_front();
			}
			return 1;
		}
		assert(status == 0);
		return 0;
	}
	int mysqlcli::__onstore()
	{
		if (auto it = m_list.begin(); it != m_list.end())
		{
			int status = 0;
			MYSQL_RES *results;
			while ((status = mysql_store_result_nonblocking(m_mysql, &results)) == NET_ASYNC_NOT_READY)
			{
			}
			assert(status == NET_ASYNC_COMPLETE);
			sql_results r;
			r.num_rows = mysql_affected_rows(m_mysql);
			r.query = std::move(it->m_query);
			auto _callback = std::move(it->m_callback);
			if (_callback == nullptr)
			{
				LOG_DBG("callback is nullptr num_rows:%d", r.num_rows);
			}
			else if (results != nullptr)
			{
				while ((status = mysql_fetch_row_nonblocking(results, &r.rows)) == NET_ASYNC_COMPLETE && r.rows)
				{
					assert(mysql_errno(m_mysql) == 0);
					_callback(&r);
				}
				while ((status = mysql_free_result_nonblocking(results)) != NET_ASYNC_COMPLETE)
				{
				}
			}
			else
			{
				r.last_error = mysql_errno(m_mysql);
				if (r.last_error == 0)
				{
					char _msg[0x100];
					snprintf(_msg, sizeof(_msg), "affected %d rows", r.num_rows);
					r.last_msg = _msg;
					_callback(&r);
					LOG_DBG("callback error %s", _msg);
				}
				else
				{
					r.last_msg = mysql_error(m_mysql);
					_callback(&r);
					LOG_DBG("callback errer %s", r.last_msg);
				}
			}
			m_list.pop_front();
		}
		return 0;
	}
	void mysqlcli::__ontimer()
	{
		m_timer = INVALID;
		__init();
	}
	void mysqlcli::__onrecv()
	{
		int status = 0;
		if (m_state != e_clistate_connected)
		{
			while ((status = __tryconnect()) == NET_ASYNC_NOT_READY)
			{
			}
			if (__checkerror(NET_ASYNC_ERROR) == INVALID)
			{
				return;
			}
			status = __onquery();
			assert(status != NET_ASYNC_ERROR);
		}
		else
		{
			while ((status = __onquery()) == NET_ASYNC_NOT_READY)
			{
			}
			if (__checkerror(status) == INVALID)
			{
				return;
			}
			if ((status = __onqueryerr(status)) == 0)
			{
				__onstore();
				if (__checkerror(NET_ASYNC_ERROR) == INVALID)
				{
					return;
				}
			}
		__retry__:
			if ((status = __onquery()) == NET_ASYNC_COMPLETE)
			{
				__onstore();
				goto __retry__;
			}
			if (__checkerror(status) == INVALID)
			{
				return;
			}
		}
	}
	void mysqlcli::__onconnect(asyncnet *_net, int fd)
	{
		LOG_DBG("connect success fd:%d", fd);
		_net->mod_event(fd, asyncnet::e_state_client_ex, EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET);
		_net->set_client(
			fd, [this](auto, auto)
			{ __onrecv(); });
		__tryconnect();
	}
	void mysqlcli::__onclose(asyncnet *, int fd)
	{
		LOG_ERR("mysql close error fd:%d", fd);
		__clear();
		__retry();
	}
	void mysqlcli::__init()
	{
		auto state = __tryconnect();
		m_fd = mysql_get_socket_descriptor(m_mysql);
		LOG_DBG("fd:%d %d", m_fd, state);
		if (m_fd == INVALID)
		{
			return __retry();
		}
		asyncnet::get().__fd_add_event(
			m_fd, asyncnet::e_state_connect_ex,
			[this](auto _net, auto fd)
			{ __onconnect(_net, fd); },
			[this](auto _net, auto fd)
			{ __onclose(_net, fd); });
	}
	void mysqlcli::__retry()
	{
		__retry_timeout();
		m_timer = asyncnet::get().set_timer(
			m_timeout, 0,
			[this](auto, auto)
			{ __ontimer(); });
		LOG_DBG("__retry fd:%d timeout:%d", m_timer, m_timeout);
	}
	void mysqlcli::__retry_timeout()
	{
		const long _timeout = MAX_RETRY_TIMEOUT;
		auto t = mytime::now();
		for (auto it = m_list.begin(); it != m_list.end();)
		{
			if (it->m_tstamp + _timeout > t)
			{
				break;
			}
			sql_results r;
			r.last_error = TIMEOUT;
			r.last_msg = "retry time out";
			r.query = std::move(it->m_query);
			auto _callback = std::move(it->m_callback);
			if (_callback)
			{
				_callback(&r);
			}
			it = m_list.erase(it);
		}
	}
	int mysqlcli::__checkerror(int status)
	{
		if (status == NET_ASYNC_ERROR)
		{
			if (is_network_error(mysql_errno(m_mysql)))
			{
				__clear();
				__retry();
				return INVALID;
			}
		}
		return 0;
	}
	void mysqlcli::__clear()
	{
		int fd = mysql_get_socket_descriptor(m_mysql);
		LOG_ERR("__clear network error fd:%d:%d %d %s", fd, m_fd,
				mysql_errno(m_mysql), mysql_error(m_mysql));
		if (m_fd == INVALID)
		{
			return;
		}

		m_state = e_clistate_closed;
		asyncnet::get().__clientclosed_already(m_fd);
		m_fd = INVALID;
	}

}
