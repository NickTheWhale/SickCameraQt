#pragma once
#include <qthread.h>
#include "..\Stream.h"
#include "..\TinyColormap.hpp"
#include <qmutex>
#include <qwaitcondition.h>

class RenderThread : public QThread
{
	Q_OBJECT

signals:
	void renderedImage(const QImage& lastImage);
	void fingerprint(const uint32_t fp);

public:
	explicit RenderThread(QObject* parent = nullptr);
	~RenderThread();

	void setPaused(const bool paused);

	void setCycleTimeTarget(const qint64 cycleTime);
	void setStreamType(const Stream& streamType);
	void setStreamColorMapType(const tinycolormap::ColormapType& streamColorMapType);
	void setInvertedColor(const bool& invertedColor);
	void setOverLayStats(const bool& overlayStats);

	const qint64 getCycleTimeTarget() const;
	const Stream getStreamType() const;
	const tinycolormap::ColormapType getStreamColorMapType() const;
	const bool getInvertedColor() const;
	const bool getOverLayStats() const;

protected:
	void run() override;

private:
	QMutex pauseMutex;
	volatile bool paused = true;

	QMutex timeMutex;
	qint64 cycleTimeTarget;

	QMutex streamMutex;
	Stream streamType;

	QMutex colorMutex;
	tinycolormap::ColormapType streamColorMapType;
	
	QMutex invertMutex;
	bool invertedColor;

	QMutex statsMutex;
	bool overLayStats;

	QMutex mutex;
	QWaitCondition condition;
	volatile bool _stop = false;
};

