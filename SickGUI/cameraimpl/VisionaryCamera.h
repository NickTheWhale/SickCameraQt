/*****************************************************************//**
 * @file   VisionaryCamera.h
 * @brief  Implementation of Camera interface for Sick Visionary-T Mini.
 * 
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once
#include "../Camera.h"

#include <VisionaryTMiniData.h>
#include <VisionaryDataStream.h>
#include <VisionaryControl.h>
#include <string>
#include <map>

/**
 * @brief Implementation of Camera interface for Sick Visionary-T Mini.
 */
class VisionaryCamera : public Camera
{
public:
	/**
	 * @brief VisionaryCamera constructor.
	 * 
	 * @note Construction does not ensure the camera is accessible. Construction only creates
	 * the necessary handlers to interface with a camera.
	 * @param ipAddress IP address of the camera (optional). ex. "192.168.0.1".
	 * @param dataPort Blob control port (optional).
	*/
	VisionaryCamera(std::string ipAddress = "", short dataPort = 2114);
	
	/**
	 * @brief Camera destructor. Calls close().
	*/
	~VisionaryCamera() override;

	/**
	 * @brief Opens data and control streams to the camera.
	 * 
	 * Opens data and control streams to the camera with the specified ip address. If the
	 * ip address is empty (""), an attempt is made to scan the network and find the first
	 * available camera.
	 * @return true if opened, false otherwise.
	*/
	bool open() override;

	/**
	 * @brief Closes data and control streams to the camera.
	 * @return true
	*/
	bool close()  override;

	/**
	 * @brief Gets connection status.
	 * @return true is open, false otherwise.
	*/
	bool isOpen() override;

	/**
	 * @brief Starts continuous mode acquisition.
	 * @return true if capture started, false otherwise.
	*/
	bool startCapture() override;

	/**
	 * @brief Stops acquisition.
	 * @return true if capture stopped, false otherwise.
	*/
	bool stopCapture() override;

	/**
	 * @brief Polls camera for the latest frameset.
	 * @param fs Output frameset.
	 * @return true if frameset was received, false otherwise.
	*/
	bool getNextFrameset(Frameset::frameset_t &fs) override;

	/**
	 * @brief Gets camera parameters.
	 * @return camera parameters.
	*/
	const std::map<std::string, std::string> getParameters() override;

	/**
	 * @brief Gets camera availability.
	 * 
	 * This method runs a network scan to get a list of all available cameras and then
	 * checks if the current camera's ip address matches any camera in the list.
	 * @param timeout Scan timeout in milliseconds.
	 * @return true if available, false otherwise.
	*/
	bool available(int timeout);

	/**
	 * @brief IP address setter.
	 * @note Setting the IP address only affects subsequent calls open(). Setting the IP address
	 * will not affect the current connection.
	 * @param ipAddress IP address in "x.x.x.x" format.
	*/
	void setIp(std::string ipAddress);

	/**
	 * @brief Blob control port setter.
	 * @note Setting the port only affects subsequent calls open(). Setting the port
	 * will not affect the current connection.
	 * @param dataPort Data port.
	*/
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
};

