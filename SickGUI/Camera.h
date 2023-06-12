#pragma once
#include <qthread.h>

class Frameset;

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
	virtual bool getNextFrameset(Frameset &fs) = 0;
};

