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

	void resetPlcTimes();
	void resetCamTimes();
	void resetWebTimes();

public:
	explicit CycleTimeWidget(QWidget* parent = nullptr);

private:
	int watchDogDuration = 1000; /* ms */

	CycleTime* plcTimes;
	CycleTime* camTimes;
	CycleTime* webTimes;

	QTimer* plcWatchDog;
	QTimer* camWatchDog;
	QTimer* webWatchDog;

	void nullPlcTimes();
	void nullCamTimes();
	void nullWebTimes();
};
