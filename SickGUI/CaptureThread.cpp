#include "CaptureThread.h"

#include "Camera.h"


bool CaptureThread::startCapture(Camera* camera)
{
	if (isRunning())
		return true;

	if (!camera || !camera->open())
		return false;

	this->camera = camera;
	this->stop = false;

	if (!this->camera->startCapture())
		return false;

	start(QThread::Priority::NormalPriority);

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
		if (!camera->getNextFrameset(lastFrameset))
		{

			//++retryCounter;
			//if (retryCounter * frameRetryDelay > maxRetryTime)
			//{
			//	retryCounter = 0;
			//	stop = true;
			//	emit lostConnection();
			//}

			QThread::msleep(frameRetryDelay);
			continue;
		}

		emit newFrameset(lastFrameset);
	}

	camera->stopCapture();
}
