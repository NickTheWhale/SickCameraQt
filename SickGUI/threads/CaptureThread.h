/*****************************************************************//**
 * @file   CaptureThread.h
 * @brief  QThread subclass used to retrieve frames from the Camera
 *
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once

#include <qthread.h>

#include "..\VisionaryFrameset.h"

class Camera;

/**
 * @brief The CaptureThread class handles the capture of frames from a camera.
 *
 * The CaptureThread class is a QThread subclass that captures frames from a camera and emits signals
 * whenever a new frameset is available or the connection to the camera is lost.
 */
class CaptureThread : public QThread
{
	Q_OBJECT

signals:
	/**
	 * @brief Signal emitted when the connection to the camera is lost.
	 */
	void lostConnection();

	void addTime(const int time);

public:
	/**
	 * @brief Starts capturing frames from the specified camera.
	 * @param camera Pointer to the Camera object to capture frames from.
	 * @return true if the capture was started successfully, false otherwise.
	 */
	bool startCapture(Camera* camera);

	/**
	 * @brief Stops capturing frames.
	 */
	void stopCapture();

protected:
	/**
	 * @brief Overridden method from QThread that runs the capture loop.
	 */
	void run() override;

private:
	Camera* camera;
	volatile bool _stop = false;
	volatile bool sendTime = false; 
};
