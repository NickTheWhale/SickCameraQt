#include "PlcThread.h"

bool PlcThread::startPlc(TS7Client* client)
{
	this->client = client;
	return true;
}

void PlcThread::stop()
{
	if (client)
	{
		delete client;
		client = nullptr;
	}
}

void PlcThread::run()
{
	while (!_stop)
	{

		QThread::msleep(100);
	}
}

bool PlcThread::readData()
{

	return false;
}

bool PlcThread::readCpuInfo(TS7CpuInfo& info)
{
	return false;
}
