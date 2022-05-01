#ifndef __THDID_H__
#define __THDID_H__
#include <atomic>
#include "thdlocal.h"

namespace ashan
{
#define g_thdid thdid::get().data()

	class thdid final : public thdlocal<thdid>
	{
		static std::atomic_int idx;
		int m_index = 0;

	public:
		thdid();
		int max() const { return idx; }
		int data() const { return m_index; }
	};
}
#endif
