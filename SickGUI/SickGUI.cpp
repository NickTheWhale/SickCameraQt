#include "SickGUI.h"

#include <qtimer.h>

#include <snap7.h>
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
#include <qpushbutton.h>
#include <qsettings.h>
#include "CloseDockWidget.h"
#include <qbuffer.h>
#include <HistogramWidget.h>
#include <qimage.h>
#include "VisionaryAutoIPScanCustom.h"


SickGUI::SickGUI(CustomMessageHandler* messageHandler, QWidget* parent) :
	renderThread(RenderThread()),
	messageHandler(messageHandler),
	threadInterface(ThreadInterface::instance()),
	QMainWindow(parent)
{
	loadConfiguration();

	ui.setupUi(this);

	initializeWidgets();

	// create and connect future watcher to check thread status
	threadWatcher = new QFutureWatcher<ThreadResult>(this);
	QObject::connect(threadWatcher, &QFutureWatcher<ThreadResult>::finished, this, &SickGUI::checkThreads);

	QFuture<ThreadResult> future = QtConcurrent::run(&SickGUI::startThreads, this);

	threadWatcher->setFuture(future);

	// restore window state and geometry
	restoreLayout();
}

SickGUI::~SickGUI()
{
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
	saveLayout();
	QMainWindow::closeEvent(event);
}

void SickGUI::initializeWidgets()
{
	// QLabel used to display live camera lastImage
#pragma region CAMERA_VIEW

	cameraView = new CameraViewWidget(ui.centralWidget);
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
			//streamType = streamActionInfo.streamType;
			renderThread.setStreamType(streamActionInfo.streamType);
			streamButton->setText(streamActionInfo.name);
		}
		// when a menu option is triggered, update the stream type
		streamGroup->addAction(action);
		QObject::connect(action, &QAction::triggered, [this, streamActionInfo, streamButton]()
			{
				//streamType = streamActionInfo.streamType;
				renderThread.setStreamType(streamActionInfo.streamType);

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
			//streamColorMapType = colorActionInfo.colormapType;
			renderThread.setStreamColorMapType(colorActionInfo.colormapType);
			colorButton->setText(colorActionInfo.name);
		}
		// when a menu option is triggered, update the colormap type
		colorGroup->addAction(action);
		QObject::connect(action, &QAction::triggered, [this, colorActionInfo, colorButton]()
			{
				//streamColorMapType = colorActionInfo.colormapType;
				renderThread.setStreamColorMapType(colorActionInfo.colormapType);
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
	//invertedColor = false;
	renderThread.setInvertedColor(false);
	QAction* action = colorMenu->addAction("Invert");
	action->setCheckable(true);
	action->setStatusTip("Select To Invert Colormap");
	QObject::connect(action, &QAction::changed, [this, action]()
		{
			//invertedColor = action->isChecked();
			renderThread.setInvertedColor(action->isChecked());
		});

	buttonWidth += colorButton->iconSize().width() + 30;
	colorButton->setMinimumWidth(buttonWidth);
	ui.toolBar->addWidget(colorButton);

#pragma endregion


#pragma region OVERLAY_STATS_BUTTON

	QToolButton* statsButton = new QToolButton(this);
	statsButton->setCheckable(true);
	renderThread.setOverLayStats(false);
	statsButton->setChecked(renderThread.getOverLayStats());
	statsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	statsButton->setText("Stats");
	statsButton->setStatusTip("Toggle Statistics Overlay");
	statsButton->setIcon(QIcon(":/SickGUI/icons/info_FILL0_wght400_GRAD0_opsz40.png"));
	QObject::connect(statsButton, &QToolButton::toggled, [this, statsButton]()
		{
			//overLayStats = statsButton->isChecked();
			renderThread.setOverLayStats(statsButton->isChecked());
		});
	ui.toolBar->addWidget(statsButton);

#pragma endregion


#pragma region DEPTH_HISTOGRAM
	// the axis range and step is kinda hacky. Best not to mess with it.
	depthHistogram = new HistogramWidget(100, 5'000, 100, 20'000, ui.centralWidget);
	depthHistogram->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
	loggingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
	cycleTimeWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	dock = new CloseDockWidget("Cycle Time", this);
	dock->setObjectName("cycleTimeWidgetDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(cycleTimeWidget);
	dock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);


#pragma endregion


#pragma region FRAME_COMPARE_WIDGET

	frameCompareWidget = new FrameCompareWidget(this);
	frameCompareWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dock = new CloseDockWidget("Compare", this);
	dock->setObjectName("frameCompareWidgetDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(frameCompareWidget);
	dock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, dock);

#pragma endregion


#pragma region FILTER_EDITOR_WIDGET

	filterEditorWidget = new FilterEditorWidget(this);
	filterEditorWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	dock = new CloseDockWidget("Filter Editor", this);
	dock->setObjectName("filterEditorWidgetDock");
	dock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	dock->setWidget(filterEditorWidget);
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

void SickGUI::initializeWeb()
{
	webSocket = new QAutoWebSocket(QUrl("ws://127.0.0.1:3000"), this);
	webSocket->start();
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

	if (cameraIpAddress == "")
	{
		visionary::VisionaryAutoIPScanCustom scanner;
		auto devices = scanner.doScan(10000);
		if (devices.empty())
			return false;
		else
			cameraIpAddress = devices.front().IpAddress;
	}

	camera = new(std::nothrow) VisionaryCamera(cameraIpAddress);

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
		std::string msg = "Failed to connect to camera and plc";
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
		makeConnections();
		// if both threads are good, show the ip's on the status bar and reset the cycle times
		QTimer::singleShot(1000, [this]()
			{
				auto camParams = camera->getParameters();
				std::string msg;
				if (camParams.count("ipAddress") != 0)
				{
					std::string cameraIpAddress = camParams.at("ipAddress");
					msg = "camera: " + cameraIpAddress + "  |  plc: " + plcIpAddress;
				}
				else
				{
					msg = "plc: " + plcIpAddress;
				}
				statusBarLabel->setText(msg.c_str());

				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetPlcTimes(); });
				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetCamTimes(); });
			});
	}
}

void SickGUI::makeConnections()
{
	QObject::connect(&renderThread, &RenderThread::renderedImage, this, &SickGUI::updateDisplay);
	QObject::connect(captureThread, &CaptureThread::addTime, cycleTimeWidget, &CycleTimeWidget::addCamTime);
	QObject::connect(plcThread, &PlcThread::addTime, cycleTimeWidget, &CycleTimeWidget::addPlcTime);
	QObject::connect(cameraView, &CameraViewWidget::newMask, this, [=](const QRectF& mask) { captureThread->setMask(mask); });
	QObject::connect(cameraView, &CameraViewWidget::setEnableMask, this, [=](const bool enable) { captureThread->setEnableMask(enable); });
}

void SickGUI::startThreads(QPromise<ThreadResult>& promise)
{
	promise.addResult(startCamThread(), 0);
	promise.addResult(startPlcThread(), 1);
	renderThread.start();
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
	catch (...)
	{
		qCritical() << "exception while starting camera thread";
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
		int connectRet = s7Client->ConnectTo(plcIpAddress.c_str(), plcRack, plcSlot);
		if (0 != connectRet)
		{
			qCritical() << "failed to connect plc " << plcIpAddress << ": " << CliErrorText(connectRet);
			ret.error = true;
			ret.message = "failed to connect plc " + QString(plcIpAddress.c_str()) + ": " + QString(CliErrorText(connectRet).c_str());
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

		plcThread->setCycleTimeTarget(plcCycleTimeTarget);
		ret.error = !plcThread->startPlc(s7Client);
		if (!ret.error)
		{
			qInfo() << "plc thread success";
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
	catch (...)
	{
		qCritical() << "exception while starting plc thread:";
		ret.error = true;
		ret.message = "failed to start plc thread";
		return ret;
	}
}

void SickGUI::saveLayout()
{
	QSettings settings("WF", "SickGUI");
	settings.setValue("windowState", this->saveState());
	settings.setValue("geometry", this->saveGeometry());

	settings.sync();
}

void SickGUI::restoreLayout()
{
	QSettings settings("WF", "SickGUI");
	this->restoreState(settings.value("windowState").toByteArray());
	this->restoreGeometry(settings.value("geometry").toByteArray());
}

void SickGUI::loadConfiguration()
{
	QSettings settings(CONFIG_PATH, QSettings::Format::IniFormat);

	// plc
	plcIpAddress = settings.value("plc/ip", "").value<QString>().toStdString();
	plcRack = settings.value("plc/rack", 0).value<qint16>();
	plcSlot = settings.value("plc/slot", 0).value<qint16>();
	plcCycleTimeTarget = settings.value("plc/cycle_time_target_ms", 10).value<qint64>();

	// camera
	cameraIpAddress = settings.value("camera/ip", "").value<QString>().toStdString();

	// ui
	renderThread.setCycleTimeTarget(settings.value("ui/render_refresh_interval_ms", 100).value<qint16>());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                                      SLOTS                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void SickGUI::playVideo()
{
	renderThread.setPaused(false);
	ui.actionPlay->setEnabled(false);
	ui.actionPause->setEnabled(true);
}

void SickGUI::pauseVideo()
{
	renderThread.setPaused(true);
	ui.actionPlay->setEnabled(true);
	ui.actionPause->setEnabled(false);
}

void SickGUI::showStatusBarMessage(const QString& text, int timeout)
{
	QMetaObject::invokeMethod(this, [this, text, timeout]()
		{
			statusBar()->showMessage(text, timeout);
		});
}

void SickGUI::updateDisplay(const QImage& lastImage)
{
	if (this->isMinimized())
		return;

	QMetaObject::invokeMethod(this, [=]()
		{
			auto pixmap = QPixmap::fromImage(lastImage);
			cameraView->setPixmap(pixmap);
		});

	frameset::Frameset fs = threadInterface.peekGuiFrame();
	if (frameset::isValid(fs.depth))
		return;

	QMetaObject::invokeMethod(this, [=]()
		{
			depthHistogram->updateHistogram(fs.depth.data);
			depthHistogram->update();
		});
}
