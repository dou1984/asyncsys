#include "thdid.h"

namespace ashan
{
	std::atomic_int thdid::idx(0);

	thdid::thdid()
	{
		m_index = idx++;
	}
	thdid &thdid::instance()
	{
		static thread_local thdid _ins;
		return _ins;
	}
}