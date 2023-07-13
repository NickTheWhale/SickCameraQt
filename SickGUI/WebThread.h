#pragma once
#include <qthread.h>
#include <qimage.h>
#include <qmutex.h>
#include "AutoWebSocket.h"

class WebThread : public QThread
{
	Q_OBJECT

public slots:
	void newImage(QImage image);

public:
	bool startWeb(AutoWebSocket* socket);
	void stopWeb();

protected:
	void run() override;

private:
	AutoWebSocket* socket = nullptr;
	volatile bool _stop = false;
	QImage lastImage;
	QMutex lastImageMutex;
};

