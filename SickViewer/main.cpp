#include "SickViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SickViewer w;
    w.show();
    return a.exec();
}
