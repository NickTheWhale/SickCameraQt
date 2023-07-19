#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>
#include "..\Fingerprint.h"
#include <qelapsedtimer.h>
#include <qrandom.h>

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;

	start(QThread::Priority::HighPriority);
	return true;
}

void PlcThread::newFrameset(const Frameset::frameset_t& fs)
{
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
		Frameset::frameset_t fs = fsBuff;
		uploadDB();

		msleep(QRandomGenerator::global()->bounded(1, 100));
		qint64 time = timer.restart();
		emit addTime(static_cast<int>(time));
	}
}

void PlcThread::uploadDB()
{
	const char* data = "user data";
	int size = sizeof(data);

	client->Upload(Block_SDB, 0, &data, &size);
}

