#ifndef __MYTIME_H__
#define __MYTIME_H__
#include <chrono>

namespace ashan
{
	using namespace std::chrono;
	struct mytime
	{
		static auto now()
		{			
			return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		}
	};
}
#endif