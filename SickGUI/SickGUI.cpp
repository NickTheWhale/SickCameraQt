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
#include "MutexTryLocker.h"


SickGUI::SickGUI(QWidget* parent) : QMainWindow(parent), framesetBuffer(framesetBufferSize)
{
	// create and connect timers to their corresponding update methods
	displayTimer = new QTimer(this);
	chartTimer = new QTimer(this);
	webTimer = new QTimer(this);
	QObject::connect(displayTimer, &QTimer::timeout, this, &SickGUI::updateDisplay);
	QObject::connect(chartTimer, &QTimer::timeout, this, &SickGUI::updateCharts);
	QObject::connect(webTimer, &QTimer::timeout, this, &SickGUI::updateWeb);

	ui.setupUi(this);

	initializeWidgets();
	initializeWebServerConnection();

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

	if (webTimer)
	{
		webTimer->stop();
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


#pragma region INTENSITY_HISTOGRAM
	// the axis range and step is kinda hacky. Best not to mess with it.
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


#pragma region LOGGING_WINDOW

	loggingWidget = new LoggingWidget(this);
	loggingWidget->setMaxLineCount(500);

	dock = new CloseDockWidget("Log", this);
	dock->setObjectName("loggingWidgetDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(loggingWidget);
	dock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

#pragma endregion


#pragma region WEB_SERVER_BUTTON

	QToolButton* webButton = new QToolButton(this);
	webButton->setCheckable(true);
	webButton->setChecked(false);
	webButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	webButton->setText("Web Server");
	webButton->setStatusTip("Toggle Web Server");
	webButton->setIcon(QIcon(":/SickGUI/icons/lan_FILL0_wght400_GRAD0_opsz40.png"));
	QObject::connect(webButton, &QToolButton::toggled, [this, webButton]()
		{
			if (webButton->isChecked())
			{
				if (!webTimer->isActive())
				{
					webTimer->start();
				}
			}
			else
			{
				if (webTimer->isActive())
				{
					webTimer->stop();
				}
			}
		});
	ui.toolBar->addWidget(webButton);

#pragma endregion


#pragma region MISC

	ui.actionPlay->setEnabled(true);
	ui.actionPause->setEnabled(false);

	statusBarLabel = new QLabel(this);
	statusBar()->addPermanentWidget(statusBarLabel);

#pragma endregion
}

bool SickGUI::initializeWebServerConnection()
{
	webSocket = new AutoWebSocket(QUrl("ws://localhost:3000"), QWebSocketProtocol::VersionLatest, this);
	webSocket->start();

	webTimer->setInterval(webTimerInterval);
	return true;
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
				// recalculate the 'grams
				depthHistogram->updateHistogram(fs.depth);
				intensityHistogram->updateHistogram(fs.intensity);
				// force a repaint
				depthHistogram->update();
				intensityHistogram->update();
			}
		, Qt::QueuedConnection);
	}
}

void SickGUI::updateWeb()
{
	// gotta use a mutex since the frameset buffer is accessed by multiple threads
	if (!framesetMutex.tryLock())
	{
		return;
	}
	if (framesetBuffer.empty())
	{
		framesetMutex.unlock();
		return;
	}
	else
	{
		Frameset::frameset_t fs = framesetBuffer.back();
		framesetMutex.unlock();

		QImage qImage;
		Frameset::depthToQImage(fs, qImage);

		QByteArray bytes;
		// packet start
		for (int i = 0; i < 4; ++i)
		{
			bytes.append(static_cast<char>(0));
			bytes.append(static_cast<char>(255));
		}

		// height
		for (int i = 0; i < 4; ++i)
		{
			bytes.append(static_cast<uint8_t>((qImage.height() >> 8 * i) & 0xff));
		}

		// width
		for (int i = 0; i < 4; ++i)
		{
			bytes.append(static_cast<uint8_t>((qImage.width() >> 8 * i) & 0xff));
		}

		// frame number
		for (int i = 0; i < 4; ++i)
		{
			bytes.append(static_cast<uint8_t>((fs.number >> 8 * i) & 0xff));
		}

		// frame time
		for (int i = 0; i < 8; ++i)
		{
			bytes.append(static_cast<uint8_t>((fs.time >> 8 * i) & 0xff));
		}

		// image data (as png)
		QByteArray imageBytes;
		QBuffer buffer(&imageBytes);
		buffer.open(QIODevice::WriteOnly);
		qImage.save(&buffer, "PNG", -1);

		// image size (in bytes)
		for (int i = 0; i < 8; ++i)
		{
			bytes.append(static_cast<uint8_t>((imageBytes.size() >> 8 * i) & 0xff));
		}

		bytes.append(imageBytes);

		qint64 bytesWritten = webSocket->socket()->sendBinaryMessage(bytes);
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

	camera = new(std::nothrow) VisionaryCamera("168.254.43.104");

	return camera != nullptr;
}

void SickGUI::checkThreads()
{
	ThreadResult camThreadResult = threadWatcher->resultAt(0);
	ThreadResult plcThreadResult = threadWatcher->resultAt(1);

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
		// if both threads are good, show the ip's on the status bar
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

void SickGUI::startThreads(QPromise<ThreadResult>& promise)
{
	promise.addResult(startCameraThread());
	promise.addResult(startPlcThread());
}

ThreadResult SickGUI::startCameraThread()
{
	ThreadResult ret;
	try {
		showStatusBarMessage("starting camera thread");
		if (!camera)
		{
			showStatusBarMessage("creating camera");
			if (!createCamera())
			{
				showStatusBarMessage("failed to create camera");
				ret.error = true;
				ret.message = "failed to create camera";
				return ret;
			}
		}
		showStatusBarMessage("opening camera");
		OpenResult openRet = camera->open();
		if (openRet.error != ErrorCode::NONE_ERROR)
		{
			showStatusBarMessage("failed to open camera: " + openRet.message);
			delete camera;
			camera = nullptr;
			ret.error = true;
			ret.message = "failed to open camera: " + openRet.message;
			return ret;
		}
		showStatusBarMessage("starting underlying camera thread handler");
		if (!captureThread)
		{
			showStatusBarMessage("creating underlying camera thread handler");
			captureThread = new(std::nothrow) CaptureThread();
			if (!captureThread)
			{
				showStatusBarMessage("failed to create underlying camera thread handler");
				ret.error = true;
				ret.message = "failed to create underlying camera thread handler";
				return ret;
			}
		}

		QObject::connect(captureThread, SIGNAL(newFrameset(Frameset::frameset_t)), SLOT(newFrameset(Frameset::frameset_t)), Qt::DirectConnection);

		ret.error = !captureThread->startCapture(camera);

		if (!ret.error)
		{
			showStatusBarMessage("camera thread success");
		}
		else
		{
			showStatusBarMessage("camera thread failed");
			ret.message = "camera thread failed";
		}

		return ret;
	}
	catch (std::exception e)
	{
		showStatusBarMessage("failed to start camera thread");
		ret.error = true;
		ret.message = "failed to start camera thread";
		return ret;
	}
}

ThreadResult SickGUI::startPlcThread()
{
	ThreadResult ret;
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
			ret.error = true;
			ret.message = "failed to connect plc client";
			return ret;
		}

		if (!plcThread)
		{
			showStatusBarMessage("creating underlying plc thread handler");
			plcThread = new(std::nothrow) PlcThread();
			if (!plcThread)
			{
				showStatusBarMessage("failed to create underlying plc thread handler");
				qDebug() << "SickGUI::startPlcThread() failed to create plc thread";
				ret.error = true;
				ret.message = "failed to create underlying plc thread handler";
				return ret;
			}
		}

		QObject::connect(captureThread, SIGNAL(newFrameset(Frameset::frameset_t)), plcThread, SLOT(newFrameset(Frameset::frameset_t)), Qt::DirectConnection);
		ret.error = !plcThread->startPlc(s7Client);
		if (!ret.error)
		{
			showStatusBarMessage("plc thread success");
		}
		else
		{
			showStatusBarMessage("plc thread failed");
			qDebug() << "SickGUI::startPlcThread() plc thread failed";
			ret.message = "plc thread failed";
		}
		return ret;
	}
	catch (std::exception e)
	{
		std::string msg = std::format("failed to start plc thread: {}", e.what());
		showStatusBarMessage(msg.c_str());
		qDebug() << "Exception SickGUI::startPlcThread(): " << e.what();
		ret.error = true;
		ret.message = "failed to start plc thread";
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

void SickGUI::newFrameset(Frameset::frameset_t fs)
{
	// keep this method short and sweet for speed.
	//  if you want to do calculations on the frames, do it somewhere else
	MutexTryLocker locker(&framesetMutex);

	if (!locker.isLocked())
	{
		qDebug() << "GUI THREAD NEW FRAMESET NO LOCK";
		return;
	}

	framesetBuffer.push_back(fs);
}

void SickGUI::showStatusBarMessage(const QString& text, int timeout)
{
	qDebug() << text;
	QMetaObject::invokeMethod(this, [this, text, timeout]()
		{
			statusBar()->showMessage(text, timeout);
		}
	, Qt::ConnectionType::DirectConnection);
}