#pragma once
#include <qthread.h>
#include <qimage.h>
#include <qmutex.h>
#include "AutoWebSocket.h"
#include "..\VisionaryFrameset.h"

class WebThread : public QThread
{
	Q_OBJECT

signals:
	void addTime(const int time);

public slots:
	void newFrameset(const Frameset::frameset_t& fs);

public:
	bool startWeb(AutoWebSocket* socket);
	void stopWeb();

protected:
	void run() override;

private:
	volatile bool _stop = false;
	Frameset::frameset_t fsBuff;
	qint64 minCycleTime = 100;
	AutoWebSocket* socket = nullptr;

	const QByteArray buildImagePacket(const Frameset::frameset_t& fs);
};

