#pragma once
#include <qthread.h>

#include "Image.h"

class Camera : protected QThread
{
public:
	Camera() {}
	virtual ~Camera() {}

	virtual bool open() = 0;
	virtual bool close() = 0;
	virtual bool isOpen() = 0;
	virtual bool startCapture() = 0;
	virtual bool stopCapture() = 0;
	virtual bool getNextFrameSet(Frame::frameSet_t &fs) = 0;
};

