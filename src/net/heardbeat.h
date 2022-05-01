#ifndef __HEARDBEATH_H__
#define __HEARDBEATH_H__
#include <functional>
#include "../boolean.h"
#include "../log.h"
#include "asyncnet.h"

namespace ashan
{
	struct __heartbeat
	{
		int m_time = INVALID;
		int m_count = 1;
		std::function<void()> onsend;
		std::function<void()> onend;

		__heartbeat();
		virtual ~__heartbeat();

		void tick();
		void ontime();
	};
	template <class _CLS>
	struct heartbeat : __heartbeat
	{
		static std::function<void(_CLS *)> oninit;
	};
	template <class _CLS>
	std::function<void(_CLS *)> heartbeat<_CLS>::oninit = [](auto) {};

	template <class _CLS, class _MSG>
	int heartbeat_init()
	{
		_CLS::oninit = [](auto s)
		{
			s->onsend = [=]()
			{
				_MSG _msg;
				_msg.len = 0;
				s->send((char *)&_msg, sizeof(_msg));
			};
			s->onend = [=]()
			{ s->stop(); };
		};
		LOG_DBG("init %s", "begin");
		return 0;
	}
}

#endif