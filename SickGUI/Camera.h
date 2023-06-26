#pragma once
#include <qthread.h>
#include <map>

#include "VisionaryFrameset.h"

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
	virtual bool getNextFrameset(Frameset::frameset_t &fs) = 0;
	virtual const std::map<std::string, std::string> getParameters() = 0;
};

