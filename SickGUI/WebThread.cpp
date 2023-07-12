#include "WebThread.h"

bool WebThread::startWeb()
{
    start(QThread::Priority::NormalPriority);
    return true;
}

void WebThread::stopWeb()
{
    _stop = true;
}

void WebThread::run()
{
    while (!_stop)
    {

    }
}

void WebThread::newImage(const QImage& image)
{

}
