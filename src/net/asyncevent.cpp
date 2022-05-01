#include <assert.h>
#include "../mythrow.h"
#include "../log.h"
#include "asyncevent.h"

namespace ashan
{
	void asyncevent::onevent(const std::string &_str, fcb &&_callback)
	{
		auto r = m_lock.wrlock();
		assert(r == 0);
		m_events.emplace(_str, std::move(_callback));
		m_lock.unlock();
	}
	void asyncevent::emit(const std::string &_str, const void *_data)
	{
		auto r = m_lock.rdlock();
		assert(r == 0);
		if (auto it = m_events.find(_str); it != m_events.end())
		{
			__TRY()
			it->second(_data);
			__CATCH()
		}
		m_lock.unlock();
	}
}