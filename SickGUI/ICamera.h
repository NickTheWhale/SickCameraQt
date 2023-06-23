#pragma once
#include <qthread.h>

#include "VisionaryFrameset.h"

class ICamera : protected QThread
{
public:
	ICamera() {}
	virtual ~ICamera() {}

	virtual bool open() = 0;
	virtual bool close() = 0;
	virtual bool isOpen() = 0;
	virtual bool startCapture() = 0;
	virtual bool stopCapture() = 0;
	virtual bool getNextFrameset(Frameset::frameset_t &fs) = 0;
};

