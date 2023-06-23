#pragma once
#include "ICamera.h"

#include "VisionaryFrameset.h"

class RealsenseCamera : public ICamera
{
public:
	RealsenseCamera();
	~RealsenseCamera() override;

	bool open() override;
	bool close() override;
	bool isOpen() override;
	bool startCapture() override;
	bool stopCapture() override;
	bool getNextFrameset(Frameset::frameset_t& fs) override;

private:

};

