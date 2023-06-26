#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>
#include "Fingerprint.h"

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;
	framesetBuffer = boost::circular_buffer<Frameset::frameset_t>(framesetBufferSize);

	start(QThread::Priority::TimeCriticalPriority);
	return true;
}

void PlcThread::newFrameset(Frameset::frameset_t fs)
{
	if (!framesetMutex.tryLock())
	{
		//qDebug() << "plc thread could not acquire lock";
		return;
	}

	framesetBuffer.push_back(fs);
	framesetMutex.unlock();
}

void PlcThread::stopPlc()
{
	_stop = true;
}

void PlcThread::run()
{
	Frameset::frameset_t fs;
	while (!_stop)
	{
		if (!framesetMutex.tryLock())
		{
			continue;
		}
		if (framesetBuffer.empty())
		{
			framesetMutex.unlock();
			continue;
		}

		fs = framesetBuffer.back();

		framesetMutex.unlock();

		auto fp = Fingerprint::calculateFingerprint(fs.width, fs.height, fs.depth);
		//qDebug() << "Fingerprint:" << fp << "frame #:" << fs.number;

		msleep(1000);
	}
}

void PlcThread::uploadDB()
{
	const char* data = "user data";
	int size = sizeof(data);

	client->Upload(Block_SDB, 0, &data, &size);
}

