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
#include <CaptureThread.h>
#include <PlcThread.h>
#include "VisionaryFrameset.h"
#include "Stream.h"
#include <qpromise.h>
#include <qfuturewatcher.h>
#include "TinyColormap.hpp"
#include <HistogramWidget.h>
#include <LoggingWidget.h>
#include "CustomMessageHandler.h"
#include <CycleTimeWidget.h>
#include <QAutoWebSocket.h>
#include <ThreadInterface.h>
#include <RenderThread.h>
#include "PlotWidget.h"
#include <CameraViewWidget.h>
#include "VisionaryCamera.h"
#include <FrameCompareWidget.h>

class CaptureThread;

class TS7Client;

struct ThreadResult
{
	bool error;
	QString message;

	ThreadResult() : error(false), message("") {};
};

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
	 * @brief Slot to show a message on the status bar.
	 * @note  *Messages shown with this method are not required to be persistent even
	 *        with a timeout of 0.
	 * @param msg Message to show.
	 * @param timeout How long to display the message in milliseconds.
	 *        Use timeout = 0 for a persistent* message.
	 */
	void showStatusBarMessage(const QString& msg, int timeout = 0);

public:
	SickGUI(CustomMessageHandler* messageHandler, QWidget* parent = nullptr);
	~SickGUI();

private slots:
	void updateDisplay(const QImage& image);
	/**
	 * @brief QFutureWatcher callback to check camera and plc thread start status.
	 *
	 * @note If startThreads is not successful, a popup will show which thread(s) errored
	 *       and then the application will close.
	 */
	void checkThreads();

	void pushFingerprint(const uint32_t fp);

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

	void initializeWeb();

	/**
	 * @brief Creates a new Camera.
	 *
	 * @return true if created, false otherwise.
	 */
	bool createCamera();

	void makeConnections();

	/**
	 * @brief Method to start camera and plc threads.
	 *
	 * @param promise Output promise
	 */
	void startThreads(QPromise<ThreadResult>& promise);

	/**
	 * @brief Starts camera thread.
	 *
	 * Creates and open a Camera, assigns camera to a CaptureThread, and starts the CaptureThread.
	 * Also connects the CaptureThread's newFrameset signal to SickGUI::newFrameset slot.
	 *
	 * @return true if started, false otherwise.
	 */
	ThreadResult startCamThread();

	/**
	 * @brief Starts plc thread.
	 *
	 * Creates snap7 client, connects to plc, and starts PlcThread.
	 * Also connects the CaptureThread's newFrameset signal to PlcThread::newFrameset slot.
	 *
	 * @note This method cannot be called unless startCamThread() was called and returned true.
	 *
	 * @return true if started, false otherwise.
	 */
	ThreadResult startPlcThread();

	/**
	 * @brief Saves window state and geometry.
	 *
	 */
	void saveLayout();

	/**
	 * @brief Restores window state and geometry.
	 *
	 */
	void restoreLayout();

	void loadConfiguration();

	const QString CONFIG_PATH = "configuration/configuration.ini";

	std::string plcIpAddress = "";
	qint16 plcRack = 0;
	qint16 plcSlot = 0;

	Ui::SickGUIClass ui;

	ThreadInterface& threadInterface;

	std::string cameraIpAddress = "";
	VisionaryCamera* camera = nullptr;
	CaptureThread* captureThread = nullptr;

	TS7Client* s7Client = nullptr;
	PlcThread* plcThread = nullptr;
	qint64 plcCycleTimeTarget = 0;

	QAutoWebSocket* webSocket = nullptr;

	QFutureWatcher<ThreadResult>* threadWatcher;

	CameraViewWidget* cameraView = nullptr;
	HistogramWidget* depthHistogram = nullptr;
	CycleTimeWidget* cycleTimeWidget = nullptr;
	LoggingWidget* loggingWidget = nullptr;
	PlotWidget* plotWidget = nullptr;
	FrameCompareWidget* frameCompareWidget = nullptr;

	RenderThread renderThread;
	QLabel* statusBarLabel = nullptr;
	CustomMessageHandler* messageHandler = nullptr;
};