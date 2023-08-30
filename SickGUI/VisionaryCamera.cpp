#include "VisionaryCamera.h"
#include <CoLaParameterWriter.h>
#include <CoLaCommand.h>
#include <CoLaCommandType.h>
#include <Frameset.h>

#include <qdebug.h>
#include <qtimer.h>

#include <vector>

using namespace visionary;

VisionaryCamera::VisionaryCamera(std::string ipAddress, short dataPort) :
	ipAddress(ipAddress),
	dataPort(dataPort),
	frameGrabber(ipAddress, htons(dataPort), grabberTimeout)
{
	// create camera handlers
	pDataHandler = std::make_shared<visionary::VisionaryTMiniData>();
	pVisionaryControl = std::make_shared<VisionaryControl>();
}

VisionaryCamera::~VisionaryCamera()
{
	close();
}

OpenResult VisionaryCamera::open()
{
	OpenResult ret;

	if (!pVisionaryControl->open(VisionaryControl::ProtocolType::COLA_2, ipAddress, openTimeout/*ms*/))
	{
		ret.error = ErrorCode::UNKNOWN;
		ret.message = "failed to open control stream";
		return ret;
	}

	if (!pVisionaryControl->login(IAuthentication::UserLevel::SERVICE, "CUST_SERV"))
	{
		ret.error = ErrorCode::LOGIN_ERROR;
		ret.message = "failed to login with SERVICE level";
		return ret;
	}

	ret.error = ErrorCode::NONE_ERROR;
	ret.message = "successfully opened";
	return ret;
}

bool VisionaryCamera::close()
{
	if (pVisionaryControl)
	{
		pVisionaryControl->stopAcquisition();
		// logout sometimes segfaults. Doesn't seem necessary to do anyway
		//pVisionaryControl->logout();
		pVisionaryControl->close();
	}
	return true;
}

bool VisionaryCamera::startCapture()
{
	pVisionaryControl->stopAcquisition();
	return pVisionaryControl->startAcquisition();
}

bool VisionaryCamera::stopCapture()
{
	return pVisionaryControl->stopAcquisition();
}

bool VisionaryCamera::getNextFrameset(frameset::Frameset& fs)
{
	if (!frameGrabber.getNextFrame(pDataHandler))
		return false;

	const uint32_t height = pDataHandler->getHeight();
	const uint32_t width = pDataHandler->getWidth();
	const uint32_t number = pDataHandler->getFrameNum();
	const uint64_t time = pDataHandler->getTimestampMS();

	const frameset::Frame depthFrame(pDataHandler->getDistanceMap(), height, width, number, time);
	const frameset::Frame intensityFrame(pDataHandler->getIntensityMap(), height, width, number, time);
	const frameset::Frame stateFrame(pDataHandler->getStateMap(), height, width, number, time);

	fs = frameset::Frameset(depthFrame, intensityFrame, stateFrame);
	return true;
}

const std::string VisionaryCamera::getIpAddress() const
{
	return ipAddress;
}

const short VisionaryCamera::getDataPort() const
{
	return dataPort;
}
