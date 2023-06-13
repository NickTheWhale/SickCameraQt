#pragma once
#include <qthread.h>
#include <snap7.h>
#include "VisionaryFrameset.h"

class PlcThread : public QThread
{
	Q_OBJECT


public slots:
	void newFrameset(Frameset::frameset_t fs);

public:
	bool startPlc(TS7Client* client);
	void stopPlc();

protected:
	void run() override;

private:

	volatile bool _stop = false;
	TS7Client* client;
};

