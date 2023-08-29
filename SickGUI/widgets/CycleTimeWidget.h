#pragma once
#include <qwidget.h>

class CycleTime;

class CycleTimeWidget : public QWidget
{
	Q_OBJECT

public slots:
	void addPlcTime(const int time);
	void addCamTime(const int time);

	void resetPlcTimes();
	void resetCamTimes();

public:
	explicit CycleTimeWidget(QWidget* parent = nullptr);

private:
	const int minWatchDogDuration = 1000; /* ms */

	CycleTime* plcTimes;
	CycleTime* camTimes;

	QTimer* plcWatchDog = nullptr;
	QTimer* camWatchDog = nullptr;

	void nullPlcTimes();
	void nullCamTimes();
};
