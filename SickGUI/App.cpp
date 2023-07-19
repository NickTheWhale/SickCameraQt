#include "App.h"

App::App(int& argc, char** argv) : QApplication(argc, argv)
{
}

//bool App::notify(QObject* receiver, QEvent* event)
//{
//	timer.start();
//	bool ret = QApplication::notify(receiver, event);
//	qint64 elapsed = timer.elapsed();
//	if (elapsed > 10)
//	{
//		qDebug() << "Processing event type" << event->type()
//			<< "for object" << receiver->objectName()
//			<< "took" << elapsed;
//	}
//	return ret;
//}

bool App::notify(QObject* receiver, QEvent* event)
{
	return QApplication::notify(receiver, event);
}
