/*****************************************************************//**
 * @file   main.cpp
 * @brief  Attaches message handler and creates main window
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#include "SickGUI.h"
#include "App.h"
#include <qsharedmemory.h>
#include <qmessagebox.h>
#include "CustomMessageHandler.h"

CustomMessageHandler messageHandler;

void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
	messageHandler.handle(type, context, message);
	qDebug() << context.category << context.file << context.function << context.line << context.version;
}

int main(int argc, char* argv[])
{
	// this is used to redirect qDebug(), qWarning(), qCritical(), etc. to the logging widget.
	qInstallMessageHandler(msgHandler);

	App app(argc, argv);

	// this prevents having multiple instances
	QSharedMemory sharedMemory;
	sharedMemory.setKey("SickGUI-sharedMemoryKey");
	if (sharedMemory.create(1) == false)
	{
		QMessageBox::warning(nullptr, "Warning", "Another instance is already running");
		app.exit();
		return -1;
	}

	SickGUI window(&messageHandler);
	app.setWindowIcon(QIcon(":/SickGUI/icons/baseline_linked_camera_white_48dp.png"));
	window.show();

	return app.exec();
}
