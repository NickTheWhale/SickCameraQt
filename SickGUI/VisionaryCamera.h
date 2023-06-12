#pragma once
#include "Camera.h"

#include <VisionaryTMiniData.h>
#include <VisionaryDataStream.h>
#include <VisionaryControl.h>

class VisionaryCamera : public Camera
{
public:
	VisionaryCamera(std::string ipAddress, short dataPort = 2114);
	~VisionaryCamera() override;

	// overridden from Camera.h
	bool open() override;
	bool close()  override;
	bool isOpen() override;
	bool startCapture() override;
	bool stopCapture() override;
	bool getNextImage(Frame::image_t &image) override;

	bool available(int timeout);

	void setIp(std::string ipAddress);
	void setDataPort(short dataPort);

private:
	const unsigned int openTimeout = 100;
	bool connected = false;
	bool capturing = false;

	std::string ipAddress;
	short dataPort;

	std::shared_ptr<visionary::VisionaryTMiniData> pDataHandler;
	std::shared_ptr<visionary::VisionaryDataStream> pDataStream;
	std::shared_ptr<visionary::VisionaryControl> pVisionaryControl;
};

