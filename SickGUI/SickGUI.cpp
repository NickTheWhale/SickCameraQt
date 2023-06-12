#include "SickGUI.h"

#include <qtimer.h>
#include <chrono>

#include "VisionaryCamera.h"
#include "VisionaryAutoIPScanCustom.h"

SickGUI::SickGUI(QWidget* parent) : QMainWindow(parent), frameBuffer(frameBufferSize)
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

	QObject::connect(captureThread, SIGNAL(newImage(Frame::frame_t)), SLOT(newImage(Frame::frame_t)), Qt::DirectConnection);

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
		frameMutex.lock();
		if (frameBuffer.empty())
		{
			frameMutex.unlock();
		}
		else
		{
			Frame::frame_t image = frameBuffer.back();
			frameMutex.unlock();

			QImage qImage;
			if (Frame::toQImage(image, qImage))
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

void SickGUI::newImage(Frame::frame_t image)
{
	if (!frameMutex.tryLock())
		return;

	frameBuffer.push_back(image);
	frameMutex.unlock();
}

void SickGUI::testButtonClick()
{
	VisionaryAutoIPScanCustom scanner;
	auto devices = scanner.doScan(100);

	int i = 0;
}