#include <atomic>
#include "../invalid.h"
#include "../log.h"
#include "../net/asyncsys.h"
#include "msgsession.h"

namespace ashan
{
	void msgsession::msg_results::__set(int e, int l, const char *d, msgsession *c)
	{
		last_error = e;
		len = l;
		data = d;
		client = c;
	}
	msgsession::msgsession(const std::string &url) : m_url(url)
	{
		__init();
	}
	msgsession::~msgsession()
	{
		LOG_ERR("rpccli close %s", m_url.c_str());
		if (m_timer != INVALID)
		{
			asyncnet::get().stop_timer(m_timer);
		}
	}
	void msgsession::query(std::string &&_req, rpccb &&_callback)
	{
		m_list.emplace_back(std::move(_req), std::move(_callback));
		__send();
	}
	void msgsession::__onconnect(int fd)
	{
		LOG_DBG("connected %s", m_url.c_str());
		m_client = client::make(fd);
		asyncnet::get().set_client(
			fd, [this](auto, auto)
			{ m_client->recv(); });

		m_client->dispatch = [this](auto cli, auto buf, auto size)
		{ return dispatch(this, buf, size); };
		m_state = e_state_waiting;
		hb::oninit(this);
		__send();
	}
	void msgsession::__onclose()
	{
		m_client = nullptr;
		if (m_state == e_state_query)
		{
			if (auto it = m_list.begin(); it != m_list.end())
			{
				auto _callback = std::move(it->m_callback);
				m_list.erase(it);
				msg_results r;
				r.last_error = INVALID;
				r.client = this;
				_callback(&r);
			}
		}
		m_state = e_state_waiting;
		__retry();
	}
	void msgsession::__send()
	{
		if (m_state == e_state_waiting && valid())
		{
			if (auto it = m_list.begin(); it != m_list.end())
			{
				m_state = e_state_query;
				m_client->send(std::move(it->m_query));
			}
		}
	}
	int msgsession::send(std::string &&req)
	{
		return valid() ? m_client->send(std::move(req)) : 0;
	}
	int msgsession::send(const char *buf, int size)
	{
		return valid() ? m_client->send(buf, size) : 0;
	}
	void msgsession::__init()
	{
		asyncsys::get().connect(
			m_url,
			[this](auto, auto fd)
			{ __onconnect(fd); },
			[this](auto, auto fd)
			{ __onclose(); });
	}
	void msgsession::__retry()
	{
		__retry_timeout();
		m_timer = asyncnet::get().set_timer(
			m_timeout, 0,
			[this](auto, auto)
			{
				m_timer = INVALID;
				__init();
			});
	}
	void msgsession::__retry_timeout()
	{
		const long _timeout = MAX_RETRY_TIMEOUT;
		auto t = mytime::now();
		for (auto it = m_list.begin(); it != m_list.end();)
		{
			if (it->m_tstamp + _timeout > t)
			{
				break;
			}			
			auto _callback = std::move(it->m_callback);
			it = m_list.erase(it);
			msg_results r;
			r.last_error = TIMEOUT;
			r.client = this;
			if (_callback)
			{
				_callback(&r);
			}
		}
	}
	void msgsession::on_dispatch(msgsession *_cli, int _e, const char *_d, int _s)
	{
		if (auto it = m_list.begin(); it != m_list.end())
		{
			assert(m_state == e_state_query);
			m_state = e_state_waiting;
			auto _callback = std::move(it->m_callback);
			m_list.erase(it);
			msg_results r;
			r.__set(_e, _s, _d, _cli);
			__TRY()
			_callback(&r);
			__CATCH()
		}
	}
}