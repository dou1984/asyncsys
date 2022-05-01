#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include "../thdid.h"
#include "../log.h"
#include "../net/asyncnet.h"
#include "client.h"

namespace ashan
{
	client::client(int _fd) : m_fd(_fd), m_thdid(g_thdid)
	{
	}
	int client::__realsend(std::string &&msg)
	{
		return ::send(m_fd, msg.c_str(), msg.size(), 0);
	}
	int client::send(const char *buf, int size) const
	{
		return send(std::string(buf, size));
	}
	int client::send(std::string &&msg) const
	{
		assert(m_thdid == g_thdid);
		return asyncnet::get().send(m_fd, std::move(msg));
	}
	int client::recv()
	{
		assert(m_thdid == g_thdid);
		while (m_iobuf.append(
				   [this](auto buf, auto size)
				   { return ::recv(m_fd, buf, size, 0); }) > 0)
		{
			if (dispatch)
			{
			__retry__:
				int r = dispatch(this, m_iobuf.data(), m_iobuf.size());
				if (r > 0)
				{
					m_iobuf.pop_front(r);
					if (!m_iobuf.empty())
					{
						goto __retry__;
					}
				}
			}
			else
			{
				m_iobuf.clear();
			}
		}
		return 0;
	}
	std::shared_ptr<client> client::init()
	{
		assert(g_thdid == m_thdid);
		assert(m_fd != INVALID);
		auto _this = shared_from_this();
		asyncnet::get().set_client(
			m_fd, [=](auto, auto)
			{ _this->recv(); });
		asyncnet::get().set_clientclose(
			m_fd, [=](auto, auto)
			{ _this->clear(); });
		return _this;
	}
	void client::clear()
	{
		LOG_CORE("close clear fd:%d", m_fd);
		m_fd = INVALID;
	}
}