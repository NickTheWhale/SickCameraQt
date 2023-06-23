#pragma once
#include <qthread.h>
#include <snap7.h>
#include "VisionaryFrameset.h"
#include <boost/circular_buffer.hpp>
#include <qmutex.h>

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
	void uploadDB();
	volatile bool _stop = false;
	TS7Client* client;

	const size_t framesetBufferSize = 2;
	boost::circular_buffer<Frameset::frameset_t> framesetBuffer;
	QMutex framesetMutex;
};

