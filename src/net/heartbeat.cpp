#include "asyncnet.h"
#include "heardbeat.h"

namespace ashan
{
	const int g_timeout = 15 * 1000;
	__heartbeat::__heartbeat()
	{
		m_time = asyncnet::get().set_timer(
			g_timeout, g_timeout, [this](auto, auto)
			{ ontime(); });
	}
	__heartbeat::~__heartbeat()
	{
		asyncnet::get().stop_timer(m_time);
	}
	void __heartbeat::tick()
	{
		m_count++;
	}
	void __heartbeat::ontime()
	{
		if (m_count > 0)
		{
			m_count = 0;
			onsend();
		}
		else if (m_count == 0)
		{
			m_count = INVALID;
			onsend();
		}
		else
		{
			onend();
		}
	}
}