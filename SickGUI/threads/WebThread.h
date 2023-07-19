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
	AutoWebSocket* socket = nullptr;
	volatile bool _stop = false;
	Frameset::frameset_t fsBuff;

	const QByteArray buildImagePacket(const Frameset::frameset_t& fs);
};

