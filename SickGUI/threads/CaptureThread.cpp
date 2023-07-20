#include "CaptureThread.h"

#include "..\Camera.h"
#include <qelapsedtimer.h>
#include <BufferManager.h>

bool CaptureThread::startCapture(Camera* camera)
{
	if (isRunning())
		return true;

	if (!camera || !camera->open())
		return false;

	this->camera = camera;
	this->_stop = false;

	if (!this->camera->startCapture())
		return false;

	start(QThread::Priority::TimeCriticalPriority);
	return true;
}

void CaptureThread::stopCapture()
{
	this->_stop = true;
}

void CaptureThread::run()
{
	BufferManager& bufferManager = BufferManager::instance();
	uint32_t prevNumber = 0;
	uint64_t totalFrames = 0;
	uint64_t missedFrames = 0;
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		Frameset::frameset_t fs;
		if (!camera->getNextFrameset(fs))
		{
			QThread::msleep(1);
			continue;
		}

		if (fs.number > prevNumber)
		{
			if (prevNumber != 0)
			{
				totalFrames += fs.number - prevNumber;
			}
			if (fs.number > prevNumber + 1 && prevNumber != 0)
			{
				missedFrames += fs.number - prevNumber;
				//qDebug() << "CAPTURE THREAD MISSED" << fs.number - prevNumber << "FRAME(S)" << "fs.number:" << fs.number << "prevNumber:" << prevNumber;
			}

			prevNumber = fs.number;
			bufferManager.pushPlcFrame(fs);
			bufferManager.pushGuiFrame(fs);
			bufferManager.pushWebFrame(fs);
		}
		qint64 time = cycleTimer.restart();
		emit addTime(static_cast<int>(time));
	}

	qDebug() << "MISSED FRAME PERCENTAGE:" << (missedFrames / static_cast<double>(totalFrames)) * 100.0f;

	camera->stopCapture();
}
