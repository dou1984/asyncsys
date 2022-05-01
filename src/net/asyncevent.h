#ifndef __ASYNCEVENT_H__
#define __ASYNCEVENT_H__
#include <string>
#include <map>
#include <functional>
#include "../rwlock.h"
#include "../singleton.h"

namespace ashan
{
	class asyncevent : public singleton<asyncevent>
	{
		using fcb = std::function<void(const void *)>;
		std::map<std::string, fcb> m_events;
		rwlock m_lock;

	public:
		void onevent(const std::string &, fcb &&);
		void emit(const std::string &, const void *);
	};
}
#endif