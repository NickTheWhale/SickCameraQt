#pragma once
#include <qwidget.h>
#include <boost/circular_buffer.hpp>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>

class CycleTime : public QWidget
{

	Q_OBJECT

public:
	struct Stats
	{
		int current;
		int average;
		int worst;
		int p75;
		int p95;
	};

	explicit CycleTime(const QString title, const size_t bufferSize = 100, QWidget* parent = nullptr);

		void reset();

		void add(const int time);

private:
	boost::circular_buffer<int> buffer;
	Stats stats = { 0, 0, 0, 0, 0 };
	QLabel* currentLabel;
	QLabel* averageLabel;
	QLabel* worstLabel;
	QLabel* p75Label;
	QLabel* p95Label;

	void update();
};
