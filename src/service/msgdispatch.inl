#include <atomic>
#include <unordered_map>
#include "msgdispatch.h"

namespace ashan
{
	template <class _ID>
	void msgdispatch<_ID>::set_method(const _ID &_cmd)
	{
		m_method.insert(_cmd);
	}
	template <class _ID>
	bool msgdispatch<_ID>::has_method(const _ID &_cmd)
	{
		return m_method.find(_cmd) != m_method.end();
	}
	template <class _ID>
	void msgdispatch<_ID>::onevent(rpccb &&_cb)
	{
		assert(m_dispatch_init == FALSE);
		m_callback = std::move(_cb);
	}
	template <class _ID>
	void msgdispatch<_ID>::onevent(const _ID &cmd, rpccb &&_cb)
	{
		assert(m_dispatch_init == FALSE);
		m_dispatch.emplace(cmd, std::move(_cb));
	}
	template <class _ID>
	void msgdispatch<_ID>::on_dispatch(msgsession *_cli, const _ID &_cmd, int _e, const char *_d, int _s)
	{
		if (auto it = m_dispatch.find(_cmd); it != m_dispatch.end())
		{
			m_dispatch_init = TRUE;
			msg_results r;
			r.__set(_e, _s, _d, _cli);
			__TRY()
			it->second(&r);
			__CATCH()
		}
		else
		{
			_cli->on_dispatch(_cli, _e, _d, _s);
		}
		_cli->send();
	}
	template <class _ID>
	void msgdispatch<_ID>::on_dispatchex(msgsession *_cli, const _ID &_cmd, int _e, const char *_d, int _s)
	{
		if (has_method(_cmd))
		{
			_cli->on_dispatch(_cli, _e, _d, _s);
		}
		else
		{
			m_dispatch_init = TRUE;
			msg_results r;
			r.__set(_e, _s, _d, _cli);
			__TRY()
			m_callback(&r);
			__CATCH()
		}
		_cli->send();
	}
}