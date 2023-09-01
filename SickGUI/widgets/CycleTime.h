/*****************************************************************//**
 * @file   CycleTime.h
 * @brief  Maintains a circular buffer of cycle time samples 
 * and performs statistical calculations of the current, average, worst, 75th percentile, and 95th percentile.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <qwidget.h>
#include <boost/circular_buffer.hpp>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>

struct Times
{
	int current;
	int average;
	int worst;
	int p75;
	int p95;
};

class CycleTime : public QWidget
{

	Q_OBJECT

public:
	explicit CycleTime(const QString title, const size_t bufferSize = 100, QWidget* parent = nullptr);

	/**
	 * @brief Add the given time to internal buffer and update statistics.
	 * 
	 * @param time
	 */
	void add(const int time);
	
	/**
	 * @brief Clears and nulls times.
	 * 
	 */
	void reset();

	/**
	 * @brief Grays times.
	 * 
	 */
	void nullTimes();

	/**
	 * @brief Stats getters.
	 * 
	 * @return Times.
	 */
	const Times times() const;

private:
	boost::circular_buffer<int> buffer;
	Times _times = { 0, 0, 0, 0, 0 };
	QLabel* currentLabel;
	QLabel* averageLabel;
	QLabel* worstLabel;
	QLabel* p75Label;
	QLabel* p95Label;

	const QString title;
	bool isNulled = true;

	/**
	 * @brief Calculates statistics and displays them.
	 * 
	 */
	void update();

	/**
	 * @brief Makes labels black.
	 * 
	 */
	void unNullTimes();
};
