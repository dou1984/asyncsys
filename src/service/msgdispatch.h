#ifndef __MSGWAITING_H__
#define __MSGWAITING_H__
#include <atomic>
#include <assert.h>
#include <set>
#include "../boolean.h"
#include "../singleton.h"
#include "msgsession.h"

namespace ashan
{
	template <class _ID>
	struct msgdispatch : singleton<msgdispatch<_ID>>
	{
		using msg_results = msgsession::msg_results;
		using rpccb = msgsession::rpccb;
		std::atomic_int m_dispatch_init = FALSE;
		std::unordered_map<_ID, rpccb> m_dispatch;
		void onevent(const _ID &cmd, rpccb &&_cb);
		void on_dispatch(msgsession *_cli, const _ID &_cmd, int _e, const char *_d, int _s);

		rpccb m_callback;
		std::set<_ID> m_method;
		void set_method(const _ID &_cmd);		
		bool has_method(const _ID &_cmd);		
		void onevent(rpccb &&_cb);
		void on_dispatchex(msgsession *_cli, const _ID &_cmd, int _e, const char *_d, int _s);
	};
}
#include "msgdispatch.inl"
#endif
