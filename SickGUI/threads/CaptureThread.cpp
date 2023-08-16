#include "CaptureThread.h"

#include <qelapsedtimer.h>
#include <ThreadInterface.h>
#include <qdebug.h>

bool CaptureThread::startCapture(VisionaryCamera* camera)
{
	if (isRunning())
		return true;

	if (!camera || !camera->open())
		return false;

	this->camera = camera;

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
	ThreadInterface& threadInterface = ThreadInterface::instance();
	uint32_t prevNumber = 0;
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		QThread::msleep(1);
		frameset::Frameset fs;
		if (!camera->getNextFrameset(fs))
		{
			qWarning() << "capture thread: failed to get frameset";
			continue;
		}

		if (!frameset::isValid(fs))
		{
			qWarning() << "capture thread: invalid frameset";
			continue;
		}

		if (fs.depth.number > prevNumber)
		{
			prevNumber = fs.depth.number;

			if (maskEnabled)
			{
				frameset::mask(fs, maskNorm);
			}

			// apply plc filters
			frameset::Frameset plcFs = fs;
			cv::Mat depthMat = frameset::toMat(plcFs.depth);
			filterManager.applyFilters(depthMat);
			plcFs.depth = frameset::toFrame(depthMat);
			threadInterface.pushPlcFrame(plcFs);

			threadInterface.pushGuiFrame(fs);
			threadInterface.pushWebFrame(fs);
		}
		else
		{
			qDebug() << "capture thread: received old frameset";
		}
		qint64 time = cycleTimer.restart();
		emit addTime(static_cast<int>(time));
	}

	camera->stopCapture();
}

void CaptureThread::setMask(const QRectF& maskNorm)
{
	QMutexLocker locker(&maskMutex);
	this->maskNorm = maskNorm;
}

void CaptureThread::setEnableMask(const bool enable)
{
	QMutexLocker locker(&maskEnabledMutex);
	maskEnabled = enable;
}

void CaptureThread::setFilters(const QJsonArray& filters)
{
	QMutexLocker locker(&filterMutex);
	filterManager.makeFilters(filters);
}
