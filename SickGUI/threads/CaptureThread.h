/*****************************************************************//**
 * @file   CaptureThread.h
 * @brief  QThread subclass used to retrieve frames from the Camera
 *
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/

#pragma once

#include <qthread.h>
#include <qrect.h>
#include <qmutex.h>
#include <qjsonarray.h>
#include <FilterManager.h>

#include <Frameset.h>
#include "..\VisionaryCamera.h"

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
	bool startCapture(VisionaryCamera* camera);

	/**
	 * @brief Stops capturing frames.
	 */
	void stopCapture();

	void setMask(const QRectF& maskNorm);
	void setEnableMask(const bool enable);
	void setFilters(const QJsonArray& filters);

protected:
	/**
	 * @brief Overridden method from QThread that runs the capture loop.
	 */
	void run() override;

private:
	VisionaryCamera* camera;
	volatile bool _stop = false;

	QMutex maskMutex;
	QRectF maskNorm;
	
	QMutex maskEnabledMutex;
	volatile bool maskEnabled = false;
	
	QMutex filterMutex;
	FilterManager filterManager;

	//void applyMask(frameset::Frameset& fs, const QRectF& maskNorm);
};
