#include "SickGUI.h"

#include <qtimer.h>
#include <chrono>

#include "VisionaryCamera.h"
#include "VisionaryAutoIPScanCustom.h"

#include <snap7.h>

SickGUI::SickGUI(QWidget* parent) : QMainWindow(parent), framesetBuffer(framesetBufferSize)
{
	ui.setupUi(this);

	displayTimer = new QTimer(this);
	QObject::connect(displayTimer, &QTimer::timeout, this, &SickGUI::updateDisplay);

	startCameraThread();
	startPlcThread();

	playVideo();
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

	displayTimer->start();
}

void SickGUI::pauseVideo()
{
	if (displayTimer)
	{
		displayTimer->stop();
	}
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
		if (Frameset::depthToQImage(fs, qImage))
		{
			writeImage(qImage);
		}
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

bool SickGUI::startCameraThread()
{
	if (!camera)
	{
		if (!createCamera())
		{
			qDebug() << "SickGUI::initializeCamera() failed to create camera";
			return false;
		}
	}

	if (!camera->open())
	{
		delete camera;
		camera = nullptr;
		qDebug() << "SickGUI::initializeCamera() failed to open camera";
		return false;
	}

	if (!captureThread)
	{
		captureThread = new CaptureThread();
		if (!captureThread)
		{
			qDebug() << "SickGUI::initializeCamera() failed to create capture thread";
			return false;
		}
	}

	QObject::connect(captureThread, SIGNAL(newFrameset(Frameset::frameset_t)), SLOT(newFrameset(Frameset::frameset_t)), Qt::DirectConnection);

	return captureThread->startCapture(camera);
}

bool SickGUI::startPlcThread()
{
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


void SickGUI::newFrameset(Frameset::frameset_t fs)
{
	if (!framesetMutex.tryLock())
		return;

	framesetBuffer.push_back(fs);
	framesetMutex.unlock();
}

void SickGUI::testButtonClick()
{
}