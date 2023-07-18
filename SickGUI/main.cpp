#include "SickGUI.h"
#include <qapplication.h>
#include <qsharedmemory.h>
#include <qmessagebox.h>
#include "CustomMessageHandler.h"

CustomMessageHandler messageHandler;

void msgHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
	messageHandler.handle(type, context, message);
}

int main(int argc, char* argv[])
{
	qInstallMessageHandler(msgHandler);

	QApplication a(argc, argv);

	// this prevents having multiple instances
	QSharedMemory sharedMemory;
	sharedMemory.setKey("SickGUI-sharedMemoryKey");
	if (sharedMemory.create(1) == false)
	{
		QMessageBox::warning(nullptr, "Warning", "Another instance is already running");
		a.exit();
		return -1;
	}

	SickGUI w(&messageHandler);

	w.show();
	return a.exec();
}
