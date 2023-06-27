#pragma once

#include <qthread.h>
#include "SickGUI.h"

#include "VisionaryFrameset.h"

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
	 * @brief Signal emitted when a new frameset is available.
	 * @param fs The frameset object containing the captured frames.
	 */
	void newFrameset(Frameset::frameset_t fs);

	/**
	 * @brief Signal emitted when the connection to the camera is lost.
	 */
	void lostConnection();

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
	Frameset::frameset_t lastFrameset; /**< The last captured frameset. */
	Camera* camera; /**< Pointer to the Camera object. */
	volatile bool stop; /**< Flag indicating whether to stop capturing. */

	unsigned int frameRetryDelay = 10; /**< Delay in milliseconds between frame capture retries. */
	uint32_t retryCounter = 0; /**< Counter for frame capture retries. */
	unsigned int maxRetryTime = 2000; /**< Maximum time in milliseconds for frame capture retries (must be greater than frameRetryDelay). */
};
