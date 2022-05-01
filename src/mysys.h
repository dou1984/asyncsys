#ifndef __MYSYS_H__
#define __MYSYS_H__
#include <signal.h>
#include <functional>
#include <thread>
#include <regex>
#include "singleton.h"

namespace ashan
{
#define g_sys ashan::mysys::get()
	struct mysys final : singleton<mysys>
	{
		using fcb = std::function<void()>;
		void ignore_signal(int __signal);
		void ignore_signal();
		void set_log_level(int);

		std::thread createthread(fcb &&);
		void fork(fcb &&, fcb &&);
		void stop();
		void splite(const std::string &, const std::string &, const std::function<void(const std::string &)> &);

		template <class... _CLS>
		bool regex_search(const std::regex &re, const std::string &s, _CLS &&..._cls)
		{
			std::smatch sm;
			if (std::regex_search(s, sm, re))
			{
				int i = 1;
				((std::forward<_CLS>(_cls) = sm[i++]), ...);
				return true;
			}
			return false;
		}
		template <class... _CLS>
		bool regex_search(const std::string &m, const std::string &s, _CLS &&..._cls)
		{
			std::regex re(m);
			return regex_search(re, s, std::forward<_CLS>(_cls)...);
		}
	};
}

#endif