#include "SickGUI.h"
#include <QtWidgets/QApplication>
#include "Camera.h"
#include "VisionaryCamera.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SickGUI w;

    //if (w.failed())
    //    return -1;

    w.show();
    return a.exec();
}
