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
	uint32_t lastNumber = 0;
	QElapsedTimer timer;
	while (!_stop)
	{
		timer.start();
		if (tryGetFrameset(fs))
		{
			if (fs.number > lastNumber)
			{
				lastNumber = fs.number;
#pragma region LOOP

				auto fp = Fingerprint::calculateFingerprint(fs.width, fs.height, fs.depth);
				//qDebug() << "Fingerprint:" << fp << "frame #:" << fs.number;

#pragma endregion

				qint64 elapsedNs = timer.nsecsElapsed();
				double elapsedMs = elapsedNs / static_cast<double>(1'000'000);
				qDebug() << "elapsed time:" << elapsedMs << "number:" << fs.number;
			}
		}
	}
}

bool PlcThread::tryGetFrameset(Frameset::frameset_t& fs)
{
	if (!framesetMutex.tryLock())
	{
		return false;
	}

	if (framesetBuffer.empty())
	{
		framesetMutex.unlock();
		return false;
	}

	fs = framesetBuffer.back();

	framesetMutex.unlock();
	return true;
}

void PlcThread::uploadDB()
{
	const char* data = "user data";
	int size = sizeof(data);

	client->Upload(Block_SDB, 0, &data, &size);
}

