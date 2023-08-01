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

	const QPoint maskTopLeft = QPoint(maskNorm.topLeft().x() * fs.width, maskNorm.topLeft().y() * fs.height);
	const QPoint maskBottomRight = QPoint(maskNorm.bottomRight().x() * fs.width, maskNorm.bottomRight().y() * fs.height);

	const QRect mask = QRect(maskTopLeft, maskBottomRight);
	const size_t maskedWidth = mask.width();
	const size_t maskedHeight = mask.height();

	std::vector<uint16_t> maskedDepth;
	std::vector<uint16_t> maskedIntensity;
	std::vector<uint16_t> maskedState;

	//maskedDepth.reserve(maskedWidth * maskedHeight);
	//maskedIntensity.reserve(maskedWidth * maskedHeight);
	//maskedState.reserve(maskedWidth * maskedHeight);

	for (size_t y = maskTopLeft.y(); y < maskTopLeft.y() + maskedHeight; ++y)
	{
		for (size_t x = maskTopLeft.x(); x < maskTopLeft.x() + maskedWidth; ++x)
		{
			const int index = y * fs.width + x;
			maskedDepth.push_back(fs.depth[index]);
			maskedIntensity.push_back(fs.intensity[index]);
			maskedState.push_back(fs.state[index]);
		}
	}
	fs.height = maskedHeight;
	fs.width = maskedWidth;
	fs.depth = maskedDepth;
	fs.intensity = maskedIntensity;
	fs.state = maskedState;

#pragma region OLD_MASKING_ALGORITHM
	//if (maskNorm.isEmpty())
	//	return;

	//// un-normalize the mask
	//QPoint maskTopLeft = QPoint(maskNorm.maskTopLeft().x() * fs.width, maskNorm.maskTopLeft().y() * fs.height);
	//QPoint maskBottomRight = QPoint(maskNorm.maskBottomRight().x() * fs.width, maskNorm.maskBottomRight().y() * fs.height);

	//QRect mask(maskTopLeft, maskBottomRight);

	//// apply mask
	//for (int x = 0; x < fs.width; ++x)
	//	for (int y = 0; y < fs.height; ++y)
	//		if (!mask.contains(x, y))
	//			fs.depth[y * fs.width + x] = 0;
#pragma endregion
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
