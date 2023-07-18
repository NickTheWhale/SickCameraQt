#include "CycleTime.h"


#pragma once
#include <qwidget.h>
#include <boost/circular_buffer.hpp>
#include <qgridlayout.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qsizepolicy.h>

CycleTime::CycleTime(const QString title, const size_t bufferSize, QWidget* parent) : QWidget(parent), buffer(bufferSize)
{
	QGridLayout* grid = new QGridLayout(this);

	grid->addWidget(new QLabel("Current:", this), 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
	grid->addWidget(new QLabel("Average:", this), 0, 2, Qt::AlignLeft | Qt::AlignVCenter);
	grid->addWidget(new QLabel("Worst:", this), 0, 4, Qt::AlignLeft | Qt::AlignVCenter);
	grid->addWidget(new QLabel("P75:", this), 0, 6, Qt::AlignLeft | Qt::AlignVCenter);
	grid->addWidget(new QLabel("P95:", this), 0, 8, Qt::AlignLeft | Qt::AlignVCenter);

	currentLabel = new QLabel("--", this); grid->addWidget(currentLabel, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
	averageLabel = new QLabel("--", this); grid->addWidget(averageLabel, 0, 3, Qt::AlignLeft | Qt::AlignVCenter);
	worstLabel = new QLabel("--", this); grid->addWidget(worstLabel, 0, 5, Qt::AlignLeft | Qt::AlignVCenter);
	p75Label = new QLabel("--", this); grid->addWidget(p75Label, 0, 7, Qt::AlignLeft | Qt::AlignVCenter);
	p95Label = new QLabel("--", this); grid->addWidget(p95Label, 0, 9, Qt::AlignLeft | Qt::AlignVCenter);

	QPushButton* resetButton = new QPushButton(QIcon(":/SickGUI/icons/restart_alt_FILL0_wght400_GRAD0_opsz20.png"), "", this);
	resetButton->setMaximumSize(QSize(20, 20));
	QObject::connect(resetButton, &QPushButton::pressed, [this]()
		{
			reset();
		});

	grid->addWidget(resetButton, 0, 10, Qt::AlignRight | Qt::AlignVCenter);

	QGroupBox* group = new QGroupBox(title, this);
	group->setLayout(grid);
	QWidget::setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
};

void CycleTime::reset()
{
	currentLabel->setText("--");
	averageLabel->setText("--");
	worstLabel->setText("--");
	p75Label->setText("--");
	p95Label->setText("--");
	buffer.clear();
	stats.worst = 0;
};

void CycleTime::add(const int time)
{
	if (time > 0)
	{
		buffer.push_back(time);
		update();
	}
};

void CycleTime::update()
{
	if (buffer.empty())
		return;

	// current
	stats.current = buffer.back();

	// average & worst
	int accum = 0;
	for (const int& value : buffer)
	{
		if (value > stats.worst)
			stats.worst = value;

		accum += value;
	}
	stats.average = accum / buffer.size();

	// p75 & p95
	boost::circular_buffer<int> sortedBuffer(buffer);
	std::sort(sortedBuffer.begin(), sortedBuffer.end());
	double p75Index = 0.75 * (sortedBuffer.size() - 1);
	double p95Index = 0.95 * (sortedBuffer.size() - 1);
	stats.p75 = sortedBuffer[static_cast<int>(p75Index)];
	stats.p95 = sortedBuffer[static_cast<int>(p95Index)];

	currentLabel->setText(QString::number(stats.current));
	averageLabel->setText(QString::number(stats.average));
	worstLabel->setText(QString::number(stats.worst));
	p75Label->setText(QString::number(stats.p75));
	p95Label->setText(QString::number(stats.p95));
}
