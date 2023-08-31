/*****************************************************************//**
 * @file   VisionaryCamera.cpp
 * @brief  Visionary T Mini camera interface. Sorta handles auto reconnection.
 * 
 * Much of this code is adapted from SampleVisionaryTMiniFrameGrabber.cpp which can be
 * downloaded from the Sick website.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

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

	// open control channel
	if (!pVisionaryControl->open(VisionaryControl::ProtocolType::COLA_2, ipAddress, openTimeout/*ms*/))
	{
		ret.error = ErrorCode::UNKNOWN;
		ret.message = "failed to open control stream";
		return ret;
	}

	// login with service level (I think this is only needed if you want to write camera variables)
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
		// stop streaming
		pVisionaryControl->stopAcquisition();
		// logout sometimes crashes due to nullptr. Doesn't seem necessary to do anyway
		//pVisionaryControl->logout();
		pVisionaryControl->close();
	}
	return true;
}

bool VisionaryCamera::startCapture()
{
	// stop, then start streaming
	pVisionaryControl->stopAcquisition();
	return pVisionaryControl->startAcquisition();
}

bool VisionaryCamera::stopCapture()
{
	// stop streaming
	return pVisionaryControl->stopAcquisition();
}

bool VisionaryCamera::getNextFrameset(frameset::Frameset& fs)
{
	// get next frame
	if (!frameGrabber.getNextFrame(pDataHandler))
		return false;

	// get frame parameters
	const uint32_t height = pDataHandler->getHeight();
	const uint32_t width = pDataHandler->getWidth();
	const uint32_t number = pDataHandler->getFrameNum();
	const uint64_t time = pDataHandler->getTimestampMS();

	// get frame data
	const frameset::Frame depthFrame(pDataHandler->getDistanceMap(), height, width, number, time);
	const frameset::Frame intensityFrame(pDataHandler->getIntensityMap(), height, width, number, time);
	const frameset::Frame stateFrame(pDataHandler->getStateMap(), height, width, number, time);

	// pack frameset struct
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
