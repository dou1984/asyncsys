#include "thdid.h"

namespace ashan
{
	std::atomic_int thdid::idx(0);

	thdid::thdid()
	{
		m_index = idx++;
	}
}