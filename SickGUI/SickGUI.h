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

class CaptureThread;

class Camera;

class TS7Client;

class SickGUI : public QMainWindow
{
	Q_OBJECT


public slots:
	void playVideo();
	void pauseVideo();

	void selectDepth();
	void selectIntensity();
	void selectState();

	void openStreamSettingsDialog();

	void newFrameset(Frameset::frameset_t fs);

public:
	SickGUI(QWidget* parent = nullptr);
	~SickGUI();

private:
	void initializeControls();

	void updateDisplay();
	void writeImage(QImage image);
	bool createCamera();

	bool startCameraThread();
	bool startPlcThread();

	const std::string CAMERA_IP_ADDRESS = "169.254.49.161";
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

	QTimer *displayTimer;
	int displayTimerInterval = 33; /* ms */

	Stream streamType;
};