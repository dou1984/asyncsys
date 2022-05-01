#ifndef __RWLOCK_H__
#define __RWLOCK_H__

#include <pthread.h>

namespace ashan
{
	class rwlock final
	{
		pthread_rwlock_t m_rwlock;

	public:
		rwlock();
		~rwlock();
		rwlock(const rwlock &) = delete;
		rwlock(rwlock &&) = delete;
		int rdlock();
		int unlock();
		int wrlock();
		int tryrdlock();
		int trywrlock();
	};
}
#endif