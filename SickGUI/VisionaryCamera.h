/*****************************************************************//**
 * @file   VisionaryCamera.h
 * @brief  Visionary T Mini camera interface. Sorta handles auto reconnection.
 *
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once

#include <qobject.h>
#include <VisionaryTMiniData.h>
#include <VisionaryControl.h>
#include <Framegrabber.h>
#include <string>
#include <map>
#include <Frameset.h>

enum ErrorCode
{
	NONE_ERROR = 0,
	CAMERA_NOT_FOUND,
	PERMISSION_DENIED,
	INVALID_IP,
	UNREACHABLE_ADDRESS,
	LOGIN_ERROR,
	UNKNOWN
};

struct OpenResult
{
	ErrorCode error;
	QString message;

	OpenResult() : error(NONE_ERROR), message("") {};

	explicit operator bool() const
	{
		return error == NONE_ERROR;
	}
};

class VisionaryCamera
{
public:
	/**
	 * @brief VisionaryCamera constructor.
	 *
	 * @note Construction does not ensure the camera is accessible. Construction only creates
	 * the necessary handlers to interface with a camera.
	 * @param ipAddress IP address of the camera. ex. "192.168.0.1".
	 * @param dataPort Blob control port (optional).
	*/
	VisionaryCamera(std::string ipAddress, short dataPort = 2114);

	/**
	 * @brief Camera destructor. Calls close().
	*/
	~VisionaryCamera();

	/**
	 * @brief Opens data and control streams to the camera.
	 *
	 * Opens data and control streams to the camera with the specified ip address.
	 * @return OpenResult.
	*/
	OpenResult open();

	/**
	 * @brief Closes data and control streams to the camera.
	 * @return true
	*/
	bool close();

	/**
	 * @brief Starts continuous mode acquisition.
	 * @return true if capture started, false otherwise.
	*/
	bool startCapture();

	/**
	 * @brief Stops acquisition.
	 * @return true if capture stopped, false otherwise.
	*/
	bool stopCapture();

	/**
	 * @brief Queries frameset from camera handler buffer. Note that the depth frame is in units of 1/4mm
	 * @param fs Output FramesetType.
	 * @return true if FramesetType was received, false otherwise.
	*/
	bool getNextFrameset(frameset::Frameset& fs);

	const std::string getIpAddress() const;

	const short getDataPort() const;

private:
	const unsigned int openTimeout = 5000;
	const unsigned int grabberTimeout = 5000;

	const std::string ipAddress;
	const short dataPort;

	visionary::FrameGrabber<visionary::VisionaryTMiniData> frameGrabber;
	std::shared_ptr<visionary::VisionaryTMiniData> pDataHandler;
	std::shared_ptr<visionary::VisionaryControl> pVisionaryControl;
};

