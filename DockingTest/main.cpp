#include "DockingTest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DockingTest w;
    w.show();
    return a.exec();
}
