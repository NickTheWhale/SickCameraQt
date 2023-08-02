/*****************************************************************//**
 * @file   VisionaryCamera.h
 * @brief  Implementation of Camera interface for Sick Visionary-T Mini.
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

/**
 * @brief Implementation of Camera interface for Sick Visionary-T Mini.
 */
class VisionaryCamera : public QObject
{
	Q_OBJECT

public slots:
	void setDepthFilterRange(const uint16_t low, const uint16_t high);
	void setDepthFilterEnable(const bool enable);
	void setDepthMaskEnable(const bool enable);

public:
	/**
	 * @brief VisionaryCamera constructor.
	 *
	 * @note Construction does not ensure the camera is accessible. Construction only creates
	 * the necessary handlers to interface with a camera.
	 * @param ipAddress IP address of the camera (optional). ex. "192.168.0.1".
	 * @param dataPort Blob control port (optional).
	*/
	VisionaryCamera(std::string ipAddress = "", short dataPort = 2114, QObject* parent = nullptr);

	/**
	 * @brief Camera destructor. Calls close().
	*/
	~VisionaryCamera();

	/**
	 * @brief Opens data and control streams to the camera.
	 *
	 * Opens data and control streams to the camera with the specified ip address. If the
	 * ip address is empty (""), an attempt is made to scan the network and find the first
	 * available camera.
	 * @return OpenResult.
	*/
	OpenResult open();

	/**
	 * @brief Closes data and control streams to the camera.
	 * @return true
	*/
	bool close();

	/**
	 * @brief Gets connection status.
	 * @return true is open, false otherwise.
	*/
	bool isOpen();

	bool isAvailable();

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
	 * @brief Polls camera for the latest FramesetType.
	 * @param fs Output FramesetType.
	 * @return true if FramesetType was received, false otherwise.
	*/
	bool getNextFrameset(frameset::Frameset& fs);

	/**
	 * @brief Gets camera parameters.
	 * @return camera parameters.
	*/
	const std::map<std::string, std::string> getParameters();

	/**
	 * @brief Gets camera availability.
	 *
	 * This method runs a network scan to get a list of all available cameras and then
	 * checks if the current camera's ip address matches any camera in the list.
	 * @param timeout Scan timeout in milliseconds.
	 * @return true if available, false otherwise.
	*/
	bool available(int timeout);

private:
	const unsigned int openTimeout = 5000;
	const unsigned int grabberTimeout = 5000;

	const std::string ipAddress;
	const short dataPort;

	std::map<std::string, std::string> parameters;

	visionary::FrameGrabber<visionary::VisionaryTMiniData> frameGrabber;
	std::shared_ptr<visionary::VisionaryTMiniData> pDataHandler;
	std::shared_ptr<visionary::VisionaryControl> pVisionaryControl;

	const bool ping(const std::string ip);
};

