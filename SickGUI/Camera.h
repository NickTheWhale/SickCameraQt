/*****************************************************************//**
 * @file   Camera.h
 * @brief  Camera interface
 *
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/
#pragma once

#include <qthread.h>
#include <map>

#include "VisionaryFrameset.h"

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
	//std::string message;
	QString message;
	
	OpenResult() : error(NONE_ERROR), message("") {};

	explicit operator bool() const
	{
		return error == NONE_ERROR;
	}
};

class Camera/* : protected QThread*/
{
public:
	/**
	 * @brief Default constructor for the Camera class.
	 */
	Camera() {}

	/**
	 * @brief Virtual destructor for the Camera class.
	 */
	virtual ~Camera() {}

	/**
	 * @brief Opens the camera device.
	 * @note Implementation must support repeated calls, even if already opened.
	 * @return OpenResult
	 */
	virtual OpenResult open() = 0;

	/**
	 * @brief Closes the camera device.
	 * @note Implementation must support repeated calls, even if already closed.
	 * @return true if the camera device is successfully closed, false otherwise.
	 */
	virtual bool close() = 0;

	/**
	 * @brief Checks if the camera device is open.
	 * @return true if the camera device is open, false otherwise.
	 */
	virtual bool isOpen() = 0;

	/**
	 * @brief Starts capturing frames from the camera device.
	 * @note Implementation must support repeated calls, even if already capturing.
	 * @return true if the capture is started successfully, false otherwise.
	 */
	virtual bool startCapture() = 0;

	/**
	 * @brief Stops capturing frames from the camera device.
	 * * @note Implementation must support repeated calls, even if already stopped.
	 * @return true if the capture is stopped successfully, false otherwise.
	 */
	virtual bool stopCapture() = 0;

	/**
	 * @brief Retrieves the next frameset from the camera device.
	 * @param fs Reference to a frameset object to store the captured frames.
	 * @return true if a frameset is successfully retrieved, false otherwise.
	 */
	virtual bool getNextFrameset(Frameset::frameset_t& fs) = 0;

	/**
	 * @brief Retrieves the parameters of the camera device.
	 *
	 * The parameters are stored in a std::map<std::string, std::string> with no required format.
	 * This is useful if instantiating a subclass as Camera and needing to retrieve model-specific information,
	 * such as an ip address or usb port.
	 * @note the subclass is not required to implement.
	 * @return A map containing the camera parameters, where the keys are parameter names
	 * and the values are parameter values.
	 */
	virtual const std::map<std::string, std::string> getParameters()
	{
		return std::map<std::string, std::string>();
	}
};
