#include "asyncsys.h"
#include "msgsession.h"
#include "invalid.h"
#include "log.h"

namespace ashan
{	
	msgsession::msgsession(const std::string &url) : m_url(url)
	{
		__init();
	}
	msgsession::~msgsession()
	{
		LOG_ERR("rpcclient close %s", m_url.c_str());
		if (m_timer != INVALID)
		{
			asyncnet::get().stop_timer(m_timer);
		}
	}
	int msgsession::query(std::string &&_req, rpccb &&_callback)
	{
		m_list.emplace_back(std::move(_req), std::move(_callback));
		__send();
		return 0;
	}
	int msgsession::__dispatch(const char *buf, int size)
	{
		MSG *_msg = (MSG *)(buf);
		if (_msg->message_size() > size)
		{
			return 0;
		}
		if (auto it = m_list.begin(); it != m_list.end())
		{
			assert(m_state == e_state_query);
			m_state = e_state_waiting;
			auto _callback = std::move(it->m_callback);
			m_list.erase(it);
			rpc_results r;
			r.last_error = _msg->error;
			r.len = _msg->len;
			r.data = _msg->data;
			r.cli = shared_from_this();
			_callback(&r);
		}
		__send();
		return _msg->message_size();
	}
	void msgsession::__onconnect(int fd)
	{
		LOG_DBG("connected %s", m_url.c_str());
		m_client = client::make(fd);
		asyncnet::get().set_client(
			fd, [this](auto, auto)
			{ m_client->recv(); });

		m_client->dispatch = [this](auto cli, auto buf, auto size)
		{ return __dispatch(buf, size); };
		m_state = e_state_waiting;
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
				rpc_results r;
				r.last_error = INVALID;
				r.cli = shared_from_this();
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
		m_timer = asyncnet::get().set_timer(
			m_timeout, 0,
			[this](auto, auto)
			{
				m_timer = INVALID;
				__init();
			});
	}
}