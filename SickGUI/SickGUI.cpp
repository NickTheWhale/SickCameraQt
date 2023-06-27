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
#include "Fingerprint.h"

#include <qtoolbutton.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include "CloseDockWidget.h"

#include <HistogramWidget.h>


SickGUI::SickGUI(QWidget* parent) : QMainWindow(parent), framesetBuffer(framesetBufferSize)
{
	displayTimer = new QTimer(this);
	chartTimer = new QTimer(this);
	QObject::connect(displayTimer, &QTimer::timeout, this, &SickGUI::updateDisplay);
	QObject::connect(chartTimer, &QTimer::timeout, this, &SickGUI::updateChart);

	ui.setupUi(this);

	initializeWidgets();

	threadWatcher = new QFutureWatcher<bool>(this);
	QObject::connect(threadWatcher, &QFutureWatcher<bool>::finished, this, &SickGUI::checkThreads);

	QFuture<bool> future = QtConcurrent::run(&SickGUI::startThreads, this);

	threadWatcher->setFuture(future);

	restoreSettings();
}

SickGUI::~SickGUI()
{
	if (displayTimer)
	{
		displayTimer->stop();
	}

	if (chartTimer)
	{
		chartTimer->stop();
	}

	if (threadWatcher && !threadWatcher->isFinished())
	{
		threadWatcher->waitForFinished();
	}

	if (captureThread)
	{
		captureThread->stopCapture();
		captureThread->wait(10'000);
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
		plcThread->wait(10'000);
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
}

void SickGUI::closeEvent(QCloseEvent* event)
{
	saveSettings();
	QMainWindow::closeEvent(event);
}

void SickGUI::initializeWidgets()
{

#pragma region CAMERA_VIEW

	cameraView = new AspectRatioPixmapLabel(ui.centralWidget);
	cameraView->setMinimumSize(QSize(300, 300));
	setCentralWidget(cameraView);
	//QDockWidget* dock = new QDockWidget("Camera", this);
	//dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	//dock->setWidget(cameraView);
	//dock->adjustSize();
	//addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, dock);

#pragma endregion

#pragma region STREAM MENU

	QToolButton* streamButton = new QToolButton(this);
	QMenu* streamMenu = new QMenu(streamButton);
	QActionGroup* streamGroup = new QActionGroup(this);

	streamButton->setMenu(streamMenu);
	streamButton->setPopupMode(QToolButton::InstantPopup);
	streamButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	streamButton->setIcon(QIcon(":/SickGUI/icons/bar_chart_4_bars_FILL0_wght400_GRAD0_opsz40.png"));
	streamButton->setStatusTip("Select Stream Type");

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
		action->setStatusTip("Select " + streamActionInfo.name + " Stream");
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
		auto currWidth = fontMetrics.boundingRect(streamActionInfo.name).width();
		if (currWidth > buttonWidth)
		{
			buttonWidth = currWidth;
		}
	}
	QObject::connect(streamMenu, &QMenu::aboutToHide, [this, streamMenu]()
		{
			if (streamMenu->rect().contains(streamMenu->mapFromGlobal(QCursor::pos()))) {
				QTimer::singleShot(0, streamMenu, &QMenu::show);
			}
		});
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
		{"Gray",      tinycolormap::ColormapType::Gray},
		{"Jet",	      tinycolormap::ColormapType::Jet},
		{"Heat",      tinycolormap::ColormapType::Heat},
		{"Hot",       tinycolormap::ColormapType::Hot},
		{"Github",    tinycolormap::ColormapType::Github},
		{"Turbo",     tinycolormap::ColormapType::Turbo},
		{"TurboFast",     tinycolormap::ColormapType::TurboFast}
	};

	QToolButton* colorButton = new QToolButton(this);
	QMenu* colorMenu = new QMenu(colorButton);
	QActionGroup* colorGroup = new QActionGroup(this);

	colorButton->setMenu(colorMenu);
	colorButton->setPopupMode(QToolButton::InstantPopup);
	colorButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	colorButton->setIcon(QIcon(":/SickGUI/icons/palette_FILL0_wght400_GRAD0_opsz40.png"));
	colorButton->setStatusTip("Select Colormap Type");

	fontMetrics = QFontMetrics(colorButton->font());
	buttonWidth = 0;
	firstItem = true;
	for (const auto& colorActionInfo : colorActions)
	{
		QAction* action = colorMenu->addAction(colorActionInfo.name);
		action->setCheckable(true);
		action->setStatusTip("Select " + colorActionInfo.name + " Colormap");
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
		auto currWidth = fontMetrics.boundingRect(colorActionInfo.name).width();
		if (currWidth > buttonWidth)
		{
			buttonWidth = currWidth;
		}
	}
	QObject::connect(colorMenu, &QMenu::aboutToHide, [this, colorMenu]()
		{
			if (colorMenu->rect().contains(colorMenu->mapFromGlobal(QCursor::pos()))) {
				QTimer::singleShot(0, colorMenu, &QMenu::show);
			}
		});

	colorMenu->addSeparator();

	invertedColor = false;
	QAction* action = colorMenu->addAction("Invert");
	action->setCheckable(true);
	action->setStatusTip("Select To Invert Colormap");
	QObject::connect(action, &QAction::changed, [this, action]()
		{
			invertedColor = action->isChecked();
		});

	buttonWidth += colorButton->iconSize().width() + 30;
	colorButton->setMinimumWidth(buttonWidth);
	ui.toolBar->addWidget(colorButton);

#pragma endregion


#pragma region OVERLAY_STATS_BUTTON

	QToolButton* statsButton = new QToolButton(this);
	statsButton->setCheckable(true);
	statsButton->setChecked(overLayStats);
	statsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	statsButton->setText("Stats");
	statsButton->setStatusTip("Toggle Statistics Overlay");
	statsButton->setIcon(QIcon(":/SickGUI/icons/info_FILL0_wght400_GRAD0_opsz40.png"));
	QObject::connect(statsButton, &QToolButton::toggled, [this, statsButton]()
		{
			overLayStats = statsButton->isChecked();
		});
	ui.toolBar->addWidget(statsButton);

#pragma endregion


#pragma region DEPTH_HISTOGRAM

	depthHistogram = new HistogramWidget(100, 5'000, 100, 20'000, ui.centralWidget);
	depthHistogram->setXAxis<double>(0, 0.5, 5, "distance (m)");
	depthHistogram->setYAxis(3'000, 3'000, 23'000, "count");
	depthHistogram->setMinimumSize(QSize(260, 200));
	
	auto dock = new CloseDockWidget("Depth Histogram", this);
	dock->setObjectName("depthHistogramDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(depthHistogram);
	dock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

#pragma endregion


#pragma region INTENSITY_HISTOGRAM

	intensityHistogram = new HistogramWidget(0, 300, 100, 20'000, ui.centralWidget);
	intensityHistogram->setXAxis<double>(0, 20, 100, "intensity (%)");
	intensityHistogram->setYAxis(3'000, 3'000, 23'000, "count");
	intensityHistogram->setMinimumSize(QSize(260, 200));
	
	dock = new CloseDockWidget("Intensity Histogram", this);
	dock->setObjectName("intensityHistogramDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(intensityHistogram);
	dock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

#pragma endregion


#pragma region MISC

	ui.actionPlay->setEnabled(true);
	ui.actionPause->setEnabled(false);

	statusBarLabel = new QLabel(this);
	statusBar()->addPermanentWidget(statusBarLabel);

#pragma endregion
}

void SickGUI::updateDisplay()
{
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
		{
			Frameset::depthToQImage(fs, qImage, streamColorMapType, invertedColor);
			if (overLayStats)
				Fingerprint::overlayStats(qImage, fs.width, fs.height, fs.depth);
		}
		break;
		case Stream::Intensity:
		{
			Frameset::intensityToQImage(fs, qImage, streamColorMapType, invertedColor);
			if (overLayStats)
				Fingerprint::overlayStats(qImage, fs.width, fs.height, fs.intensity);
		}
		break;
		case Stream::State:
		{
			Frameset::stateToQImage(fs, qImage, streamColorMapType, invertedColor);
			if (overLayStats)
				Fingerprint::overlayStats(qImage, fs.width, fs.height, fs.state);
		}
		break;
		}

		writeImage(qImage);
	}
}

void SickGUI::updateChart()
{
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

		QMetaObject::invokeMethod(this, [this, fs]()
			{
				depthHistogram->updateHistogram(fs.depth);
				intensityHistogram->updateHistogram(fs.intensity);
				depthHistogram->update();
				intensityHistogram->update();
			}
		, Qt::QueuedConnection);
	}
}

void SickGUI::writeImage(QImage image)
{
	QMetaObject::invokeMethod(this, [this, image]()
		{
			auto pixmap = QPixmap::fromImage(image);
			cameraView->setPixmap(pixmap);
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

	camera = new(std::nothrow) VisionaryCamera();

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
				auto camParams = camera->getParameters();
				std::string msg;
				if (camParams.count("ipAddress") != 0)
				{
					std::string cameraIpAddress = camParams.at("ipAddress");
					msg = "camera: " + cameraIpAddress + "  |  plc: " + PLC_IP_ADDRESS;
				}
				else
				{
					msg = "plc: " + PLC_IP_ADDRESS;
				}
				statusBarLabel->setText(msg.c_str());
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
				return false;

			}
		}
		showStatusBarMessage("opening camera");
		if (!camera->open())
		{
			showStatusBarMessage("failed to open camera");
			delete camera;
			camera = nullptr;
			return false;
		}
		showStatusBarMessage("starting underlying camera thread handler");
		if (!captureThread)
		{
			showStatusBarMessage("creating underlying camera thread handler");
			captureThread = new(std::nothrow) CaptureThread();
			if (!captureThread)
			{
				showStatusBarMessage("failed to create underlying camera thread handler");
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
		}

		return ret;
	}
	catch (std::exception e)
	{
		showStatusBarMessage("failed to start camera thread");
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
			plcThread = new(std::nothrow) PlcThread();
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

void SickGUI::saveSettings()
{
	QSettings settings("WF", "SickGUI");
	settings.setValue("windowState", this->saveState());
	settings.setValue("geometry", this->saveGeometry());

	settings.sync();
}

void SickGUI::restoreSettings()
{
	QSettings settings("WF", "SickGUI");
	this->restoreState(settings.value("windowState").toByteArray());
	this->restoreGeometry(settings.value("geometry").toByteArray());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                                      SLOTS                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void SickGUI::playVideo()
{
	if (chartTimer && !chartTimer->isActive())
	{
		if (chartTimer->interval() != chartTimerInterval)
		{
			chartTimer->setInterval(chartTimerInterval);
		}
		chartTimer->start();
	}


	if (displayTimer && !displayTimer->isActive())
	{
		if (displayTimer->interval() != displayTimerInterval)
		{
			displayTimer->setInterval(displayTimerInterval);
		}
		displayTimer->start();
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

	if (chartTimer)
	{
		chartTimer->stop();
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
		return;
	}

	framesetBuffer.push_back(fs);
	framesetMutex.unlock();
}