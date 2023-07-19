#include "MutexTryLocker.h"

MutexTryLocker::MutexTryLocker(QMutex* mutex) : _mutex(mutex), _locked(mutex->tryLock())
{
}

MutexTryLocker::~MutexTryLocker()
{
	if (_locked)
	{
		_mutex->unlock();
	}
}

bool MutexTryLocker::isLocked() const
{
	return _locked;
}

void MutexTryLocker::unlock() const
{
	if (_locked)
	{
		_mutex->unlock();
	}
}
