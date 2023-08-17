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
		frameset::Frameset fs_raw;
		if (!camera->getNextFrameset(fs_raw))
		{
			qWarning() << "capture thread: failed to get frameset";
			continue;
		}

		if (!frameset::isValid(fs_raw))
		{
			qWarning() << "capture thread: invalid frameset";
			continue;
		}

		if (fs_raw.depth.number <= prevNumber)
		{
			qWarning() << "capture thread: received old frameset";
			continue;
		}

		prevNumber = fs_raw.depth.number;

		// apply plc filters
		frameset::Frameset fs_plc = fs_raw;
		cv::Mat depthMat = frameset::toMat(fs_plc.depth);
		filterManager.applyFilters(depthMat);
		fs_plc.depth = frameset::toFrame(depthMat);

		threadInterface.pushPlcFrame(fs_plc);
		threadInterface.pushRawFrame(fs_raw);
		qint64 time = cycleTimer.restart();
		emit addTime(static_cast<int>(time));
	}

	camera->stopCapture();
}

void CaptureThread::setFilters(const QJsonArray& filters)
{
	QMutexLocker locker(&filterMutex);
	filterManager.makeFilters(filters);
}
