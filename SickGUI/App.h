#pragma once
#include <qapplication.h>
#include <qelapsedtimer.h>

class App : public QApplication
{
public:
	App(int& argc, char** argv);

	bool notify(QObject* receiver, QEvent* event) override;

private:
	QElapsedTimer timer;
};

