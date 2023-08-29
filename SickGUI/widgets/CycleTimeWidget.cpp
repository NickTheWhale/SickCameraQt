#include "CycleTimeWidget.h"
#include "CycleTime.h"
#include <qtimer.h>

CycleTimeWidget::CycleTimeWidget(QWidget* parent) : QWidget(parent)
{
	QGridLayout* grid = new QGridLayout(this);

	plcTimes = new CycleTime("Plc", 100, this);
	camTimes = new CycleTime("Camera", 100, this);

	grid->addWidget(plcTimes, 0, 0, 1, 1);
	grid->addWidget(camTimes, 1, 0, 1, 1);

	setLayout(grid);
}

void CycleTimeWidget::nullPlcTimes()
{
	plcTimes->nullTimes();
}

void CycleTimeWidget::nullCamTimes()
{
	camTimes->nullTimes();
}

void CycleTimeWidget::addPlcTime(const int time)
{
	plcTimes->add(time);

	if (!plcWatchDog)
	{
		plcWatchDog = new QTimer(this);
		plcWatchDog->callOnTimeout(this, &CycleTimeWidget::nullPlcTimes);
	}
	int timeOut = plcTimes->times().current > 0 ? std::max(plcTimes->times().current * 5, minWatchDogDuration) : minWatchDogDuration;
	plcWatchDog->start(timeOut);
}

void CycleTimeWidget::addCamTime(const int time)
{
	camTimes->add(time);

	if (!camWatchDog)
	{
		camWatchDog = new QTimer(this);
		camWatchDog->callOnTimeout(this, &CycleTimeWidget::nullCamTimes);
	}
	int timeOut = camTimes->times().current > 0 ? std::max(camTimes->times().current * 5, minWatchDogDuration) : minWatchDogDuration;
	camWatchDog->start(timeOut);
}

void CycleTimeWidget::resetPlcTimes()
{
	plcTimes->reset();
}

void CycleTimeWidget::resetCamTimes()
{
	camTimes->reset();
}