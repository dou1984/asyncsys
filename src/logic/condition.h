#ifndef __CONDITION_H__
#define __CONDITION_H__
#include <functional>
#include "../sp.h"

namespace ashan
{
	struct condition : sp<condition>
	{
		using fcb = std::function<int()>;
		std::vector<int> m_state;
		fcb m_success;
		fcb m_failed;

		condition(int c);
		int set(int, int);
		int set(fcb &&success, fcb &&failed);
	};
}
#endif
