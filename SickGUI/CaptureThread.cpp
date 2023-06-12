#include "CaptureThread.h"

#include "Camera.h"


bool CaptureThread::startCapture(Camera* camera)
{
	if (isRunning())
		return false;

	if (!camera || !camera->open())
		return false;

	this->camera = camera;
	this->stop = false;

	if (!this->camera->startCapture())
		return false;

	start(QThread::TimeCriticalPriority);

	return true;
}

void CaptureThread::stopCapture()
{
	this->stop = true;
}

void CaptureThread::run()
{
	while (!stop)
	{
		if (!camera->getNextImage(lastFrame))
		{

			++retryCounter;
			if (retryCounter * frameRetryDelay > maxRetryTime)
			{
				retryCounter = 0;
				stop = true;
				emit lostConnection();
			}

			QThread::msleep(frameRetryDelay);
			continue;
		}

		emit newImage(lastFrame);
	}

	camera->stopCapture();
}
