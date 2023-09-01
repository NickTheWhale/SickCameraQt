/*****************************************************************//**
 * @file   CycleTime.cpp
 * @brief  Maintains a circular buffer of cycle time samples
 * and performs statistical calculations of the current, average, worst, 75th percentile, and 95th percentile.
 *
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "CycleTime.h"

#include <qsizepolicy.h>

CycleTime::CycleTime(const QString title, const size_t bufferSize, QWidget* parent)
	: buffer(bufferSize), title(title), QWidget(parent)
{
	// create layouts
	QGridLayout* mainGrid = new QGridLayout(this);
	QGridLayout* groupGrid = new QGridLayout();

	QGroupBox* group = new QGroupBox(title, this);

	// create time labels
	groupGrid->addWidget(new QLabel("Current:", this), 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
	groupGrid->addWidget(new QLabel("Average:", this), 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
	groupGrid->addWidget(new QLabel("Worst:", this), 0, 4, Qt::AlignLeft | Qt::AlignVCenter);
	groupGrid->addWidget(new QLabel("P75:", this), 0, 6, Qt::AlignLeft | Qt::AlignVCenter);
	groupGrid->addWidget(new QLabel("P95:", this), 0, 8, Qt::AlignLeft | Qt::AlignVCenter);

	// create time values
	currentLabel = new QLabel("--", this); groupGrid->addWidget(currentLabel, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
	averageLabel = new QLabel("--", this); groupGrid->addWidget(averageLabel, 0, 3, Qt::AlignLeft | Qt::AlignVCenter);
	worstLabel = new QLabel("--", this); groupGrid->addWidget(worstLabel, 0, 5, Qt::AlignLeft | Qt::AlignVCenter);
	p75Label = new QLabel("--", this); groupGrid->addWidget(p75Label, 0, 7, Qt::AlignLeft | Qt::AlignVCenter);
	p95Label = new QLabel("--", this); groupGrid->addWidget(p95Label, 0, 9, Qt::AlignLeft | Qt::AlignVCenter);

	// configure reset button

	QPushButton* resetButton = new QPushButton(QIcon(":/SickGUI/icons/restart_alt_FILL0_wght400_GRAD0_opsz20.png"), "", this);
	resetButton->setMaximumSize(QSize(20, 20));
	QObject::connect(resetButton, &QPushButton::pressed, this, &CycleTime::reset);

	// layout stuff

	groupGrid->addWidget(resetButton, 0, 10, Qt::AlignRight | Qt::AlignVCenter);
	groupGrid->setContentsMargins(QMargins(5, 0, 5, 3));
	group->setLayout(groupGrid);

	mainGrid->addWidget(group);
	mainGrid->setContentsMargins(QMargins(0, 0, 0, 0));

	setLayout(mainGrid);
	nullTimes();
}

void CycleTime::reset()
{
	currentLabel->setText("--");
	averageLabel->setText("--");
	worstLabel->setText("--");
	p75Label->setText("--");
	p95Label->setText("--");
	buffer.clear();
	_times.worst = 0;
}

const Times CycleTime::times() const
{
	return _times;
}

void CycleTime::add(const int time)
{
	// add the time to the buffer if its positive, and update the statistics
	if (time >= 0)
	{
		buffer.push_back(time);
		update();
	}
}

void CycleTime::nullTimes()
{
	// gray out the labels

	currentLabel->setStyleSheet("color: gray;");
	averageLabel->setStyleSheet("color: gray;");
	worstLabel->setStyleSheet("color: gray;");
	p75Label->setStyleSheet("color: gray;");
	p95Label->setStyleSheet("color: gray;");
}

void CycleTime::update()
{
	if (buffer.empty())
		return;

	// current
	_times.current = buffer.back();

	// average & worst
	int accum = 0;
	for (const int& value : buffer)
	{
		if (value > _times.worst)
		{
			_times.worst = value;
		}

		accum += value;
	}
	_times.average = accum / buffer.size();

	// p75 & p95
	boost::circular_buffer<int> sortedBuffer(buffer);
	std::sort(sortedBuffer.begin(), sortedBuffer.end());
	double p75Index = 0.75 * (sortedBuffer.size() - 1);
	double p95Index = 0.95 * (sortedBuffer.size() - 1);
	_times.p75 = sortedBuffer[static_cast<int>(p75Index)];
	_times.p95 = sortedBuffer[static_cast<int>(p95Index)];

	if (isNulled)
	{
		unNullTimes();
	}

	if (buffer.full())
	{
		averageLabel->setText(QString::number(_times.average));
		p75Label->setText(QString::number(_times.p75));
		p95Label->setText(QString::number(_times.p95));
	}

	currentLabel->setText(QString::number(_times.current));
	worstLabel->setText(QString::number(_times.worst));
}

void CycleTime::unNullTimes()
{
	// make labels black

	currentLabel->setStyleSheet("color: black;");
	averageLabel->setStyleSheet("color: black;");
	worstLabel->setStyleSheet("color: black;");
	p75Label->setStyleSheet("color: black;");
	p95Label->setStyleSheet("color: black;");
}
