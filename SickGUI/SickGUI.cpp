/*****************************************************************//**
 * @file   SickGUI.cpp
 * @brief  Main GUI class
 *
 * Creates UI components and threads.
 *
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "SickGUI.h"

#include <snap7.h>

#include <qtimer.h>
#include <qmessagebox.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <qfuture.h>
#include <qsizepolicy.h>
#include <qmenu.h>
#include <qsettings.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qstatusbar.h>
#include <qdockwidget.h>
#include <qdesktopservices.h>
#include <qapplication.h>

#include <global.h>

#include <format>

SickGUI::SickGUI(CustomMessageHandler* messageHandler, QWidget* parent) :
	messageHandler(messageHandler),
	threadInterface(ThreadInterface::instance()),
	QMainWindow(parent)
{
	// set some variables from the configuration file
	loadConfiguration();

	// initialize logging dock, filter editor, cycle time dock, and tool bar
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
	// make sure the threads have been started, otherwise we get null pointer issues
	if (threadWatcher && !threadWatcher->isFinished())
	{
		threadWatcher->waitForFinished();
	}

	// wait and delete capture thread
	if (captureThread)
	{
		captureThread->stopCapture();
		captureThread->wait(10'000 /*ms*/);
		delete captureThread;
		captureThread = nullptr;
	}

	// deleate camera
	if (camera)
	{
		delete camera;
		camera = nullptr;
	}

	// wait and delete plc thread
	if (plcThread)
	{
		plcThread->stopPlc();
		plcThread->wait(10'000 /*ms*/);
		delete plcThread;
		plcThread = nullptr;
	}

	// disconnect and delete plc
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
	// save the dock layout and window(s) geometry
	saveLayout();
	// pass the event on
	QMainWindow::closeEvent(event);
}

void SickGUI::initializeWidgets()
{
#pragma region LOGGING_DOCK
	loggingWidget = new LoggingWidget(this);
	loggingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	loggingWidget->setMaxLineCount(500);

	// connect our message handler to the log widget to redirect qDebug(), qWarning(), etc output
	connect(messageHandler, &CustomMessageHandler::newMessage, loggingWidget, &LoggingWidget::showMessage);

	QDockWidget* loggingDock = new QDockWidget("Log", this);
	// set the name so the dock layout can be saved
	loggingDock->setObjectName("loggingWidgetDock");
	loggingDock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	loggingDock->setWidget(loggingWidget);
	loggingDock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, loggingDock);
#pragma endregion

#pragma region FILTER_EDITOR
	filterEditorWidget = new FilterEditorWidget(this);
	filterEditorWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setCentralWidget(filterEditorWidget);
#pragma endregion

#pragma region CYCLE_TIME_DOCK
	cycleTimeWidget = new CycleTimeWidget(this);
	cycleTimeWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	QDockWidget* cycleTimeDock = new QDockWidget("Cycle Time", this);
	// set the name so the dock layout can be saved
	cycleTimeDock->setObjectName("cycleTimeWidgetDock");
	cycleTimeDock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	cycleTimeDock->setWidget(cycleTimeWidget);
	cycleTimeDock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, cycleTimeDock);
#pragma endregion

#pragma region MENU_BAR
	// filter menu
	QMenu* filterMenu = new QMenu("Filters", this);
	QAction* filterSaveAction = filterMenu->addAction("Save");
	connect(filterSaveAction, &QAction::triggered, filterEditorWidget, &FilterEditorWidget::save);

	QAction* filterLoadAction = filterMenu->addAction("Load");
	connect(filterLoadAction, &QAction::triggered, filterEditorWidget, &FilterEditorWidget::load);

	// view menu
	QMenu* viewMenu = new QMenu("View", this);
	QAction* logViewAction = viewMenu->addAction("Log");
	logViewAction->setCheckable(true);
	connect(logViewAction, &QAction::triggered, this, [=](bool checked) { loggingDock->setHidden(!checked); });
	connect(loggingDock, &QDockWidget::visibilityChanged, this, [=](bool visible) { logViewAction->setChecked(visible); });

	QAction* cycleTimeViewAction = viewMenu->addAction("Cycle Times");
	cycleTimeViewAction->setCheckable(true);
	connect(cycleTimeViewAction, &QAction::triggered, this, [=](bool checked) { cycleTimeDock->setHidden(!checked); });
	connect(cycleTimeDock, &QDockWidget::visibilityChanged, this, [=](bool visible) { cycleTimeViewAction->setChecked(visible); });

	// about menu
	QMenu* aboutMenu = new QMenu("About", this);
	QAction* aboutThis = aboutMenu->addAction("About");
	QAction* aboutQt = aboutMenu->addAction("About Qt");
	connect(aboutThis, &QAction::triggered, this, []() { QDesktopServices::openUrl(global::ABOUT_GITHUB_URL); });
	connect(aboutQt, &QAction::triggered, this, [=]() { QApplication::aboutQt(); });

	// used to sync state
	connect(viewMenu, &QMenu::triggered, this, [=]()
		{
			logViewAction->setChecked(!loggingDock->isHidden());
			cycleTimeViewAction->setChecked(!cycleTimeDock->isHidden());
		});

	// menubar
	QMenuBar* menuBar = new QMenuBar(this);
	menuBar->addMenu(filterMenu);
	menuBar->addMenu(viewMenu);
	menuBar->addMenu(aboutMenu);

	this->setMenuBar(menuBar);
#pragma endregion

#pragma region MISC

	// used to show ip addresses and connection status
	statusLabel = new QLabel(this);
	this->statusBar()->addPermanentWidget(statusLabel);

#pragma endregion
}

bool SickGUI::createCamera()
{
	// if we already have a camera, get rid of it
	if (camera)
	{
		delete camera;
		camera = nullptr;
	}

	camera = new(std::nothrow) VisionaryCamera(cameraIpAddress);

	return camera != nullptr;
}

void SickGUI::checkThreads()
{
	// resultAt(index) order matters here
	ThreadResult camThreadResult = threadWatcher->resultAt(0);
	ThreadResult plcThreadResult = threadWatcher->resultAt(1);

	// if both threads have an error, say that
	if (camThreadResult.error && plcThreadResult.error)
	{
		//std::string msg = std::format("Failed to connect to camera and plc.\nCamera error: {}\nPlc error: {}", camThreadResult.message, plcThreadResult.message);
		//std::string msg = "Failed to connect to camera and plc";
		QString msg = QString("Failed to connect to camera and plc.\n\tCamera error: %1\n\tPlc error: %2").arg(camThreadResult.message).arg(plcThreadResult.message);
		QMessageBox::critical(this, "Error", msg);
	}
	// if only the camera has an error, say that instead
	else if (camThreadResult.error)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to camera");
	}
	// if only the plc has an error, say that instead
	else if (plcThreadResult.error)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to plc");
	}
	// if either thread has an error, say that
	if (camThreadResult.error || plcThreadResult.error)
	{
		constexpr auto msg = """Due to being unable to connect the camera and/or plc"""
			""", the application is only usable as a node editor"""
			""". Please check your configuration and restart to"""
			""" enable camera and plc functionality""";
		QMessageBox::information(this, "Info", msg);
	}
	// if both threads threads started successfully
	else
	{
		// connect up components
		makeConnections();

		// display the ip addresses
		updateStatusBar();

		// reset the cycle time buffers in a second so the times are more accurate since the cycle times should be more stable
		QTimer::singleShot(1000, [this]()
			{
				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetPlcTimes(); });
				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetCamTimes(); });
			});
	}
}

void SickGUI::makeConnections()
{
	// cycle times
	QObject::connect(captureThread, &CaptureThread::addTime, cycleTimeWidget, &CycleTimeWidget::addCamTime);
	QObject::connect(plcThread, &PlcThread::addTime, cycleTimeWidget, &CycleTimeWidget::addPlcTime);

	// filters
	QObject::connect(filterEditorWidget, &FilterEditorWidget::updatedFilters, this,
		[&](const QJsonArray& filters)
		{
			captureThread->setFilters(filters);
		});

	// filter status
	QObject::connect(captureThread, &CaptureThread::filtersApplied, filterEditorWidget, &FilterEditorWidget::captureFiltersApplied);
	QObject::connect(captureThread, &CaptureThread::filtersFailed, filterEditorWidget, &FilterEditorWidget::captureFiltersFailed);

	// camera connection status
	QObject::connect(captureThread, &CaptureThread::reconnected, this, [&]() { cameraConnected = true; updateStatusBar(); });
	QObject::connect(captureThread, &CaptureThread::disconnected, this, [&]() { cameraConnected = false; updateStatusBar();  });

	// plc connection status
	QObject::connect(plcThread, &PlcThread::reconnected, this, [&]() { plcConnected = true; updateStatusBar(); });
	QObject::connect(plcThread, &PlcThread::disconnected, this, [&]() { plcConnected = false; updateStatusBar(); });
}

void SickGUI::startThreads(QPromise<ThreadResult>& promise)
{
	// order here matters, must be reflected in checkThreads()
	promise.addResult(startCamThread(), 0);
	promise.addResult(startPlcThread(), 1);
}

ThreadResult SickGUI::startCamThread()
{
	ThreadResult ret;
	try {
		qInfo() << "starting camera thread";

		// create a camera if needed
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

		// open camera
		qInfo() << "opening camera";
		OpenResult openRet = camera->open();
		if (openRet.error != ErrorCode::NONE_ERROR)
		{
			delete camera;
			camera = nullptr;
			qCritical() << "failed to open camera: " << openRet.message;
			ret.error = true;
			ret.message = openRet.message;
			return ret;
		}

		// create capture thread if needed
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

		// pass the camera to the capture thread and start capturing frames
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
		// if we already have a plc client, delete it
		if (s7Client)
		{
			if (s7Client->Connected())
				s7Client->Disconnect();
			delete s7Client;
			s7Client = nullptr;
		}

		// make plc client
		qInfo() << "creating plc client";
		s7Client = new TS7Client();

		// connect plc client
		qInfo() << "connecting plc client";
		int connectRet = s7Client->ConnectTo(plcIpAddress.c_str(), plcRack, plcSlot);
		if (0 != connectRet)
		{
			qCritical() << "failed to connect plc " << plcIpAddress << ": " << CliErrorText(connectRet);
			ret.error = true;
			ret.message = QString(CliErrorText(connectRet).c_str());
			return ret;
		}

		// make plc thread if needed
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

		// pass plc client to plc thread and start it
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
	// save settings to windows registry
	QSettings settings("WF", "SickGUI");
	settings.setValue("windowState", this->saveState());
	settings.setValue("geometry", this->saveGeometry());

	settings.sync();
}

void SickGUI::restoreLayout()
{
	// retrieve settings from windows registry
	QSettings settings("WF", "SickGUI");
	this->restoreState(settings.value("windowState").toByteArray());
	this->restoreGeometry(settings.value("geometry").toByteArray());
}

void SickGUI::loadConfiguration()
{
	// load settings from config file
	const auto ConfigPath = global::CONFIG_FILE_RELATIVE_PATH;
	QSettings settings(ConfigPath, QSettings::Format::IniFormat);

	// set plc settings
	plcIpAddress = settings.value("plc/ip", "").value<QString>().toStdString();
	plcRack = settings.value("plc/rack", 0).value<qint16>();
	plcSlot = settings.value("plc/slot", 0).value<qint16>();

	// set camera settings
	cameraIpAddress = settings.value("camera/ip", "").value<QString>().toStdString();
}

void SickGUI::updateStatusBar()
{
	// updates ip address color, if needed (red = disconnected, green = connected)
	if (plcConnected != lastPlcConnected || cameraConnected != lastCameraConnected)
	{
		lastPlcConnected = plcConnected;
		lastCameraConnected = cameraConnected;

		const QString connectedStyle = "color: green;";
		const QString disconnectedStyle = "color: red;";

		const QString text = QString("camera: <span style=\"%1\">%2</span> | plc: <span style=\"%3\">%4</span>")
			.arg(cameraConnected ? connectedStyle : disconnectedStyle)
			.arg(cameraIpAddress.c_str())
			.arg(plcConnected ? connectedStyle : disconnectedStyle)
			.arg(plcIpAddress.c_str());

		statusLabel->setText(text);
	}
}
