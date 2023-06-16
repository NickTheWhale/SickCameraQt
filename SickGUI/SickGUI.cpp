#include "SickGUI.h"

#include <qtimer.h>
#include <chrono>

#include "VisionaryCamera.h"
#include "VisionaryAutoIPScanCustom.h"
#include <snap7.h>
#include "ui_StreamSettingDialog.h"
#include <qmessagebox.h>
#include <qfuture.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <format>
#include <qactiongroup.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qsizepolicy.h>

#include <qtoolbutton.h>
#include <qmenu.h>


SickGUI::SickGUI(QWidget* parent) : QMainWindow(parent), framesetBuffer(framesetBufferSize)
{
	displayTimer = new QTimer(this);
	QObject::connect(displayTimer, &QTimer::timeout, this, &SickGUI::updateDisplay);

	ui.setupUi(this);

	initializeControls();

	threadWatcher = new QFutureWatcher<bool>(this);
	QObject::connect(threadWatcher, &QFutureWatcher<bool>::finished, this, &SickGUI::checkThreads);

	QFuture<bool> future = QtConcurrent::run(&SickGUI::startThreads, this);

	threadWatcher->setFuture(future);
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
#pragma region STREAM MENU
	QToolButton* streamButton = new QToolButton(this);
	QMenu* streamMenu = new QMenu(streamButton);
	QActionGroup* streamGroup = new QActionGroup(this);

	streamButton->setMenu(streamMenu);
	streamButton->setPopupMode(QToolButton::InstantPopup);
	streamButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	streamButton->setIcon(QIcon(":/SickGUI/icons/bar_chart_4_bars_FILL0_wght400_GRAD0_opsz40.png"));

	struct StreamActionInfo
	{
		QString name;
		Stream streamType;
	};

	StreamActionInfo streamActions[] =
	{
		{"Depth",     Stream::Depth},
		{"Intensity", Stream::Intensity},
		{"State",     Stream::State}
	};

	QFontMetrics fontMetrics(streamButton->font());
	size_t buttonWidth = 0;
	bool firstItem = true;
	for (const auto& streamActionInfo : streamActions)
	{
		QAction* action = streamMenu->addAction(streamActionInfo.name);
		action->setCheckable(true);
		if (firstItem)
		{
			firstItem = false;
			action->setChecked(true);
			streamType = streamActionInfo.streamType;
			streamButton->setText(streamActionInfo.name);
		}
		streamGroup->addAction(action);
		QObject::connect(action, &QAction::triggered, [this, streamActionInfo, streamButton]()
			{
				streamType = streamActionInfo.streamType;
				streamButton->setText(streamActionInfo.name);
			});
		QObject::connect(streamMenu, &QMenu::aboutToHide, [this, streamActionInfo, streamMenu]()
			{
				if (streamMenu->rect().contains(streamMenu->mapFromGlobal(QCursor::pos()))) {
					QTimer::singleShot(0, streamMenu, &QMenu::show);
				}
			});
		auto currWidth = fontMetrics.boundingRect(streamActionInfo.name).width();
		if (currWidth > buttonWidth)
		{
			buttonWidth = currWidth;
		}
	}
	buttonWidth += streamButton->iconSize().width() + 30;
	streamButton->setMinimumWidth(buttonWidth);
	ui.toolBar->addWidget(streamButton);
#pragma endregion


#pragma region COLORMAP MENU
	struct ColorMapActionInfo
	{
		QString name;
		tinycolormap::ColormapType colormapType;
	};

	ColorMapActionInfo colorActions[] =
	{
		{"Gray",   tinycolormap::ColormapType::Gray},
		{"Jet",	   tinycolormap::ColormapType::Jet},
		{"Heat",   tinycolormap::ColormapType::Heat},
		{"Hot",    tinycolormap::ColormapType::Hot},
		{"Github", tinycolormap::ColormapType::Github}
	};

	QToolButton* colorButton = new QToolButton(this);
	QMenu* colorMenu = new QMenu(colorButton);
	QActionGroup* colorGroup = new QActionGroup(this);

	colorButton->setMenu(colorMenu);
	colorButton->setPopupMode(QToolButton::InstantPopup);
	colorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	colorButton->setIcon(QIcon(":/SickGUI/icons/palette_FILL0_wght400_GRAD0_opsz40.png"));

	fontMetrics = QFontMetrics(colorButton->font());
	buttonWidth = 0;
	firstItem = true;
	for (const auto& colorActionInfo : colorActions)
	{
		QAction* action = colorMenu->addAction(colorActionInfo.name);
		action->setCheckable(true);
		if (firstItem)
		{
			firstItem = false;
			action->setChecked(true);
			streamColorMapType = colorActionInfo.colormapType;
			colorButton->setText(colorActionInfo.name);
		}
		colorGroup->addAction(action);
		QObject::connect(action, &QAction::triggered, [this, colorActionInfo, colorButton]()
			{
				streamColorMapType = colorActionInfo.colormapType;
				colorButton->setText(colorActionInfo.name);
			});
		QObject::connect(colorMenu, &QMenu::aboutToHide, [this, colorActionInfo, colorMenu]()
			{
				if (colorMenu->rect().contains(colorMenu->mapFromGlobal(QCursor::pos()))) {
					QTimer::singleShot(0, colorMenu, &QMenu::show);
				}
			});
		auto currWidth = fontMetrics.boundingRect(colorActionInfo.name).width();
		if (currWidth > buttonWidth)
		{
			buttonWidth = currWidth;
		}
	}

	colorMenu->addSeparator();

	invertedColor = false;
	QAction* action = colorMenu->addAction("Invert");
	action->setCheckable(true);
	QObject::connect(action, &QAction::changed, [this, action]()
		{
			invertedColor = action->isChecked();
		});

	buttonWidth += colorButton->iconSize().width() + 30;
	colorButton->setMinimumWidth(buttonWidth);
	ui.toolBar->addWidget(colorButton);
#pragma endregion

	ui.actionPlay->setEnabled(true);
	ui.actionPause->setEnabled(false);
}

void SickGUI::updateDisplay()
{
	/*QtConcurrent::run([this]() {
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
				Frameset::depthToQImage(fs, qImage, streamColorMapType);
				break;
			case Stream::Intensity:
				Frameset::intensityToQImage(fs, qImage);
				break;
			case Stream::State:
				Frameset::stateToQImage(fs, qImage);
				break;
			}
			writeImage(qImage);
		}
		});*/

	if (this->isMinimized())
		return;

	if (!framesetMutex.tryLock())
		return;
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
			Frameset::depthToQImage(fs, qImage, streamColorMapType, invertedColor);
			break;
		case Stream::Intensity:
			Frameset::intensityToQImage(fs, qImage, streamColorMapType, invertedColor);
			break;
		case Stream::State:
			Frameset::stateToQImage(fs, qImage, streamColorMapType, invertedColor);
			break;
		}
		writeImage(qImage);
	}
}

void SickGUI::resizeEvent(QResizeEvent* event)
{
	QMetaObject::invokeMethod(this, [this]()
		{
			if (ui.cameraView->pixmap().isNull())
				return;
			auto w = ui.cameraView->width();
			auto h = ui.cameraView->height();
			ui.cameraView->setPixmap(ui.cameraView->pixmap().scaled(w, h, Qt::KeepAspectRatio));
			ui.cameraView->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		});
}

void SickGUI::writeImage(QImage image)
{
	QMetaObject::invokeMethod(this, [this, image]()
		{
			auto w = ui.cameraView->width();
			auto h = ui.cameraView->height();
			const auto pixmap = QPixmap::fromImage(image).scaled(w, h, Qt::KeepAspectRatio);
			ui.cameraView->setPixmap(pixmap);
			ui.cameraView->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
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

	camera = new VisionaryCamera();

	return camera != nullptr;
}

void SickGUI::showStatusBarMessage(const QString& text, int timeout)
{
	QMetaObject::invokeMethod(this, [this, text, timeout]()
		{
			statusBar()->showMessage(text, timeout);
		}
	, Qt::ConnectionType::DirectConnection);
}

void SickGUI::checkThreads()
{
	auto camThreadOK = threadWatcher->resultAt(0);
	auto plcThreadOK = threadWatcher->resultAt(1);

	if (!camThreadOK && !plcThreadOK)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to camera and plc");
	}
	else if (!camThreadOK)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to camera");
	}
	else if (!plcThreadOK)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to plc");
	}
	if (!camThreadOK || !plcThreadOK)
	{
		QMessageBox::information(this, "Info", "Application will close now");
		QCoreApplication::quit();
	}
	else
	{
		QTimer::singleShot(1000, [this]()
			{
				std::string msg = std::format("camera: {}  |  plc: {}", CAMERA_IP_ADDRESS, PLC_IP_ADDRESS);
				statusBar()->showMessage(msg.c_str());
			});
	}
}

void SickGUI::startThreads(QPromise<bool>& promise)
{
	promise.addResult(startCameraThread());
	promise.addResult(startPlcThread());
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
				qDebug() << "SickGUI::startCameraThread() failed to create camera";
				return false;
			}
		}
		showStatusBarMessage("opening camera");
		if (!camera->open())
		{
			showStatusBarMessage("failed to open camera");
			qDebug() << "SickGUI::startCameraThread() failed to open camera";
			delete camera;
			camera = nullptr;
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
				qDebug() << "SickGUI::startCameraThread() failed to create underlying camera thread handler";
				return false;
			}
		}

		QObject::connect(captureThread, SIGNAL(newFrameset(Frameset::frameset_t)), SLOT(newFrameset(Frameset::frameset_t)), Qt::DirectConnection);

		auto ret = captureThread->startCapture(camera);

		if (ret)
		{
			showStatusBarMessage("camera thread success");
		}
		else
		{
			showStatusBarMessage("camera thread failed");
			qDebug() << "SickGUI::startCameraThread() camera thread failed";
		}

		return ret;
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
		showStatusBarMessage("starting plc thread");
		if (s7Client)
		{
			if (s7Client->Connected())
				s7Client->Disconnect();
			delete s7Client;
			s7Client = nullptr;
		}

		showStatusBarMessage("creating plc client");
		s7Client = new TS7Client();

		showStatusBarMessage("connecting plc client");
		if (0 != s7Client->ConnectTo(PLC_IP_ADDRESS.c_str(), PLC_RACK, PLC_SLOT))
		{
			showStatusBarMessage("failed to connect plc client");
			qDebug() << "SickGUI::startPlcThread() failed to connect plc client";
			return false;
		}

		if (!plcThread)
		{
			showStatusBarMessage("creating underlying plc thread handler");
			plcThread = new PlcThread();
			if (!plcThread)
			{
				showStatusBarMessage("failed to create underlying plc thread handler");
				qDebug() << "SickGUI::startPlcThread() failed to create plc thread";
				return false;
			}
		}

		QObject::connect(captureThread, SIGNAL(newFrameset(Frameset::frameset_t)), plcThread, SLOT(newFrameset(Frameset::frameset_t)), Qt::DirectConnection);
		auto ret = plcThread->startPlc(s7Client);
		if (ret)
		{
			showStatusBarMessage("plc thread success");
		}
		else
		{
			showStatusBarMessage("plc thread failed");
			qDebug() << "SickGUI::startPlcThread() plc thread failed";
		}
		return ret;
	}
	catch (std::exception e)
	{
		std::string msg = std::format("failed to start plc thread: {}", e.what());
		showStatusBarMessage(msg.c_str());
		qDebug() << "Exception SickGUI::startPlcThread(): " << e.what();
		return false;
	}
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

void SickGUI::openStreamSettingsDialog()
{
	auto* dialog = new QDialog(this);

	Ui::streamSettingDialog ui;
	ui.setupUi(dialog);
	dialog->exec();
	dialog->deleteLater();
}

void SickGUI::newFrameset(Frameset::frameset_t fs)
{
	if (!framesetMutex.tryLock())
	{
		qDebug() << "gui could not acquire lock";
		return;
	}

	framesetBuffer.push_back(fs);
	framesetMutex.unlock();
}