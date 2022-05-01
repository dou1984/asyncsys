#include "rwlock.h"

namespace ashan
{
	rwlock::rwlock()
	{
		pthread_rwlock_init(&m_rwlock, nullptr);
	}
	rwlock::~rwlock()
	{
		pthread_rwlock_destroy(&m_rwlock);
	}
	int rwlock::rdlock()
	{
		return pthread_rwlock_rdlock(&m_rwlock);
	}
	int rwlock::unlock()
	{
		return pthread_rwlock_unlock(&m_rwlock);
	}
	int rwlock::wrlock()
	{
		return pthread_rwlock_wrlock(&m_rwlock);
	}
	int rwlock::tryrdlock()
	{
		return pthread_rwlock_tryrdlock(&m_rwlock);
	}
	int rwlock::trywrlock()
	{
		return pthread_rwlock_trywrlock(&m_rwlock);
	}
}