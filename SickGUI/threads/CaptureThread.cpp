#include "CaptureThread.h"

#include <ThreadInterface.h>

#include <qelapsedtimer.h>
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
			emit disconnected();
			qWarning() << "capture thread: failed to get frameset";
			continue;
		}
			
		emit reconnected();

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
		frameset::Frameset fs_filtered = fs_raw;
		cv::Mat depthMat = frameset::toMat(fs_filtered.depth);

		if (filterManager.applyFilters(depthMat))
		{
			emit filtersApplied(getFiltersJson());
			
			const frameset::Frame depth = frameset::toFrame(depthMat);
			fs_filtered.depth.data = depth.data;
			fs_filtered.depth.height = depth.height;
			fs_filtered.depth.width = depth.width;

			threadInterface.pushFilteredFrame(fs_filtered);
		}
		else
		{
			emit filtersFailed();
		}

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

const QJsonArray CaptureThread::getFiltersJson()
{
	QMutexLocker locker(&filterMutex);
	return filterManager.filtersJson();
}
