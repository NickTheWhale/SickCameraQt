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

class CaptureThread;

class Camera;

class TS7Client;

class SickGUI : public QMainWindow
{
	Q_OBJECT

public slots:
	void playVideo();
	void pauseVideo();

	void openStreamSettingsDialog();

	void newFrameset(Frameset::frameset_t fs);

	void showStatusBarMessage(const QString& msg, int timeout = 0);

public:
	SickGUI(QWidget* parent = nullptr);
	~SickGUI();

private:
	void resizeEvent(QResizeEvent* event) override;

private:
	void initializeControls();

	void updateDisplay();
	void writeImage(QImage image);
	bool createCamera();

	void checkThreads();

	void startThreads(QPromise<bool>& promise);
	bool startCameraThread();
	bool startPlcThread();

	const std::string CAMERA_IP_ADDRESS = "223.168.0.21";
	const std::string PLC_IP_ADDRESS = "127.0.0.1";
	const int PLC_RACK = 0;
	const int PLC_SLOT = 2;

	Ui::SickGUIClass ui;

	const size_t framesetBufferSize = 10;
	boost::circular_buffer<Frameset::frameset_t> framesetBuffer;
	QMutex framesetMutex;

	Camera* camera = nullptr;
	CaptureThread* captureThread = nullptr;

	TS7Client* s7Client = nullptr;
	PlcThread* plcThread = nullptr;

	QFutureWatcher<bool>* threadWatcher;

	QTimer* displayTimer;
	int displayTimerInterval = 100; /* ms */

	Stream streamType;
	tinycolormap::ColormapType streamColorMapType;
	volatile bool invertedColor;
};