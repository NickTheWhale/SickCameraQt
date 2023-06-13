#include "PlcThread.h"
#include "qdebug.h"

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;

	start(QThread::TimeCriticalPriority);
	return true;
}

void PlcThread::newFrameset(Frameset::frameset_t fs)
{
	//qDebug() << "PlcThread received a new frameset #" << fs.number;
}

void PlcThread::stopPlc()
{
	_stop = true;
}

void PlcThread::run()
{
	while (!_stop)
	{
		QThread::msleep(100);
	}
}


