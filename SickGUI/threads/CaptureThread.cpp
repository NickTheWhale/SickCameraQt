#include "CaptureThread.h"

#include <qelapsedtimer.h>
#include <ThreadInterface.h>

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
		Frameset::frameset_t fs;
		if (!camera->getNextFrameset(fs))
		{
			qWarning() << "capture thread: failed to get frameset";
			continue;
		}

		if (fs.number > prevNumber)
		{
			prevNumber = fs.number;

			if (maskEnabled)
				applyMask(fs, maskNorm);

			threadInterface.pushPlcFrame(fs);
			threadInterface.pushGuiFrame(fs);
			threadInterface.pushWebFrame(fs);
		}
		qint64 time = cycleTimer.restart();
		emit addTime(static_cast<int>(time));
	}

	camera->stopCapture();
}

void CaptureThread::applyMask(Frameset::frameset_t& fs, const QRectF& maskNorm)
{
	if (maskNorm.isEmpty())
		return;

	// un-normalize the mask
	QPoint topLeft = QPoint(maskNorm.topLeft().x() * fs.width, maskNorm.topLeft().y() * fs.height);
	QPoint bottomRight = QPoint(maskNorm.bottomRight().x() * fs.width, maskNorm.bottomRight().y() * fs.height);

	QRect mask(topLeft, bottomRight);

	// apply mask
	for (int x = 0; x < fs.width; ++x)
		for (int y = 0; y < fs.height; ++y)
			if (!mask.contains(x, y))
				fs.depth[y * fs.width + x] = 0;
}

void CaptureThread::setMask(const QRectF& maskNorm)
{
	QMutexLocker locker(&maskMutex);
	this->maskNorm = maskNorm;
	qDebug() << __FUNCTION__ << "new mask:" << this->maskNorm;
}

void CaptureThread::setEnableMask(const bool enable)
{
	QMutexLocker locker(&maskEnabledMutex);
	maskEnabled = enable;
	qDebug() << __FUNCTION__ << "setting mask:" << enable;
}
