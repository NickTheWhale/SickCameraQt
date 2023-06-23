#include "SickGUI.h"
#include <QtWidgets/QApplication>
#include "Camera.h"
#include "VisionaryCamera.h"
#include <qsharedmemory.h>
#include <qmessagebox.h>


int main(int argc, char *argv[])
{
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

    SickGUI w;

    w.show();
    return a.exec();
}
