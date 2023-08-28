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

#include <qpromise.h>
#include <qmainwindow.h>
#include <qfuturewatcher.h>
#include <qlabel.h>

#include "VisionaryCamera.h"
#include <CaptureThread.h>
#include <PlcThread.h>

#include <CycleTimeWidget.h>
#include <LoggingWidget.h>
#include <FilterEditorWidget.h>

#include <Frameset.h>
#include "Stream.h"
#include "TinyColormap.hpp"
#include <ThreadInterface.h>
#include "CustomMessageHandler.h"

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

public:
	SickGUI(CustomMessageHandler* messageHandler, QWidget* parent = nullptr);
	~SickGUI();

private slots:
	/**
	 * @brief QFutureWatcher callback to check camera and plc thread start status.
	 *
	 * @note If startThreads is not successful, a popup will show which thread(s) errored
	 *       and then the application will close.
	 */
	void checkThreads();

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

	void updateStatusBar();
	bool cameraConnected = false;
	bool plcConnected = false;
	bool lastCameraConnected = false;
	bool lastPlcConnected = false;
	QLabel* statusLabel = nullptr;

	std::string plcIpAddress = "";
	qint16 plcRack = 0;
	qint16 plcSlot = 0;

	ThreadInterface& threadInterface;

	std::string cameraIpAddress = "";
	VisionaryCamera* camera = nullptr;
	CaptureThread* captureThread = nullptr;

	TS7Client* s7Client = nullptr;
	PlcThread* plcThread = nullptr;
	qint64 plcCycleTimeTarget = 0;

	QFutureWatcher<ThreadResult>* threadWatcher;

	FilterEditorWidget* filterEditorWidget = nullptr;
	CycleTimeWidget* cycleTimeWidget = nullptr;
	LoggingWidget* loggingWidget = nullptr;

	CustomMessageHandler* messageHandler = nullptr;
};