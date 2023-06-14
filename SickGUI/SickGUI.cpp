#include "SickGUI.h"

#include <qtimer.h>
#include <chrono>

#include "VisionaryCamera.h"
#include "VisionaryAutoIPScanCustom.h"
#include <snap7.h>
#include "ui_StreamSettingDialog.h"
#include <qmessagebox.h>
#include <future>


SickGUI::SickGUI(QWidget* parent) : QMainWindow(parent), framesetBuffer(framesetBufferSize)
{
	displayTimer = new QTimer(this);
	QObject::connect(displayTimer, &QTimer::timeout, this, &SickGUI::updateDisplay);

	ui.setupUi(this);

	initializeControls();

	QTimer::singleShot(1000, this, &SickGUI::startCameraThread);
	//QTimer::singleShot(1000, this, &SickGUI::startPlcThread);

	//auto camThreadOK = startCameraThread();
	//auto plcThreadOK = startPlcThread();


	//if (!camThreadOK && !plcThreadOK)
	//{
	//	QMessageBox::critical(this, "Error", "Failed to start camera and plc thread");
	//}
	//else if (!camThreadOK)
	//{
	//	QMessageBox::critical(this, "Error", "Failed to start camera thread");
	//}
	//else if (!plcThreadOK)
	//{
	//	QMessageBox::critical(this, "Error", "Failed to start plc thread");
	//}
	//if (!camThreadOK || !plcThreadOK)
	//{
	//	_failed = true;
	//	QMessageBox::information(this, "Info", "Application will close now");
	//}
}

SickGUI::~SickGUI()
{
	if (captureThread)
	{
		captureThread->stopCapture();
		captureThread->wait(2000);
		delete captureThread;
		captureThread = nullptr;
	}

	if (camera)
	{
		delete camera;
		camera = nullptr;
	}

	if (plcThread)
	{
		plcThread->stopPlc();
		plcThread->wait(2000);
		delete plcThread;
		plcThread = nullptr;
	}

	if (s7Client)
	{
		if (s7Client->Connected())
			s7Client->Disconnect();
		delete s7Client;
		s7Client = nullptr;
	}

	if (displayTimer)
	{
		displayTimer->stop();
	}
}


void SickGUI::initializeControls()
{
	ui.actionPlay->setEnabled(true);
	ui.actionPause->setEnabled(false);
	ui.actionDepth->setChecked(true);
	ui.actionIntensity->setChecked(false);
	ui.actionState->setChecked(false);

	streamType = Stream::Depth;
}

void SickGUI::updateDisplay()
{
	framesetMutex.lock();
	if (framesetBuffer.empty())
	{
		framesetMutex.unlock();
	}
	else
	{
		Frameset::frameset_t fs = framesetBuffer.back();
		framesetMutex.unlock();

		QImage qImage;

		switch (streamType)
		{
		case Stream::Depth:
			if (!Frameset::depthToQImage(fs, qImage))
				return;
			break;
		case Stream::Intensity:
			if (!Frameset::intensityToQImage(fs, qImage))
				return;
			break;
		case Stream::State:
			if (!Frameset::stateToQImage(fs, qImage))
				return;
			break;
		}
		writeImage(qImage);
	}
}

void SickGUI::writeImage(QImage image)
{
	int newWidth = this->width();
	int newHeight = this->height();

	QPixmap pixmap = QPixmap::fromImage(image);
	pixmap = pixmap.scaled(newWidth, newHeight);


	QMetaObject::invokeMethod(this, [this, pixmap]()
		{
			ui.cameraView->setPixmap(pixmap);
		}
	, Qt::QueuedConnection);
}

bool SickGUI::createCamera()
{
	if (captureThread && captureThread->isRunning())
		pauseVideo();

	if (camera)
	{
		delete camera;
		camera = nullptr;
	}

	camera = new VisionaryCamera(CAMERA_IP_ADDRESS);

	return camera != nullptr;
}

void SickGUI::showStatusBarMessage(const QString& text, int timeout)
{
	QMetaObject::invokeMethod(this, [this, text, timeout]()
		{
			ui.statusBar->showMessage(text, timeout);
			this->update();
			qDebug() << text;
		}
	, Qt::ConnectionType::DirectConnection);
}

bool SickGUI::startCameraThread()
{
	try {
		showStatusBarMessage("starting camera thread");
		if (!camera)
		{
			showStatusBarMessage("creating camera");
			if (!createCamera())
			{
				showStatusBarMessage("failed to create camera");
				qDebug() << "SickGUI::initializeCamera() failed to create camera";
				return false;
			}
		}
		showStatusBarMessage("opening camera");
		if (!camera->open())
		{
			showStatusBarMessage("failed to open camera");
			delete camera;
			camera = nullptr;
			qDebug() << "SickGUI::initializeCamera() failed to open camera";
			return false;
		}
		showStatusBarMessage("starting underlying camera thread handler");
		if (!captureThread)
		{
			showStatusBarMessage("creating underlying camera thread handler");
			captureThread = new CaptureThread();
			if (!captureThread)
			{
				showStatusBarMessage("failed to create underlying camera thread handler");
				qDebug() << "SickGUI::initializeCamera() failed to create capture thread";
				return false;
			}
		}

		QObject::connect(captureThread, SIGNAL(newFrameset(Frameset::frameset_t)), SLOT(newFrameset(Frameset::frameset_t)), Qt::DirectConnection);

		return captureThread->startCapture(camera);
	}
	catch (std::exception e)
	{
		showStatusBarMessage("failed to start camera thread");
		qDebug() << "Exception SickGUI::startCameraThread(): " << e.what();
		return false;
	}
}

bool SickGUI::startPlcThread()
{
	try {
		if (s7Client)
		{
			if (s7Client->Connected())
				s7Client->Disconnect();
			delete s7Client;
			s7Client = nullptr;
		}

		s7Client = new TS7Client();

		if (0 != s7Client->ConnectTo(PLC_IP_ADDRESS.c_str(), PLC_RACK, PLC_SLOT))
		{
			return false;
		}

		if (!plcThread)
		{
			plcThread = new PlcThread();
			if (!plcThread)
			{
				qDebug() << "SickGUI::startPlcThread() failed to create plc thread";
				return false;
			}
		}

		QObject::connect(captureThread, SIGNAL(newFrameset(Frameset::frameset_t)), plcThread, SLOT(newFrameset(Frameset::frameset_t)), Qt::DirectConnection);
		return plcThread->startPlc(s7Client);
	}
	catch (std::exception e)
	{
		qDebug() << "Exception SickGUI::startPlcThread(): " << e.what();
		return false;
	}
}

bool SickGUI::failed()
{
	return _failed;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                                      SLOTS                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void SickGUI::playVideo()
{
	if (!displayTimer)
		return;

	if (displayTimer->isActive())
		return;

	if (displayTimer->interval() != displayTimerInterval)
	{
		displayTimer->setInterval(displayTimerInterval);
	}

	ui.actionPlay->setEnabled(false);
	ui.actionPause->setEnabled(true);

	displayTimer->start();
}

void SickGUI::pauseVideo()
{
	if (displayTimer)
	{
		displayTimer->stop();
	}

	ui.actionPlay->setEnabled(true);
	ui.actionPause->setEnabled(false);
}

void SickGUI::selectDepth()
{
	ui.actionDepth->setChecked(true);

	ui.actionIntensity->setEnabled(true);
	ui.actionIntensity->setChecked(false);

	ui.actionState->setEnabled(true);
	ui.actionState->setChecked(false);

	streamType = Stream::Depth;
}

void SickGUI::selectIntensity()
{
	ui.actionDepth->setEnabled(true);
	ui.actionDepth->setChecked(false);

	ui.actionIntensity->setChecked(true);

	ui.actionState->setEnabled(true);
	ui.actionState->setChecked(false);

	streamType = Stream::Intensity;
}

void SickGUI::selectState()
{
	ui.actionDepth->setEnabled(true);
	ui.actionDepth->setChecked(false);

	ui.actionIntensity->setEnabled(true);
	ui.actionIntensity->setChecked(false);

	ui.actionState->setChecked(true);

	streamType = Stream::State;
}

void SickGUI::openStreamSettingsDialog()
{
	QDialog* dialog = new QDialog(this);

	Ui::streamSettingDialog ui;
	ui.setupUi(dialog);
	dialog->exec();
	dialog->deleteLater();
}

void SickGUI::newFrameset(Frameset::frameset_t fs)
{
	if (!framesetMutex.tryLock())
		return;

	framesetBuffer.push_back(fs);
	framesetMutex.unlock();
}