#include "WebWorker.h"

WebWorker::WebWorker()
{
}

WebWorker::~WebWorker()
{
}

void WebWorker::process()
{
	qDebug() << "WebWorker process";
	emit finished();
}
