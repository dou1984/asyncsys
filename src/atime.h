#ifndef __ATIME_H__
#define __ATIME_H__
#include <chrono>

namespace ashan
{
	using namespace std::chrono;
	struct atime
	{
		static auto now()
		{			
			return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		}
	};
}
#endif