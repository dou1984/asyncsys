#include "../invalid.h"
#include "condition.h"

namespace ashan
{
	condition::condition(int c)
	{
		m_state.resize(c);
		for (auto &it : m_state)
		{
			it = INVALID;
		}
	}
	int condition::set(int pos, int state)
	{
		if (pos >= 0 && pos < m_state.size())
		{
			m_state[pos] = state;
		}
		bool r = true;
		for (auto &it : m_state)
		{
			if (it == INVALID)
				return 0;
			else if (it == 0)
				r = false;
		}
		return r ? m_success() : m_failed();
	}
	int condition::set(fcb &&success, fcb &&failed)
	{
		m_success = std::move(success);
		m_failed = std::move(failed);
		return 0;
	}
}