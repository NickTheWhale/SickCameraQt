#pragma once
#include <qthread.h>
#include <snap7.h>

class PlcThread : public QThread
{
	Q_OBJECT

public:
	bool startPlc(TS7Client* client);
	void stop();

protected:
	void run() override;

private:
	bool readData();
	bool readCpuInfo(TS7CpuInfo &info);

	volatile bool _stop = false;
	TS7Client* client;
};

