/*****************************************************************//**
 * @file   CycleTimeWidget.h
 * @brief  Displays the cycle times of each thread and handles "graying out" stale times.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <qwidget.h>

class CycleTime;

class CycleTimeWidget : public QWidget
{
	Q_OBJECT

public slots:
	/**
	 * @brief Adds a time to plc time buffer for computation.
	 */
	void addPlcTime(const int time);

	/**
	 * @brief Adds a time to cam time buffer for computation.
	 */
	void addCamTime(const int time);

	/**
	 * @brief Clears plc times.
	 * 
	 */
	void resetPlcTimes();

	/**
	 * @brief Clears cam times.
	 * 
	 */
	void resetCamTimes();

public:
	explicit CycleTimeWidget(QWidget* parent = nullptr);

private:
	// minimum time required before graying out times
	const int minWatchDogDuration = 1000; /* ms */

	CycleTime* plcTimes;
	CycleTime* camTimes;

	QTimer* plcWatchDog = nullptr;
	QTimer* camWatchDog = nullptr;

	/**
	 * @brief Grays out plc times.
	 * 
	 */
	void nullPlcTimes();

	/**
	 * @brief Grays out cam times.
	 * 
	 */
	void nullCamTimes();
};
