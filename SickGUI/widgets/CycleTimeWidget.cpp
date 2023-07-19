#include "CycleTimeWidget.h"
#include "CycleTime.h"
#include <qtimer.h>

CycleTimeWidget::CycleTimeWidget(QWidget* parent) : QWidget(parent)
{
	QGridLayout* grid = new QGridLayout(this);

	plcTimes = new CycleTime("Plc", 100, this);
	camTimes = new CycleTime("Camera", 100, this);
	webTimes = new CycleTime("Web", 100, this);

	grid->addWidget(plcTimes, 0, 0, 1, 1);
	grid->addWidget(camTimes, 1, 0, 1, 1);
	grid->addWidget(webTimes, 2, 0, 1, 1);

	setLayout(grid);


	plcWatchDog = new QTimer(this);
	camWatchDog = new QTimer(this);
	webWatchDog = new QTimer(this);

	plcWatchDog->setInterval(watchDogDuration);
	camWatchDog->setInterval(watchDogDuration);
	webWatchDog->setInterval(watchDogDuration);

	plcWatchDog->callOnTimeout(this, &CycleTimeWidget::nullPlcTimes);
	camWatchDog->callOnTimeout(this, &CycleTimeWidget::nullCamTimes);
	webWatchDog->callOnTimeout(this, &CycleTimeWidget::nullWebTimes);

	plcWatchDog->start();
	camWatchDog->start();
	webWatchDog->start();
}

void CycleTimeWidget::nullPlcTimes()
{
	plcTimes->nullTimes();
}

void CycleTimeWidget::nullCamTimes()
{
	camTimes->nullTimes();
}

void CycleTimeWidget::nullWebTimes()
{
	webTimes->nullTimes();
}

void CycleTimeWidget::addPlcTime(const int time)
{
	plcTimes->add(time);
	plcWatchDog->stop();
	plcWatchDog->start(plcTimes->times().p75 * 5);
}

void CycleTimeWidget::addCamTime(const int time)
{
	camTimes->add(time);
	camWatchDog->stop();
	camWatchDog->start(camTimes->times().p75 * 5);
}
void CycleTimeWidget::addWebTime(const int time)
{
	webTimes->add(time);
	webWatchDog->stop();
	webWatchDog->start(webTimes->times().p75 * 5);
}

void CycleTimeWidget::resetPlcTimes()
{
	plcTimes->reset();
}

void CycleTimeWidget::resetCamTimes()
{
	camTimes->reset();
}

void CycleTimeWidget::resetWebTimes()
{
	webTimes->reset();
}
