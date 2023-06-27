#pragma once
#include "Camera.h"

#include <VisionaryTMiniData.h>
#include <VisionaryDataStream.h>
#include <VisionaryControl.h>
#include <string>
#include <map>

class VisionaryCamera : public Camera
{
public:
	VisionaryCamera(std::string ipAddress = "", short dataPort = 2114);
	~VisionaryCamera() override;

	// overridden from Camera.h
	bool open() override;
	bool close()  override;
	bool isOpen() override;
	bool startCapture() override;
	bool stopCapture() override;
	bool getNextFrameset(Frameset::frameset_t &fs) override;
	const std::map<std::string, std::string> getParameters() override;

	bool available(int timeout);

	void setIp(std::string ipAddress);
	void setDataPort(short dataPort);

private:
	const unsigned int openTimeout = 5000;
	bool connected = false;
	bool capturing = false;

	std::string ipAddress;
	short dataPort;

	std::map<std::string, std::string> parameters;

	std::shared_ptr<visionary::VisionaryTMiniData> pDataHandler;
	std::shared_ptr<visionary::VisionaryDataStream> pDataStream;
	std::shared_ptr<visionary::VisionaryControl> pVisionaryControl;

	uint32_t lastFrameNumber;
};

