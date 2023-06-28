/*****************************************************************//**
 * @file   SickGUI.h
 * @brief  Main GUI class
 * 
 * Handles all UI interaction and creates capture and plc threads.
 * 
 * @author Nicholas Loehrke
 * @date   June 2023
 *********************************************************************/
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SickGUI.h"
#include <qstack.h>
#include <qmutex.h>
#include <chrono>
#include <boost/circular_buffer.hpp>
#include "CaptureThread.h"
#include "PlcThread.h"
#include "VisionaryFrameset.h"
#include "Stream.h"
#include <qpromise.h>
#include <qfuturewatcher.h>
#include "TinyColormap.hpp"
#include <HistogramWidget.h>
#include <AspectRatioPixmapLabel.h>

class CaptureThread;

class Camera;

class TS7Client;

class SickGUI : public QMainWindow
{
	Q_OBJECT

public slots:
	/**
	 * @brief Slot to start live video stream and histograms.
	 * @note  This slot has no effect on the plcThread and 
	 *        will not effect the plcThread's ability to receive new frames. 
	 */
	void playVideo();

	/**
	 * @brief Slot to pause live video stream and histograms.
	 * @note  This slot has no effect on the plcThread and 
	 *		  will not pause the plcThread's frame capture.
	 * 
	 */
	void pauseVideo();
	
	/**
	 * @brief Slot to receive new frameset emitted by captureThread.
	 * 
	 * @param fs Output frameset.
	 */
	void newFrameset(Frameset::frameset_t fs);

	/**
	 * @brief Slot to show a message on the status bar.
	 * @note  *Messages shown with this method are not required to be persistent even 
	 *        with a timeout of 0.
	 * @param msg Message to show.
	 * @param timeout How long to display the message in milliseconds. 
	 *        Use timeout = 0 for a persistent* message.
	 */
	void showStatusBarMessage(const QString& msg, int timeout = 0);

public:
	SickGUI(QWidget* parent = nullptr);
	~SickGUI();

private:
	/**
	 * @brief Called when the main window is closing. Used to save dock layout.
	 * 
	 * @param event QCloseEvent.
	 */
	void closeEvent(QCloseEvent* event) override;

	/**
	 * @brief Initializes widgets.
	 * 
	 */
	void initializeWidgets();

	/**
	 * @brief Shows latest camera frame.
	 * 
	 * Grabs the latest frameset from the frameset buffer and converts the selected frame type 
	 * to a QImage. Optionally overlays statistics. Uses SickGUI::writeImage() to display 
	 * the image
	 */
	void updateDisplay();

	/**
	 * @brief Updates depth and intensity histograms.
	 * 
	 * Grabs the latest frameset from the frameset buffer and calls HistogramWidget::updateHistogram
	 * followed by HistogramWidget::update().
	 */
	void updateCharts();

	/**
	 * @brief Writes QImage to display.
	 * 
	 * @param image Image to write.
	 */
	void writeImage(QImage image);

	/**
	 * @brief Creates a new Camera.
	 * 
	 * @return true if created, false otherwise.
	 */
	bool createCamera();

	/**
	 * @brief QFutureWatcher callback to check camera and plc thread start status.
	 * 
	 * @note If startThreads is not successful, a popup will show which thread(s) errored
	 *       and then the application will close.
	 */
	void checkThreads();

	/**
	 * @brief Method to start camera and plc threads.
	 * 
	 * @param promise Output promise
	 */
	void startThreads(QPromise<bool>& promise);
	
	/**
	 * @brief Starts camera thread.
	 * 
	 * Creates and open a Camera, assigns camera to a CaptureThread, and starts the CaptureThread.
	 * Also connects the CaptureThread's newFrameset signal to SickGUI::newFrameset slot.
	 * 
	 * @return true if started, false otherwise.
	 */
	bool startCameraThread();

	/**
	 * @brief Starts plc thread.
	 * 
	 * Creates snap7 client, connects to plc, and starts PlcThread.
	 * Also connects the CaptureThread's newFrameset signal to PlcThread::newFrameset slot.
	 * 
	 * @note This method cannot be called unless startCameraThread() was called and returned true.
	 * 
	 * @return true if started, false otherwise.
	 */
	bool startPlcThread();

	/**
	 * @brief Saves window state and geometry.
	 * 
	 */
	void saveSettings();

	/**
	 * @brief Restores window state and geometry.
	 * 
	 */
	void restoreSettings();

	const std::string PLC_IP_ADDRESS = "127.0.0.1";
	const int PLC_RACK = 0;
	const int PLC_SLOT = 2;

	Ui::SickGUIClass ui;

	const size_t framesetBufferSize = 2;
	boost::circular_buffer<Frameset::frameset_t> framesetBuffer;
	QMutex framesetMutex;

	Camera* camera = nullptr;
	CaptureThread* captureThread = nullptr;

	TS7Client* s7Client = nullptr;
	PlcThread* plcThread = nullptr;

	QFutureWatcher<bool>* threadWatcher;

	AspectRatioPixmapLabel* cameraView = nullptr;
	HistogramWidget* depthHistogram = nullptr;
	HistogramWidget* intensityHistogram = nullptr;

	QTimer* displayTimer = nullptr;
	int displayTimerInterval = 50; /* ms */

	QTimer* chartTimer = nullptr;
	int chartTimerInterval = 50; /* ms */

	Stream streamType;
	tinycolormap::ColormapType streamColorMapType;
	volatile bool invertedColor;

	QLabel* statusBarLabel = nullptr;

	bool overLayStats = false;
};