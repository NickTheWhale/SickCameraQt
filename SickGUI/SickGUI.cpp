#include "SickGUI.h"

#include <format>

#include <qtimer.h>
#include <qmessagebox.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <qfuture.h>
#include <qactiongroup.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qtoolbutton.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qcoreapplication.h>
#include <qstatusbar.h>

#include <snap7.h>

#include <CloseDockWidget.h>
#include <HistogramWidget.h>
#include <VisionaryAutoIPScanCustom.h>
#include <global.h>

SickGUI::SickGUI(CustomMessageHandler* messageHandler, QWidget* parent) :
	messageHandler(messageHandler),
	threadInterface(ThreadInterface::instance()),
	QMainWindow(parent)
{
	loadConfiguration();

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
#pragma region LOGGING_DOCK
	loggingWidget = new LoggingWidget(this);
	loggingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	loggingWidget->setMaxLineCount(500);

	connect(messageHandler, &CustomMessageHandler::newMessage, loggingWidget, &LoggingWidget::showMessage);

	CloseDockWidget* loggingDock = new CloseDockWidget("Log", this);
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
	CloseDockWidget* cycleTimeDock = new CloseDockWidget("Cycle Time", this);
	cycleTimeDock->setObjectName("cycleTimeWidgetDock");
	cycleTimeDock->setAllowedAreas(Qt::DockWidgetArea::AllDockWidgetAreas);
	cycleTimeDock->setWidget(cycleTimeWidget);
	cycleTimeDock->adjustSize();
	addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, cycleTimeDock);
#pragma endregion

#pragma region TOOL_BAR
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
	connect(loggingDock, &CloseDockWidget::visibilityChanged, this, [=](bool visible) { logViewAction->setChecked(visible); });

	QAction* cycleTimeViewAction = viewMenu->addAction("Cycle Times");
	cycleTimeViewAction->setCheckable(true);
	connect(cycleTimeViewAction, &QAction::triggered, this, [=](bool checked) { cycleTimeDock->setHidden(!checked); });
	connect(cycleTimeDock, &CloseDockWidget::visibilityChanged, this, [=](bool visible) { cycleTimeViewAction->setChecked(visible); });

	connect(viewMenu, &QMenu::triggered, this, [=]()
		{
			logViewAction->setChecked(!loggingDock->isHidden());
			cycleTimeViewAction->setChecked(!cycleTimeDock->isHidden());
		});

	// menubar
	QMenuBar* menuBar = new QMenuBar(this);
	menuBar->addMenu(filterMenu);
	menuBar->addMenu(viewMenu);

	this->setMenuBar(menuBar);
#pragma endregion

#pragma region MISC

	statusLabel = new QLabel(this);

#pragma endregion

}

bool SickGUI::createCamera()
{
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
		//std::string msg = "Failed to connect to camera and plc";
		QString msg = QString("Failed to connect to camera and plc.\n\tCamera error: %1\n\tPlc error: %2").arg(camThreadResult.message).arg(plcThreadResult.message);
		QMessageBox::critical(this, "Error", msg);
	}
	else if (camThreadResult.error)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to camera");
	}
	else if (plcThreadResult.error)
	{
		QMessageBox::critical(this, "Error", "Failed to connect to plc");
	}
	//if (camThreadResult.error || plcThreadResult.error)
	//{
	//	QMessageBox::information(this, "Info", "Application will close now");
	//	QCoreApplication::quit();
	//}
	else
	{
		makeConnections();

		updateStatusBar();
		this->statusBar()->addPermanentWidget(statusLabel);

		QTimer::singleShot(1000, [this]()
			{
				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetPlcTimes(); });
				QMetaObject::invokeMethod(this, [this]() { cycleTimeWidget->resetCamTimes(); });
			});
	}
}

void SickGUI::makeConnections()
{
	QObject::connect(captureThread, &CaptureThread::addTime, cycleTimeWidget, &CycleTimeWidget::addCamTime);
	QObject::connect(plcThread, &PlcThread::addTime, cycleTimeWidget, &CycleTimeWidget::addPlcTime);
	QObject::connect(filterEditorWidget, &FilterEditorWidget::updatedFilters, this,
		[&](const QJsonArray& filters)
		{
			captureThread->setFilters(filters);
		});
	QObject::connect(captureThread, &CaptureThread::filtersApplied, filterEditorWidget, &FilterEditorWidget::captureFiltersApplied);
	QObject::connect(captureThread, &CaptureThread::filtersFailed, filterEditorWidget, &FilterEditorWidget::captureFiltersFailed);
	
	QObject::connect(captureThread, &CaptureThread::reconnected, this, [&]() { cameraConnected = true; updateStatusBar(); });
	QObject::connect(captureThread, &CaptureThread::disconnected, this, [&]() { cameraConnected = false; updateStatusBar();  });

	QObject::connect(plcThread, &PlcThread::reconnected, this, [&]() { plcConnected = true; updateStatusBar(); });
	QObject::connect(plcThread, &PlcThread::disconnected, this, [&]() { plcConnected = false; updateStatusBar(); });
}

void SickGUI::startThreads(QPromise<ThreadResult>& promise)
{
	promise.addResult(startCamThread(), 0);
	promise.addResult(startPlcThread(), 1);
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
			ret.message = openRet.message;
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
			ret.message = QString(CliErrorText(connectRet).c_str());
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
	const auto ConfigPath = global::CONFIG_FILE_RELATIVE_PATH;
	QSettings settings(ConfigPath, QSettings::Format::IniFormat);

	// plc
	plcIpAddress = settings.value("plc/ip", "").value<QString>().toStdString();
	plcRack = settings.value("plc/rack", 0).value<qint16>();
	plcSlot = settings.value("plc/slot", 0).value<qint16>();

	// camera
	cameraIpAddress = settings.value("camera/ip", "").value<QString>().toStdString();
}

void SickGUI::updateStatusBar()
{
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
