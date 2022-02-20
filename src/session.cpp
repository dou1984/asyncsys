#include "thdid.h"
#include "session.h"
#include "asyncnet.h"
#include "invalid.h"

namespace ashan
{
	void session::__notifysend()
	{
		if (check_valid(m_client) && m_notified == FALSE)
		{
			m_notified = TRUE;
			auto _this = shared_from_this();
			auto _net = asyncnet::at(m_client->thdid());
			_net->notify(
				[=](auto, auto)
				{
					OBJLOCK(_this->m_mutex);
					_this->m_notified = FALSE;
					_this->__send();
				});
		}
	}
	void session::__send()
	{
		for (auto it = m_list.begin(); it != m_list.end(); it = m_list.erase(it))
		{
			if (!check_valid(m_client))
				break;
			if (g_thdid == m_client->thdid())
			{
				m_client->send(std::move(*it));
			}
			else
			{
				__notifysend();
				break;
			}
		}
	}
	std::shared_ptr<session> session::set_client(std::shared_ptr<client> _cli)
	{
		OBJLOCK(m_mutex);
		m_client = _cli;
		__send();
		return shared_from_this();
	}
	void session::send(std::string &&msg)
	{
		OBJLOCK(m_mutex);
		if (check_valid(m_client))
		{
			if (!m_list.empty())
			{
				m_list.emplace_back(std::move(msg));
				return;
			}
			if (g_thdid == m_client->thdid())
			{
				m_client->send(std::move(msg));
			}
			else
			{
				auto r = m_client->__realsend(std::move(msg));
				if (r == INVALID)
				{
					m_list.emplace_back(std::move(msg));
					if (errno == EAGAIN || errno == EINPROGRESS)
					{
						__notifysend();
					}
				}
			}
		}
		else
		{
			m_list.emplace_back(std::move(msg));
		}
	}
}