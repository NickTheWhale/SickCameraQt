#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>
#include "Fingerprint.h"
#include "MutexTryLocker.h"
#include <qelapsedtimer.h>
#include <qrandom.h>

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;

	start(QThread::Priority::HighPriority);
	return true;
}

void PlcThread::newFrameset(Frameset::frameset_t fs)
{
	MutexTryLocker locker(&framesetMutex);
	if (!locker.isLocked())
	{
		qDebug() << "PLC THREAD NEW FRAMESET NO LOCK";
		return;
	}
	fsBuff = fs;
}

void PlcThread::stopPlc()
{
	_stop = true;
}

void PlcThread::run()
{
	uint32_t lastNumber = 0;
	QElapsedTimer timer;
	timer.start();
	while (!_stop)
	{
		MutexTryLocker locker(&framesetMutex);
		if (!locker.isLocked())
		{
			qDebug() << "PLC THREAD RUN NO LOCK";
			msleep(1);
			continue;
		}
		Frameset::frameset_t fs = fsBuff;
		locker.unlock();
		uploadDB();

		msleep(QRandomGenerator::global()->bounded(10, 50));
		qint64 time = timer.restart();
		emit addPlcTime(static_cast<int>(time));
	}
}

void PlcThread::uploadDB()
{
	const char* data = "user data";
	int size = sizeof(data);

	client->Upload(Block_SDB, 0, &data, &size);
}

