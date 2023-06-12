#include "SickGUI.h"

#include <qtimer.h>
#include <chrono>

#include "VisionaryCamera.h"
#include "VisionaryAutoIPScanCustom.h"

SickGUI::SickGUI(QWidget* parent) : QMainWindow(parent), framesetBuffer(framesetBufferSize)
{
	ui.setupUi(this);

	ui.startVideoButton->setEnabled(true);
	ui.stopVideoButton->setEnabled(false);
	plcTimer = QObject::startTimer(plcInterval, Qt::PreciseTimer);
	startVideo();
}

SickGUI::~SickGUI()
{
	if (captureThread)
	{
		stopVideo();
		// kind of hacky. maybe wait the thread?
		while (captureThread->isRunning()) { QThread::msleep(10); };
		delete captureThread;
		captureThread = nullptr;
	}

	if (camera)
	{
		delete camera;
		camera = nullptr;
	}

	QObject::killTimer(plcTimer);
}

void SickGUI::startVideo()
{
	if (!camera)
	{
		if (!createCamera())
			return;
	}

	if (!camera->open())
	{
		delete camera;
		camera = nullptr;
	}

	if (!captureThread)
	{
		captureThread = new CaptureThread();
		if (!captureThread)
			return;
	}

	QObject::connect(captureThread, SIGNAL(newFrameset(Frameset)), SLOT(newFrameset(Frameset)), Qt::DirectConnection);

	if (captureThread->startCapture(camera))
	{
		refreshDisplayTimer = QObject::startTimer(displayInterval, Qt::PreciseTimer);
		ui.startVideoButton->setEnabled(false);
		ui.stopVideoButton->setEnabled(true);
	}
}

void SickGUI::stopVideo()
{
	if (captureThread)
		captureThread->stopCapture();

	QObject::killTimer(refreshDisplayTimer);

	ui.startVideoButton->setEnabled(true);
	ui.stopVideoButton->setEnabled(false);
}

void SickGUI::timerEvent(QTimerEvent* event)
{
	if (event->timerId() == refreshDisplayTimer)
	{
		framesetMutex.lock();
		if (framesetBuffer.empty())
		{
			framesetMutex.unlock();
		}
		else
		{
			Frameset fs = framesetBuffer.back();
			framesetMutex.unlock();

			QImage qImage;
			if (fs.getDepth().toQImage(qImage))
			{
				showImage(qImage);
			}
		}
	}
	if (event->timerId() == plcTimer)
	{
		ui.ipAddressLineEdit->setText(QString::number(++loopCount));
	}
}

void SickGUI::showImage(QImage image)
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
		stopVideo();

	if (camera)
	{
		delete camera;
		camera = nullptr;
	}

	camera = new VisionaryCamera(IP_ADDRESS);

	return camera != nullptr;
}

void SickGUI::newFrameset(Frameset image)
{
	if (!framesetMutex.tryLock())
		return;

	framesetBuffer.push_back(image);
	framesetMutex.unlock();
}

void SickGUI::testButtonClick()
{
	VisionaryAutoIPScanCustom scanner;
	auto devices = scanner.doScan(100);

	int i = 0;
}