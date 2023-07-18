#include "CycleTimeWidget.h"
#include "CycleTime.h"

CycleTimeWidget::CycleTimeWidget(QWidget* parent) : QWidget(parent)
{
	QGridLayout* grid = new QGridLayout(this);

	plcTimes = new CycleTime("Plc", 100, this);
	camTimes = new CycleTime("Camera", 100, this);
	webTimes = new CycleTime("Web", 100, this);
	guiTimes = new CycleTime("Gui", 100, this);

	grid->addWidget(plcTimes, 0, 0, 1, 1);
	grid->addWidget(camTimes, 1, 0, 1, 1);
	grid->addWidget(webTimes, 2, 0, 1, 1);
	grid->addWidget(guiTimes, 3, 0, 1, 1);

	setLayout(grid);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CycleTimeWidget::addPlcTime(const int time)
{
	plcTimes->add(time);
}

void CycleTimeWidget::addCamTime(const int time)
{
	camTimes->add(time);
}
void CycleTimeWidget::addWebTime(const int time)
{
	webTimes->add(time);
}
void CycleTimeWidget::addGuiTime(const int time)
{
	guiTimes->add(time);
}