#include "PlcThread.h"
#include "qdebug.h"
#include <algorithm>
#include <snap7.h>

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
		qDebug() << "plc thread could not acquire lock";
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
	Frameset::frameset_t oldFs;
	while (!_stop)
	{
		if (!framesetMutex.tryLock())
		{
			qDebug() << "plc thread could not acquire lock";
			continue;
		}
		if (framesetBuffer.empty())
		{
			framesetMutex.unlock();
			continue;
		}

		framesetMutex.unlock();

		Frameset::frameset_t fs = framesetBuffer.back();

		double avgDepth = std::accumulate(fs.depth.begin(), fs.depth.end(), 0.0) / fs.depth.size();
		
		

		if (oldFs.time != fs.time)
		{
			if (fs.number - oldFs.number > 1)
			{
				qDebug() << "########missed" << fs.number - oldFs.number << "frame(s)";
			}
			oldFs = fs;
		}

		uploadDB();

		msleep(1000);
	}
}

void PlcThread::uploadDB()
{
	const char* data = "user data";
	int size = sizeof(data);

	client->Upload(Block_SDB, 0, &data, &size);
}

