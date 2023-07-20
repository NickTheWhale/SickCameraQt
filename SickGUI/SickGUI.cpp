#include "SickGUI.h"

#include <qtimer.h>
#include <chrono>

#include "cameraimpl/VisionaryCamera.h"
#include "VisionaryAutoIPScanCustom.h"
#include <snap7.h>
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
#include <qbuffer.h>
#include <HistogramWidget.h>
#include <qimage.h>


SickGUI::SickGUI(CustomMessageHandler* messageHandler, QWidget* parent)
	: QMainWindow(parent), framesetBuffer(framesetBufferSize), messageHandler(messageHandler)
{
	// create and connect timers to their corresponding update methods
	displayTimer = new QTimer(this); displayTimer->setObjectName("displayTimer");
	chartTimer = new QTimer(this); chartTimer->setObjectName("chartTimer");

	QObject::connect(displayTimer, &QTimer::timeout, this, &SickGUI::updateDisplay);
	QObject::connect(chartTimer, &QTimer::timeout, this, &SickGUI::updateCharts);

	ui.setupUi(this);

	initializeWidgets();

	// create and connect future watcher to check thread status
	threadWatcher = new QFutureWatcher<ThreadResult>(this);
	QObject::connect(threadWatcher, &QFutureWatcher<ThreadResult>::finished, this, &SickGUI::checkThreads);

	QFuture<ThreadResult> future = QtConcurrent::run(&SickGUI::startThreads, this);

	threadWatcher->setFuture(future);

	// restore window state and geometry
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
		captureThread->wait(10'000 /*ms*/);
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
		plcThread->wait(10'000 /*ms*/);
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

	if (webThread)
	{
		webThread->stopWeb();
		webThread->wait(10'000 /*ms*/);
		delete webThread;
		webThread = nullptr;
	}

	if (webSocket)
	{
		webSocket->stop();
		delete webSocket;
		webSocket = nullptr;
	}
}

void SickGUI::closeEvent(QCloseEvent* event)
{
	saveSettings();
	QMainWindow::closeEvent(event);
}

void SickGUI::initializeWidgets()
{
	// QLabel used to display live camera image
#pragma region CAMERA_VIEW

	cameraView = new AspectRatioPixmapLabel(ui.centralWidget);
	cameraView->setMinimumSize(QSize(300, 300));
	setCentralWidget(cameraView);

#pragma endregion

	// Button with menu to select stream type (depth, intensity, state)
#pragma region STREAM_MENU

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

	// if you need to add more stream types do it here
	//	note: the first item in the list will be selected by defualt
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
		// check the first item
		if (firstItem)
		{
			firstItem = false;
			action->setChecked(true);
			streamType = streamActionInfo.streamType;
			streamButton->setText(streamActionInfo.name);
		}
		// when a menu option is triggered, update the stream type
		streamGroup->addAction(action);
		QObject::connect(action, &QAction::triggered, [this, streamActionInfo, streamButton]()
			{
				streamType = streamActionInfo.streamType;
				streamButton->setText(streamActionInfo.name);
			});
		// keep track of the widest text to set the minimum button width
		auto currWidth = fontMetrics.boundingRect(streamActionInfo.name).width();
		if (currWidth > buttonWidth)
		{
			buttonWidth = currWidth;
		}
	}
	// make sure the menu doesn't hide when you click an option
	QObject::connect(streamMenu, &QMenu::aboutToHide, [this, streamMenu]()
		{
			if (streamMenu->rect().contains(streamMenu->mapFromGlobal(QCursor::pos())))
			{
				QTimer::singleShot(0, streamMenu, &QMenu::show);
			}
		});
	buttonWidth += streamButton->iconSize().width() + 30;
	streamButton->setMinimumWidth(buttonWidth);
	ui.toolBar->addWidget(streamButton);

#pragma endregion

	// Button with menu to select colormap type (gray, jet, turbo, etc.)
#pragma region COLORMAP_MENU

	struct ColorMapActionInfo
	{
		QString name;
		tinycolormap::ColormapType colormapType;
	};

	// if you need to add more colormap types do it here
	//	note: the first item will be selected by default
	ColorMapActionInfo colorActions[] =
	{
		{"Gray",      tinycolormap::ColormapType::Gray},
		{"Jet",	      tinycolormap::ColormapType::Jet},
		{"Heat",      tinycolormap::ColormapType::Heat},
		{"Hot",       tinycolormap::ColormapType::Hot},
		{"Github",    tinycolormap::ColormapType::Github},
		{"Turbo",     tinycolormap::ColormapType::Turbo},
		{"TurboFast", tinycolormap::ColormapType::TurboFast}
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
		// check the first item
		if (firstItem)
		{
			firstItem = false;
			action->setChecked(true);
			streamColorMapType = colorActionInfo.colormapType;
			colorButton->setText(colorActionInfo.name);
		}
		// when a menu option is triggered, update the colormap type
		colorGroup->addAction(action);
		QObject::connect(action, &QAction::triggered, [this, colorActionInfo, colorButton]()
			{
				streamColorMapType = colorActionInfo.colormapType;
				colorButton->setText(colorActionInfo.name);
			});
		// keep track of the widest text to set the minumum button width
		auto currWidth = fontMetrics.boundingRect(colorActionInfo.name).width();
		if (currWidth > buttonWidth)
		{
			buttonWidth = currWidth;
		}
	}
	// make sure the menu doesn't hide when you click an option
	QObject::connect(colorMenu, &QMenu::aboutToHide, [this, colorMenu]()
		{
			if (colorMenu->rect().contains(colorMenu->mapFromGlobal(QCursor::pos()))) {
				QTimer::singleShot(0, colorMenu, &QMenu::show);
			}
		});

	colorMenu->addSeparator();
	// add an option to invert the color map
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
	// the axis range and step is kinda hacky. Best not to mess with it.
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


#pragma region LOGGING_WINDOW

	loggingWidget = new LoggingWidget(this);
	loggingWidget->setMaxLineCount(250);

	connect(messageHandler, &CustomMessageHandler::newMessage, loggingWidget, &LoggingWidget::showMessage);

	dock = new CloseDockWidget("Log", this);
	dock->setObjectName("loggingWidgetDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(loggingWidget);
	dock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

#pragma endregion


#pragma region CYCLE_TIME

	cycleTimeWidget = new CycleTimeWidget(this);

	dock = new CloseDockWidget("Cycle Time", this);
	dock->setObjectName("cycleTimeWidgetDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(cycleTimeWidget);
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
	// don't bother is the window is minimized
	if (this->isMinimized())
		return;

	// gotta use a mutex since the frameset buffer is accessed by multiple threads
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

		// determine what stream we want and then overlay some stats if needed
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

		// we use this method for thread safety
		writeImage(qImage);
	}
}

void SickGUI::updateCharts()
{
	// don't bother if the window is minimized
	if (this->isMinimized())
		return;

	// gotta use a mutex since the frameset buffer is accessed by multiple threads
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

		// we use invokeMethod for thread safety
		QMetaObject::invokeMethod(this, [this, fs]()
			{
				// recalculate the 'gram
				depthHistogram->updateHistogram(fs.depth);
				// force a repaint
				depthHistogram->update();
			}
		, Qt::QueuedConnection);
	}
}

void SickGUI::writeImage(QImage image)
{
	// we use invokeMethod for thread safety
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

	camera = new(std::nothrow) VisionaryCamera("192.168.1.213");

	return camera != nullptr;
}

void SickGUI::checkThreads()
{
	ThreadResult camThreadResult = threadWatcher->resultAt(0);
	ThreadResult plcThreadResult = threadWatcher->resultAt(1);
	//ThreadResult webThreadResult = threadWatcher->resultAt(2);

	if (camThreadResult.error && plcThreadResult.error)
	{
		//std::string msg = std::format("Failed to connect to camera and plc.\nCamera error: {}\nPlc error: {}", camThreadResult.message, plcThreadResult.message);
		std::string msg = "idk";
		QMessageBox::critical(this, "Error", msg.c_str());
	}
	else if (camThreadResult.error)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to camera");
	}
	else if (plcThreadResult.error)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to plc");
	}
	if (camThreadResult.error || plcThreadResult.error)
	{
		QMessageBox::information(this, "Info", "Application will close now");
		QCoreApplication::quit();
	}
	else
	{
		// if both threads are good, show the ip's on the status bar and reset the cycle times
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

				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetPlcTimes(); });
				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetCamTimes(); });
			});
	}
}

void SickGUI::startThreads(QPromise<ThreadResult>& promise)
{
	promise.addResult(startCamThread());
	promise.addResult(startPlcThread());
	promise.addResult(startWebThread());
}

ThreadResult SickGUI::startCamThread()
{
	ThreadResult ret;
	try {
		qInfo() << "starting camera thread";

		if (!camera)
		{
			qInfo() << "creating camera";
			if (!createCamera())
			{
				qCritical() << "failed to create camera";
				ret.error = true;
				ret.message = "failed to create camera";
				return ret;
			}
		}
		qInfo() << "opening camera";
		OpenResult openRet = camera->open();
		if (openRet.error != ErrorCode::NONE_ERROR)
		{
			delete camera;
			camera = nullptr;
			qCritical() << "failed to open camera: " << openRet.message;
			ret.error = true;
			ret.message = "failed to open camera: " + openRet.message;
			return ret;
		}
		qInfo() << "starting underlying camera thread handler";
		if (!captureThread)
		{
			qInfo() << "creating underlying camera thread handler";
			captureThread = new(std::nothrow) CaptureThread();
			if (!captureThread)
			{
				qCritical() << "failed to create underlying camera thread handler";
				ret.error = true;
				ret.message = "failed to create underlying camera thread handler";
				return ret;
			}
		}

		ret.error = !captureThread->startCapture(camera);
		if (!ret.error)
		{
			qInfo() << "camera thread success";
			QObject::connect(captureThread, &CaptureThread::newFrameset, this, &SickGUI::newFrameset, Qt::DirectConnection);
			QObject::connect(captureThread, &CaptureThread::addTime, cycleTimeWidget, &CycleTimeWidget::addCamTime);
		}
		else
		{
			qCritical() << "camera thread failed";
			ret.message = "camera thread failed";
		}

		return ret;
	}
	catch (std::exception e)
	{
		qCritical() << "exception while starting camera thread:	" << e.what();
		ret.error = true;
		ret.message = "failed to start camera thread";
		return ret;
	}
}

ThreadResult SickGUI::startPlcThread()
{
	ThreadResult ret;
	try {
		qInfo() << "starting plc thread";
		if (s7Client)
		{
			if (s7Client->Connected())
				s7Client->Disconnect();
			delete s7Client;
			s7Client = nullptr;
		}

		qInfo() << "creating plc client";
		s7Client = new TS7Client();

		qInfo() << "connecting plc client";
		int connectRet = s7Client->ConnectTo(PLC_IP_ADDRESS.c_str(), PLC_RACK, PLC_SLOT);
		if (0 != connectRet)
		{
			qCritical() << "failed to connect plc client. code:	" << static_cast<int>(connectRet);
			ret.error = true;
			ret.message = "failed to connect plc client";
			return ret;
		}

		if (!plcThread)
		{
			qInfo() << "creating underlying plc thread handler";
			plcThread = new(std::nothrow) PlcThread();
			if (!plcThread)
			{
				qCritical() << "failed to create underlying plc thread handler";
				ret.error = true;
				ret.message = "failed to create underlying plc thread handler";
				return ret;
			}
		}

		ret.error = !plcThread->startPlc(s7Client);
		if (!ret.error)
		{
			qInfo() << "plc thread success";
			QObject::connect(captureThread, &CaptureThread::newFrameset, plcThread, &PlcThread::newFrameset, Qt::DirectConnection);
			QObject::connect(plcThread, &PlcThread::addTime, cycleTimeWidget, &CycleTimeWidget::addPlcTime);
		}
		else
		{
			qCritical() << "plc thread failed";
			ret.message = "plc thread failed";
		}
		return ret;
	}
	catch (std::exception e)
	{
		qCritical() << "exception while starting plc thread: " << e.what();
		ret.error = true;
		ret.message = "failed to start plc thread";
		return ret;
	}
}

ThreadResult SickGUI::startWebThread()
{
	ThreadResult ret;
	try {
		qInfo() << "starting web thread";
		if (webSocket)
		{
			webSocket->stop();
			delete webSocket;
			webSocket = nullptr;
		}

		qInfo() << "creating websocket";
		webSocket = new AutoWebSocket(QUrl("ws://127.0.0.1:3000"), QWebSocketProtocol::VersionLatest);

		qInfo() << "starting websocket";
		webSocket->start();

		if (!webThread)
		{
			qInfo() << "creating underlying web thread handler";
			webThread = new(std::nothrow) WebThread();
			if (!webThread)
			{
				qCritical() << "failed to create underlying web thread handler";
				ret.error = true;
				ret.message = "failed to create underlying web thread handler";
				return ret;
			}
		}

		webSocket->moveToThread(webThread->currentThread());
		webSocket->setParent(webThread);

		ret.error = !webThread->startWeb(webSocket);
		if (!ret.error)
		{
			qInfo() << "web thread success";
			QObject::connect(captureThread, &CaptureThread::newFrameset, webThread, &WebThread::newFrameset, Qt::DirectConnection);
			QObject::connect(webThread, &WebThread::addTime, cycleTimeWidget, &CycleTimeWidget::addWebTime);
		}
		else
		{
			qCritical() << "web thread failed";
			ret.message = "web thread failed";
		}
		return ret;
	}
	catch (std::exception e)
	{
		qCritical() << "exception while starting web thread: " << e.what();
		ret.error = true;
		ret.message = "failed to start web thread";
		return ret;
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

void SickGUI::newFrameset(const Frameset::frameset_t& fs)
{
	framesetBuffer.push_back(fs);
}

void SickGUI::showStatusBarMessage(const QString& text, int timeout)
{
	QMetaObject::invokeMethod(this, [this, text, timeout]()
		{
			statusBar()->showMessage(text, timeout);
		}
	, Qt::ConnectionType::DirectConnection);
}