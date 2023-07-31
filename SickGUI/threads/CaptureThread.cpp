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
	//if (maskNorm.isEmpty())
	//	return;

	//// Calculate the dimensions of the masked region
	//int maskedWidth = static_cast<int>(maskNorm.width() * fs.width);
	//int maskedHeight = static_cast<int>(maskNorm.height() * fs.height);

	//// Allocate a new vector to store the masked depth values
	//std::vector<uint16_t> maskedDepth(maskedWidth * maskedHeight, 0);
	//std::vector<uint16_t> maskedIntensity(maskedWidth * maskedHeight, 0);
	//std::vector<uint16_t> maskedState(maskedWidth * maskedHeight, 0);

	//// Copy the depth values from the original frameset to the new maskedDepth vector
	//for (int y = 0; y < maskedHeight; ++y)
	//{
	//	for (int x = 0; x < maskedWidth; ++x)
	//	{
	//		// Calculate the normalized coordinates of the masked region
	//		double maskX = x / static_cast<double>(maskedWidth);
	//		double maskY = y / static_cast<double>(maskedHeight);

	//		// Calculate the corresponding coordinates in the original frameset
	//		int originalX = static_cast<int>(maskX * fs.width + maskNorm.topLeft().x() * fs.width);
	//		int originalY = static_cast<int>(maskY * fs.height + maskNorm.topLeft().y() * fs.height);

	//		// Calculate the corresponding index in the masked depth vector
	//		int maskedIndex = y * maskedWidth + x;

	//		// Copy the depth value if it's within the mask, otherwise, set it to 0
	//		if (originalX >= 0 && originalX < fs.width &&
	//			originalY >= 0 && originalY < fs.height &&
	//			maskNorm.contains(maskX, maskY))
	//		{
	//			maskedDepth[maskedIndex] = fs.depth[originalY * fs.width + originalX];
	//			maskedIntensity[maskedIndex] = fs.depth[originalY * fs.width + originalX];
	//			maskedState[maskedIndex] = fs.depth[originalY * fs.width + originalX];
	//		}
	//	}
	//}

	//// Update the frameset's depth vector to the new masked depth vector
	//fs.depth = maskedDepth;
	//fs.intensity = maskedIntensity;
	//fs.state = maskedState;
	//fs.width = maskedWidth;
	//fs.height = maskedHeight;







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
}

void CaptureThread::setEnableMask(const bool enable)
{
	QMutexLocker locker(&maskEnabledMutex);
	maskEnabled = enable;
}
