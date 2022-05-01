#include <sys/epoll.h>
#include "../net/asyncnet.h"
#include "../net/asyncsys.h"
#include "../log.h"
#include "rediscli.h"

namespace ashan
{
	const char STRING_NIL[] = "nil";
	const char STRING_CLOSED[] = "closed";
	const char STRING_TIMEOUT[] = "timeout";
	void __callback(redisAsyncContext *, void *__reply, void *__data)
	{
		auto _this = (rediscli *)(__data);
		_this->__oncallback((redisReply *)(__reply));
	}
	void rediscli::__addread(void *privdata)
	{
		auto _this = (rediscli *)(privdata);
		_this->m_events |= EPOLLIN;
		asyncnet::get().mod_event(_this->m_fd, asyncnet::e_state_client_ex, _this->m_events);
		LOG_CORE("fd:%d events:%x", _this->m_fd, _this->m_events);
	}
	void rediscli::__delread(void *privdata)
	{
		auto _this = (rediscli *)(privdata);
		_this->m_events &= ~(EPOLLIN);
		asyncnet::get().mod_event(_this->m_fd, asyncnet::e_state_client_ex, _this->m_events);
		LOG_CORE("fd:%d events:%x", _this->m_fd, _this->m_events);
	}
	void rediscli::__addwrite(void *privdata)
	{
		auto _this = (rediscli *)(privdata);
		_this->m_events |= EPOLLOUT;
		auto _state = (_this->m_state == e_clistate_connected) ? asyncnet::e_state_client_ex : asyncnet::e_state_connect_ex;
		asyncnet::get().mod_event(_this->m_fd, _state, _this->m_events);
		LOG_CORE("fd:%d events:%x", _this->m_fd, _this->m_events);
	}
	void rediscli::__delwrite(void *privdata)
	{
		auto _this = (rediscli *)(privdata);
		_this->m_events &= ~EPOLLOUT;
		auto _state = (_this->m_state == e_clistate_connected) ? asyncnet::e_state_client_ex : asyncnet::e_state_connect_ex;
		asyncnet::get().mod_event(_this->m_fd, _state, _this->m_events);
		LOG_CORE("fd:%d events:%x", _this->m_fd, _this->m_events);
	}
	void rediscli::__clearup(void *privdata)
	{
		auto _this = (rediscli *)(privdata);
		LOG_CORE("%p fd:%d events:%x", _this, _this->m_fd, _this->m_events);
		_this->__clear();
	}
	void rediscli::__settimeout(void *privdata, struct timeval tv)
	{
		auto _this = (rediscli *)(privdata);
		auto fd = _this->m_fd;
		auto t = tv.tv_sec * 1000 * 1000 + tv.tv_usec;
		LOG_DBG("fd:%d", fd);
	}
	void rediscli::__attach(redisAsyncContext *c)
	{
		c->ev.addRead = &rediscli::__addread;
		c->ev.delRead = &rediscli::__delread;
		c->ev.addWrite = &rediscli::__addwrite;
		c->ev.delWrite = &rediscli::__delwrite;
		c->ev.cleanup = &rediscli::__clearup;
		c->ev.scheduleTimer = &rediscli::__settimeout;
		c->ev.data = this;
		m_fd = c->c.fd;
		m_state = e_clistate_connecting;
		m_events = (EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP);

		auto &_net = asyncnet::get();
		_net.__fd_add_event(
			m_fd, asyncnet::e_state_connect_ex, [this](auto, auto)
			{ __onrecv(m_context); },
			[this](auto, auto)
			{ __onrecv(m_context); });
		_net.set_client(
			m_fd,
			[this](auto, auto)
			{ __onrecv(m_context); },
			[this](auto, auto)
			{ __onsend(m_context); });
		LOG_DBG("fd:%d events:%x", m_fd, m_events);
	}

	void rediscli::__connected(const redisAsyncContext *ac, int status)
	{
		LOG_DBG("connected %d", status);
		if (status != REDIS_OK)
		{
			auto _this = __get(ac);
			LOG_DBG("connected error fd:%d state:%d", _this->m_fd, _this->m_state);
			_this->__clear();
			_this->__retry();
		}
		else
		{
			auto _this = __get(ac);
			LOG_DBG("connected fd:%d state:%d", _this->m_fd, _this->m_state);
			_this->__onconnect();
			_this->__onquery();
		}
	}
	void rediscli::__disconnected(const redisAsyncContext *ac, int status)
	{
		LOG_DBG("disconnected %d", status);
		if (status == REDIS_OK)
		{
		}
		else
		{
			auto _this = __get(ac);
			LOG_DBG("disconnect fd:%d state:%d", _this->m_fd, _this->m_state);
			_this->__clear();
			_this->__retry();
		}
	}

	rediscli::rediscli(const std::string &addr)
	{
		if (asyncsys::splitaddr(addr, m_ip, m_port))
		{
			__init();
		}
	}
	rediscli::~rediscli()
	{
		if (m_context)
		{
			auto _context = m_context;
			__clear();
			redisAsyncFree(_context);
		}
	}
	void rediscli::query(std::string &&_query, rediscb &&_callback)
	{
		m_list.emplace_back(std::move(_query), std::move(_callback));
		if (m_list.size() == 1 && m_state == e_clistate_connected)
		{
			__onquery();
		}
	}
	void rediscli::__onquery()
	{
		if (auto it = m_list.begin(); it != m_list.end() && m_context)
		{
			redisAsyncCommand(m_context, __callback, this, it->m_query.c_str());
		}
	}
	void rediscli::__onconnect()
	{
		m_state = e_clistate_connected;
		m_events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
		asyncnet::get().mod_event(m_fd, asyncnet::e_state_client_ex, m_events);
	}
	void rediscli::__onsend(redisAsyncContext *_context)
	{
		assert(m_context);
		assert(m_context == _context);
		redisAsyncHandleWrite(_context);
	}
	void rediscli::__onrecv(redisAsyncContext *_context)
	{
		assert(m_context);
		assert(m_context == _context);
		redisAsyncHandleRead(_context);
	}
	void rediscli::__oncallback(redisReply *_reply)
	{
		rds_results r;
		if (_reply != nullptr)
		{
			LOG_DBG("type:%d elements:%ld", _reply->type, _reply->elements);
			switch (_reply->type)
			{
			case REDIS_REPLY_STRING:
				r.str = _reply->str;
				break;
			case REDIS_REPLY_ARRAY:
				r.num_rows = _reply->elements;
				r.rows = _reply->element;
				break;
			case REDIS_REPLY_NIL:
				r.last_error = INVALID;
				r.last_msg = STRING_NIL;
				break;
			case REDIS_REPLY_ERROR:
				r.last_error = INVALID;
				r.last_msg = _reply->str;
				break;
			case REDIS_REPLY_STATUS:
				r.last_error = INVALID;
				r.last_msg = _reply->str;
				break;
			case REDIS_REPLY_INTEGER:
				r.num_rows = _reply->integer;
				break;
			default:
				assert(false);
				break;
			}
		}
		else
		{
			r.last_error = INVALID;
			r.last_msg = STRING_CLOSED;
		}
		if (auto it = m_list.begin(); it != m_list.end())
		{
			r.query = std::move(it->m_query);
			auto _callback = std::move(it->m_callback);
			if (_callback)
			{
				_callback(&r);
			}
			m_list.pop_front();
			__onquery();
		}
	}
	void rediscli::__init()
	{
		assert(m_context == nullptr);
		assert(m_fd == INVALID);
		m_context = redisAsyncConnect(m_ip.c_str(), m_port);
		__attach(m_context);
		LOG_DBG("init fd:%d", m_fd);
		redisAsyncSetConnectCallback(m_context, &rediscli::__connected);
		redisAsyncSetDisconnectCallback(m_context, &rediscli::__disconnected);
	}
	void rediscli::__retry()
	{
		__retry_timeout();
		assert(m_timer == INVALID);
		m_timer = asyncnet::get().set_timer(
			m_timeout, 0,
			[this](auto, auto fd)
			{
				assert(fd == m_timer);
				m_timer = INVALID;
				__init();
			});
		LOG_DBG("retry fd:%d", m_timer);
	}
	void rediscli::__retry_timeout()
	{
		const long _timeout = MAX_RETRY_TIMEOUT;
		auto t = mytime::now();
		for (auto it = m_list.begin(); it != m_list.end();)
		{
			if (it->m_tstamp + _timeout > t)
			{
				break;
			}
			rds_results r;
			r.last_error = INVALID;
			r.last_msg = STRING_TIMEOUT;
			r.query = std::move(it->m_query);
			auto _callback = std::move(it->m_callback);
			if (_callback)
			{
				_callback(&r);
			}
			it = m_list.erase(it);
		}
	}
	void rediscli::__clear()
	{
		m_state = e_clistate_closed;
		m_context = nullptr;
		m_events = 0;
		if (m_fd == INVALID)
		{
			return;
		}
		asyncnet::get().__clientclosed_already(m_fd);
		m_fd = INVALID;
	}
}
