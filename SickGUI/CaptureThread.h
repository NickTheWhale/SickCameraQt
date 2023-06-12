#pragma once

#include <qthread.h>
#include "SickGUI.h"

#include "VisionaryFrameset.h"

class Camera;

class CaptureThread : public QThread
{
	Q_OBJECT

signals:
	void newFrameset(Frameset fs);
	void lostConnection();

public:
	bool startCapture(Camera *camera);
	void stopCapture();

protected:
	void run() override;

private:
	Frameset lastFrameset;
	Camera *camera;
	volatile bool stop;

	unsigned int frameRetryDelay = 10; /* ms */
	uint32_t retryCounter = 0;
	unsigned int maxRetryTime = 2000; /* ms, must be greater than frameRetryDelay*/
};
