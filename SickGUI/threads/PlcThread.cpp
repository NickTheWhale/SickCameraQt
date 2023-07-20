#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>
#include "..\Fingerprint.h"
#include <qelapsedtimer.h>
#include <qrandom.h>
#include <BufferManager.h>

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;

	start(QThread::Priority::HighPriority);
	return true;
}
void PlcThread::stopPlc()
{
	_stop = true;
}

void PlcThread::run()
{
	BufferManager& bufferManager = BufferManager::instance();
	uint32_t lastNumber = 0;
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		Frameset::frameset_t fs = bufferManager.popPlcFrame();
		
		if (!fs.isNull())
		{
			uploadDB();
		}

		const qint64 timeLeft = minimumTargetCycleTime - cycleTimer.elapsed();
		if (timeLeft > 0)
		{
			msleep(timeLeft);
		}

		qint64 time = cycleTimer.restart();
		emit addTime(static_cast<int>(time));
	}
}

void PlcThread::uploadDB()
{
	const char* data = "user data";
	int size = sizeof(data);

	client->Upload(Block_SDB, 0, &data, &size);
}

