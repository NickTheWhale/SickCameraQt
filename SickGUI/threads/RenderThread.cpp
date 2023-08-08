#include "RenderThread.h"
#include <ThreadInterface.h>
#include <qtimer.h>
#include <Frameset.h>
#include <qdebug.h>

RenderThread::RenderThread(QObject* parent) :
	cycleTimeTarget(100),
	streamType(Stream::Depth),
	streamColorMapType(tinycolormap::ColormapType::Gray),
	invertedColor(false),
	overLayStats(false),
	QThread(parent)
{
	setPriority(QThread::Priority::HighPriority);
}

RenderThread::~RenderThread()
{
	mutex.lock();
	_stop = true;
	condition.wakeOne();
	mutex.unlock();

	wait(10'000);
}

void RenderThread::run()
{
	ThreadInterface& threadInterface = ThreadInterface::instance();
	QElapsedTimer cycleTimer;
	cycleTimer.start();
	while (!_stop)
	{
		if (paused)
		{
			msleep(100);
			continue;
		}

		msleep(1);
		frameset::Frameset fs = threadInterface.peekGuiFrame();

		if (!frameset::isValid(fs))
		{
			qWarning() << __FUNCTION__ << "frameset is invalid";
			continue;
		}

		QImage qImage;
		// determine what stream we want and then overlay some stats if needed
		switch (streamType)
		{
		case Stream::Depth:
		{
			frameset::ImageOptions imageOptions(streamColorMapType, true, 0, 0, invertedColor, false);
			qImage = frameset::toQImage(fs.depth, imageOptions);
		}
		break;
		case Stream::Intensity:
		{
			frameset::ImageOptions imageOptions(streamColorMapType, true, 0, 0, invertedColor, true);
			qImage = frameset::toQImage(fs.intensity, imageOptions);
		}
		break;
		case Stream::State:
		{
			frameset::ImageOptions imageOptions(streamColorMapType, true, 0, 0, invertedColor, false);
			qImage = frameset::toQImage(fs.state, imageOptions);
		}
		break;
		}

		emit renderedImage(qImage);

		const qint64 timeLeft = cycleTimeTarget - cycleTimer.elapsed();
		if (timeLeft > 0)
		{
			msleep(timeLeft);
		}
		qint64 time = cycleTimer.restart();
	}
}

void RenderThread::setPaused(const bool paused)
{
	QMutexLocker locker(&pauseMutex);
	this->paused = paused;
}

void RenderThread::setCycleTimeTarget(const qint64 cycleTime)
{
	QMutexLocker locker(&timeMutex);
	this->cycleTimeTarget = cycleTime;
}

void RenderThread::setStreamType(const Stream& streamType)
{
	QMutexLocker locker(&streamMutex);
	this->streamType = streamType;
}

void RenderThread::setStreamColorMapType(const tinycolormap::ColormapType& streamColorMapType)
{
	QMutexLocker locker(&colorMutex);
	this->streamColorMapType = streamColorMapType;
}

void RenderThread::setInvertedColor(const bool& invertedColor)
{
	QMutexLocker locker(&invertMutex);
	this->invertedColor = invertedColor;
}

void RenderThread::setOverLayStats(const bool& overlayStats)
{
	QMutexLocker locker(&statsMutex);
	this->overLayStats = overlayStats;
}

const qint64 RenderThread::getCycleTimeTarget() const
{
	return cycleTimeTarget;
}

const Stream RenderThread::getStreamType() const
{
	return streamType;
}

const tinycolormap::ColormapType RenderThread::getStreamColorMapType() const
{
	return streamColorMapType;
}

const bool RenderThread::getInvertedColor() const
{
	return invertedColor;
}

const bool RenderThread::getOverLayStats() const
{
	return overLayStats;
}
