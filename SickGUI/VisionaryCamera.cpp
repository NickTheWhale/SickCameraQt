#include "VisionaryCamera.h"
#include "VisionaryAutoIPScanCustom.h"

#include "VisionaryFrameset.h"

using namespace visionary;

VisionaryCamera::VisionaryCamera(std::string ipAddress, short dataPort)
{
	this->ipAddress = ipAddress;
	this->dataPort = dataPort;
	// create camera handlers
	pDataHandler = std::make_shared<visionary::VisionaryTMiniData>();
	pDataStream = std::make_shared<VisionaryDataStream>(pDataHandler);
	pVisionaryControl = std::make_shared<VisionaryControl>();
}

VisionaryCamera::~VisionaryCamera()
{
	close();
}

bool VisionaryCamera::open()
{
	// base case
	if (connected) { return true; }

	// check if camera is available
	if (!available(openTimeout))
	{
		if (!available(1000))
		{
			return false;
		}
	}

	// attempt to open camera
	if (!pDataStream->open(ipAddress, htons(dataPort)))
	{
		return false;
	}

	if (!pVisionaryControl->open(VisionaryControl::COLA_2, ipAddress, openTimeout /*ms*/))
	{
		return false;
	}

	if (!pVisionaryControl->login(IAuthentication::UserLevel::AUTHORIZED_CLIENT, "CLIENT"))
	{
		return false;
	}

	connected = true;

	return connected;
}

bool VisionaryCamera::close()
{
	if (pVisionaryControl)
	{
		if (capturing)
			pVisionaryControl->stopAcquisition();
		//pVisionaryControl->logout();
		pVisionaryControl->close();
	}
	if (pDataStream)
		pDataStream->close();
	connected = false;
	return true;
}

bool VisionaryCamera::isOpen()
{
	return connected;
}

bool VisionaryCamera::startCapture()
{
	// base case
	if (!connected) { return false; }
	if (capturing) { return true; }

	capturing = pVisionaryControl->startAcquisition();
	return capturing;
}

bool VisionaryCamera::stopCapture()
{
	if (!capturing) { return true; }
	capturing = false;
	return pVisionaryControl->stopAcquisition();
}

bool VisionaryCamera::getNextFrameset(Frameset &fs)
{
	if (!capturing) { return false; }

	if (!pDataStream->getNextFrame())
	{
		return false;
	}
	auto height = pDataHandler->getHeight();
	auto width = pDataHandler->getWidth();
	auto number = pDataHandler->getFrameNum();
	DepthFrame depth = DepthFrame(pDataHandler->getDistanceMap(), height, width, QImage::Format_Grayscale16, number);
	ColorFrame color = ColorFrame(pDataHandler->getIntensityMap(), height, width, QImage::Format_Grayscale16, number);
	StateFrame state = StateFrame(pDataHandler->getStateMap(), height, width, QImage::Format_Grayscale16, number);


	fs = Frameset(depth, color, state);
	return true;
}

bool VisionaryCamera::available(int timeout)
{
	VisionaryAutoIPScanCustom scanner;
	auto devices = scanner.doScan(timeout);
	for (auto const& device : devices)
	{
		if (device.IpAddress == this->ipAddress)
		{
			return true;
		}
	}
	return false;
}

void VisionaryCamera::setIp(std::string ipAddress)
{
	this->ipAddress = ipAddress;
}

void VisionaryCamera::setDataPort(short dataPort)
{
	this->dataPort = dataPort;
}

