/*****************************************************************//**
 * @file   CaptureThread.h
 * @brief  QThread subclass used to retrieve and filter frames from the Camera.
 *
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once

#include <FilterManager.h>
#include <Frameset.h>
#include <VisionaryCamera.h>

#include <qthread.h>
#include <qrect.h>
#include <qmutex.h>
#include <qjsonarray.h>

/**
 * @brief The CaptureThread class handles the capture of frames from a camera.
 *
 * The CaptureThread class is a QThread subclass that captures frames from a camera and emits signals
 * whenever a new FramesetType is available or the connection to the camera is lost.
 */
class CaptureThread : public QThread
{
	Q_OBJECT

signals:
	/**
	 * @brief Emits run() cycle time.
	 */
	void addTime(const int time);

	/**
	 * @brief Emits latest filters that are applied to the thread interface filtered framesets.
	 * 
	 * @param filters
	 */
	void filtersApplied(const QJsonArray& filters);

	/**
	 * @brief Emitted if one or more filters failed to apply.
	 * 
	 */
	void filtersFailed();

	/**
	 * @brief Emitted when the camera fails to return a frameset.
	 * 
	 */
	void disconnected();

	/**
	 * @brief Emitted when the camera succeeds in returning a frameset.
	 * 
	 */
	void reconnected();

public:
	/**
	 * @brief Starts capturing frames from the specified camera.
	 * @param camera Pointer to the Camera object to capture frames from.
	 * @return true if the capture was started successfully, false otherwise.
	 */
	bool startCapture(VisionaryCamera* camera);

	/**
	 * @brief Stops capturing frames.
	 */
	void stopCapture();
	void setFilters(const QJsonArray& filters);
	const QJsonArray getFiltersJson();

protected:
	/**
	 * @brief Overridden method from QThread that runs the capture loop.
	 */
	void run() override;

private:
	VisionaryCamera* camera;
	volatile bool _stop = false;
	
	QMutex filterMutex;
	FilterManager filterManager;
};
