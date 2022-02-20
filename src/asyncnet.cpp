#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include "asyncnet.h"
#include "log.h"
#include "thdid.h"
#include "mythrow.h"

#define MAX_EPOLL_EVENTS (1000)
#define SPLIT(S)               \
	int fd = ((S)&0xffffffff); \
	int state = ((S) >> 32);

namespace ashan
{
	static const int64_t g_msec = 1000;
	asyncnet *asyncnet::m_thdset[] = {0};

	template <class _VEC>
	auto &get_wbuffer(_VEC &vec, int fd)
	{
		assert(fd >= 0);
		if (fd >= vec.size())
		{
			vec.resize(fd + 1);
		}
		return vec[fd];
	}
	asyncnet::asyncnet()
	{
		m_epfd = epoll_create1(EPOLL_CLOEXEC);
		if (m_epfd == INVALID)
		{
			throw("epoll_create1 error");
		}
		m_eventfd = eventfd(0, EFD_NONBLOCK);
		if (m_eventfd == INVALID)
		{
			throw("eventfd error");
		}
		set_noblock(m_eventfd);
		add_event(m_eventfd, e_state_event, EPOLLIN | EPOLLET);

		assert(g_thdid < MAX_THREAD_COUNT);
		m_thdset[g_thdid] = this;
		LOG_DBG("asyncnet epoll_create1 success %d", m_epfd);
	}
	asyncnet::~asyncnet()
	{
		del_event(m_eventfd);
		::close(m_eventfd);
		::close(m_epfd);
	}
	void asyncnet::set_noblock(int __fd)
	{
		if (fcntl(__fd, F_SETFL, O_NONBLOCK | fcntl(__fd, F_GETFL)) == INVALID)
		{
			LOG_DBG("fcntl error fd:%d err:%d", __fd, errno);
		}
	}
	void asyncnet::set_reuseaddr(int __fd)
	{
		int reuse = 1;
		setsockopt(__fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	}
	int asyncnet::getsockerr(int __fd)
	{
		int err = 0;
		socklen_t len = sizeof(socklen_t);
		return getsockopt(__fd, SOL_SOCKET, SO_ERROR, (char *)&err, &len) == INVALID ? INVALID : err;
	}
	void asyncnet::add_event(int __fd, int __state, int __events)
	{
		assert(__fd != INVALID);
		struct epoll_event ev;
		ev.events = __events;
		ev.data.u64 = __merge(__fd, __state);
		if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, __fd, &ev) == INVALID)
		{
			LOG_DBG("epoll_ctl_add epfd:%d fd:%d err:%d %s", m_epfd, __fd, errno, strerror(errno));
		}
	}
	void asyncnet::mod_event(int __fd, int __state, int __events)
	{
		assert(__fd != INVALID);
		struct epoll_event ev;
		ev.events = __events;
		ev.data.u64 = __merge(__fd, __state);
		if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, __fd, &ev) == INVALID)
		{
			LOG_DBG("epoll_ctl_mod epfd:%d fd:%d err:%d %s", get_epfd(), __fd, errno, strerror(errno));
		}
	}
	void asyncnet::del_event(int __fd)
	{
		assert(__fd != INVALID);
		struct epoll_event ev;
		ev.data.fd = 0;
		ev.events = 0;
		if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, __fd, &ev) == INVALID)
		{
			LOG_DBG("epoll_ctl_del epfd:%d fd:%d err:%d %s", get_epfd(), __fd, errno, strerror(errno));
		}
	}
	void asyncnet::__emplace_back(std::vector<fcb> &vec, int fd, fcb &&obj)
	{
		assert(fd >= 0);
		if (fd >= vec.size())
			vec.resize(fd + 1);
		assert(vec[fd] == nullptr);
		vec[fd] = std::move(obj);
	}
	void asyncnet::__push_back(std::vector<fcb> &vec, int fd, const fcb &obj)
	{
		assert(fd >= 0);
		if (fd >= vec.size())
			vec.resize(fd + 1);
		// assert(vec[fd] == nullptr);
		// assert(obj);
		vec[fd] = obj;
	}
	int asyncnet::get_epfd() const
	{
		return m_epfd;
	}
	int asyncnet::wait()
	{
		while (__wait() == 0)
		{
		}
		return 0;
	}
	asyncnet &asyncnet::get()
	{
		static thread_local asyncnet _this;
		return _this;
	}
	asyncnet *asyncnet::at(int _thdid)
	{
		assert(_thdid >= 0);
		assert(_thdid < thdid::instance().max());
		assert(m_thdset[_thdid]);
		return m_thdset[_thdid];
	}
	int asyncnet::__wait()
	{
		struct epoll_event __events[MAX_EPOLL_EVENTS];
		int count = epoll_wait(m_epfd, __events, MAX_EPOLL_EVENTS, INVALID);
		for (int i = 0; i < count; i++)
		{
			SPLIT(__events[i].data.u64);
			LOG_DBG("epoll_wait %d:%d", fd, state);
			__TRY()
			switch (state)
			{
			case e_state_client:
				m_flags.set(e_flag_close);
			case e_state_client_ex:
				__onclient(fd, __events[i].events);
				break;
			case e_state_timer:
			case e_state_rtimer:
				__ontimer(fd, __events[i].events, state);
				break;
			case e_state_accept:
				__onaccept(fd, __events[i].events);
				break;
			case e_state_event:
				__onevent(fd, __events[i].events);
				break;
			case e_state_connect:
				m_flags.set(e_flag_close);
			case e_state_connect_ex:
				__onconnect(fd, __events[i].events);
				break;
			default:
				break;
			}
			__CATCH()
			m_flags.reset();
		}
		return 0;
	}
	uint64_t asyncnet::__merge(int fd, int state)
	{
		return (uint64_t(state) << 32 | uint64_t(fd));
	}
	int asyncnet::__call(std::vector<fcb> &_vec, int fd)
	{
		return __call(_vec, fd, fd);
	}
	int asyncnet::__call(std::vector<fcb> &_vec, int fd, int __fd)
	{
		assert(fd < _vec.size());
		if (_vec[fd])
		{
			_vec[fd](this, __fd);
		}
		return 0;
	}
	int asyncnet::__clear(std::vector<fcb> &_vec, int fd)
	{
		if (fd < _vec.size())
		{
			_vec[fd] = nullptr;
		}
		return 0;
	}
	int asyncnet::connect(const std::string &ip, int port, fcb &&_cli, fcb &&_close)
	{
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == INVALID)
		{
			return INVALID;
		}
		set_noblock(fd);
		if (__connect(fd, ip, port) == INVALID)
		{
			if (errno == EINPROGRESS)
			{
				return __fd_add_event(fd, e_state_connect, std::move(_cli), std::move(_close));
			}
		}
		if (_close)
		{
			_close(this, fd);
		}
		::close(fd);
		return INVALID;
	}
	int asyncnet::start(const std::string &ip, int port, fcb &&_cli, fcb &&_close)
	{
		auto fd = __bind(ip, port);
		if (fd != INVALID)
		{
			return __start(fd, std::move(_cli), std::move(_close));
		}
		return fd;
	}
	int asyncnet::__bind(const std::string &ip, int port)
	{
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd == INVALID)
		{
			return INVALID;
		}
		//	绑定fd的不能为0
		assert(fd != 0);
		set_reuseaddr(fd);
		set_noblock(fd);
		if (__bind(fd, ip, port) < 0)
		{
			LOG_DBG("bind error %d:%s", errno, strerror(errno));
			::close(fd);
			return INVALID;
		}
		if (::listen(fd, 1000) < 0)
		{
			LOG_DBG("listen error %d:%s", errno, strerror(errno));
			::close(fd);
			return INVALID;
		}
		return fd;
	}
	int asyncnet::__bind(int fd, const std::string &ip, int port)
	{
		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		if (addr.sin_addr.s_addr == INADDR_NONE)
		{
			return INVALID;
		}
		return ::bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	}
	int asyncnet::__connect(int fd, const std::string &ip, int port)
	{
		struct sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(addr.sin_family, ip.c_str(), &addr.sin_addr);
		return ::connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	}
	int asyncnet::__fd_add_event(int fd, int state, fcb &&_cli, fcb &&_close)
	{
		add_event(fd, state, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP);
		__emplace_back(m_client, fd, std::move(_cli));
		__emplace_back(m_close, fd, std::move(_close));
		return fd;
	}
	int asyncnet::__start(int fd, fcb &&_cli, fcb &&_close)
	{
		std::lock_guard<std::mutex> _lock(m_mutex);
		return __fd_add_event(fd, e_state_accept, std::move(_cli), std::move(_close));
	}
	int asyncnet::set_client(int fd, fcb &&_read, fcb &&_write)
	{
		__emplace_back(m_read, fd, std::move(_read));
		__emplace_back(
			m_write, fd,
			_write ? std::move(_write) : [this](auto, auto fd)
				{ __send(fd); });
		return fd;
	}
	int asyncnet::set_clientclose(int fd, fcb &&_close)
	{
		__clear(m_close, fd);
		__emplace_back(m_close, fd, std::move(_close));
		return fd;
	}
	int asyncnet::send(int fd, std::string &&_msg)
	{
		auto &lst = get_wbuffer(m_wbuffer, fd);
		if (!lst.empty())
		{
			lst.emplace_back(std::move(_msg));
			return 0;
		}
		auto count = ::send(fd, _msg.data(), _msg.size(), 0);
		if (count == INVALID)
		{
			if (errno == EAGAIN || errno == EINPROGRESS)
			{
				mod_event(fd, e_state_client, (EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLET));
				lst.emplace_back(std::move(_msg));
			}
			else
			{
				__clientclose(fd);
			}
		}
		else
		{
			assert(count == _msg.size());
		}
		return count;
	}
	int asyncnet::set_timer(int64_t itime, int64_t rtime, fcb &&_timer)
	{
		int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
		if (fd == INVALID)
		{
			LOG_DBG("timerfd_create errno:%d errorinfo:%s", errno, strerror(errno));
			return INVALID;
		}
		return __settimer(fd, itime, rtime, std::move(_timer));
	}
	int asyncnet::stop_timer(int fd)
	{
		struct itimerspec t;
		if (timerfd_gettime(fd, &t) != INVALID)
		{
			LOG_DBG("stop_timer fd:%d", fd);
			del_event(fd);
			__clear(m_read, fd);
			::close(fd);
		}
		return 0;
	}
	int64_t asyncnet::remaining_timer(int fd)
	{
		struct itimerspec t;
		if (timerfd_gettime(fd, &t) != INVALID)
		{
			return int64_t(t.it_value.tv_sec) * 1000 + int64_t(t.it_value.tv_nsec) / 1000;
		}
		return 0;
	}
	int asyncnet::__settimer(int fd, int64_t itime, int64_t rtime, fcb &&_timer)
	{
		struct itimerspec t;
		t.it_value.tv_sec = itime / g_msec;
		t.it_value.tv_nsec = (itime % g_msec) * 1000000;
		t.it_interval.tv_sec = rtime / g_msec;
		t.it_interval.tv_nsec = (rtime % g_msec) * 1000000;
		if (timerfd_settime(fd, 0, &t, 0) < 0)
		{
			LOG_DBG("timerfd_settime errno:%d errorinfo:%s", errno, strerror(errno));
			::close(fd);
			return INVALID;
		}
		add_event(fd, rtime == 0 ? e_state_timer : e_state_rtimer, EPOLLIN);
		__emplace_back(m_read, fd, std::move(_timer));
		return fd;
	}
	int asyncnet::__ontimer(int fd, int __events, int __state)
	{
		if (__events & EPOLLIN)
		{
			itimerspec t;
			auto r = ::read(fd, &t, sizeof(t));
			if (r == INVALID)
			{
				LOG_DBG("read timefd error:%d:%s", errno, strerror(errno));
			}
			__call(m_read, fd);
		}
		if (__state == e_state_timer)
		{
			del_event(fd);
			__clear(m_read, fd);
			::close(fd);
		}
		return 0;
	}
	int asyncnet::notify(fcb &&_event)
	{
		{
			std::lock_guard<std::mutex> _(m_mutex);
			m_trigger.emplace_back(std::move(_event));
		}
		unsigned long long reserve = 1;
		if (::write(m_eventfd, &reserve, sizeof(reserve)) == INVALID)
		{
			LOG_DBG("on_event error %d:%s", errno, strerror(errno));
		}
		return 0;
	}
	int asyncnet::notify_all(const fcb &_event)
	{
		for (int i = 0; i < thdid::instance().max(); i++)
		{
			if (m_thdset[i])
			{
				auto f = _event;
				m_thdset[i]->notify(std::move(f));
			}
		}
		return 0;
	}
	int asyncnet::__onconnect(int fd, int __events)
	{
		if (__events & (EPOLLIN | EPOLLOUT))
		{
			int err = 0;
			if ((err = getsockerr(fd)) == 0)
			{
				mod_event(fd, e_state_client, (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET));
				__call(m_client, fd);
				return fd;
			}
			LOG_DBG("connect error fd:%d err:%d", fd, err);
		}
		else if (__events & (EPOLLERR | EPOLLHUP))
		{
			LOG_DBG("connect error fd:%d err:%d", fd, errno);
		}
		del_event(fd);
		__call(m_close, fd);
		__clear(m_close, fd);
		if (m_flags[e_flag_close])
		{
			::close(fd);
		}
		return INVALID;
	}
	int asyncnet::__onaccept(int fd, int __events)
	{
		if (__events & (EPOLLIN | EPOLLOUT))
		{
			std::lock_guard<std::mutex> _(m_mutex);
			assert(fd != 0);
			struct sockaddr addr = {0};
			socklen_t addrlen = sizeof(addr);
			int __fd = ::accept(fd, &addr, &addrlen);
			if (__fd != INVALID)
			{
				LOG_DBG("accept fd:%d:%d", fd, __fd);
				set_noblock(__fd);
				add_event(__fd, e_state_client, (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET));
				__call(m_client, fd, __fd);
				__push_back(m_close, __fd, m_close[fd]);
			}
			return __fd;
		}
		return 0;
	}
	int asyncnet::__onclient(int fd, int __events)
	{
		if (__events & EPOLLIN)
		{
			LOG_DBG("client read %d", fd);
			__call(m_read, fd);
		}
		if (__events & EPOLLOUT)
		{
			LOG_DBG("client send %d", fd);
			__call(m_write, fd);
		}
		if (__events & (EPOLLERR | EPOLLHUP))
		{
			LOG_DBG("client close %d %s", fd, m_flags[e_flag_close] ? "true" : "false");
			if (m_flags[e_flag_close])
			{
				__clientclose(fd);
			}
		}
		return 0;
	}
	int asyncnet::__onevent(int fd, int __events)
	{
		decltype(m_trigger) _list;
		{
			std::lock_guard<std::mutex> _(m_mutex);
			_list.swap(m_trigger);
		}
		unsigned long long reserve = 1;
		if (::read(m_eventfd, &reserve, sizeof(reserve)) == INVALID)
		{
			LOG_DBG("read event error %d:%s", errno, strerror(errno));
		}
		for (auto &it : _list)
		{
			it(this, fd);
		}
		return 0;
	}
	int asyncnet::__send(int fd)
	{
		assert(fd != INVALID);
		assert(fd < m_wbuffer.size());
		auto &lst = get_wbuffer(m_wbuffer, fd);
		while (!lst.empty())
		{
			auto &_msg = lst.front();
			if (::send(fd, _msg.data(), _msg.size(), 0) == INVALID)
			{
				if (errno == EAGAIN || errno == EINPROGRESS)
				{
					mod_event(fd, e_state_client, (EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP | EPOLLET));
					return 0;
				}
				else
				{
					return __clientclose(fd);
				}
			}
			lst.pop_front();
		}
		mod_event(fd, e_state_client, (EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET));
		return 0;
	}
	int asyncnet::__clientclose(int fd)
	{
		assert(fd != INVALID);
		del_event(fd);

		get_wbuffer(m_wbuffer, fd).clear();
		__clear(m_read, fd);
		__clear(m_write, fd);
		__clear(m_client, fd);
		__call(m_close, fd);
		__clear(m_close, fd);
		::close(fd);
		return 0;
	}
	int asyncnet::__clientclosed_already(int fd)
	{
		assert(fd != INVALID);
		get_wbuffer(m_wbuffer, fd).clear();
		__clear(m_read, fd);
		__clear(m_write, fd);
		__clear(m_client, fd);
		__clear(m_close, fd);
		return 0;
	}
}
