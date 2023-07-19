#pragma once
#include <qmutex.h>

class MutexTryLocker
{
public:
	MutexTryLocker(QMutex* mutex);
	~MutexTryLocker();
	bool isLocked() const;
	void unlock() const;
private:
	bool _locked;
	QMutex* _mutex;
};

