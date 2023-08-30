/*****************************************************************//**
 * @file   App.h
 * @brief  Used for performance testing signal and slot computation time.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

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

