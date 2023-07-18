#pragma once
#include <qwidget.h>

class CycleTime;

class CycleTimeWidget : public QWidget
{
	Q_OBJECT

public slots:
	void addPlcTime(const int time);
	void addCamTime(const int time);
	void addWebTime(const int time);
	void addGuiTime(const int time);

public:
	explicit CycleTimeWidget(QWidget* parent = nullptr);

private:
	CycleTime* plcTimes;
	CycleTime* camTimes;
	CycleTime* webTimes;
	CycleTime* guiTimes;
};
