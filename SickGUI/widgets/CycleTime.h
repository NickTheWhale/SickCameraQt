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
	void add(const int time);
	void reset();
	const Times times() const;
	void nullTimes();

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

	void update();
	void unNullTimes();
};
