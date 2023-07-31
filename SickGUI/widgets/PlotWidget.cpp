#include "PlotWidget.h"
#include <algorithm>
#include <qvalidator.h>
#include <qlabel.h>
#include <qfontmetrics.h>
#include <qlayout.h>
#include <qpushbutton.h>

PlotWidget::PlotWidget(QWidget* parent) :
	axisX(new QValueAxis(this)),
	axisY(new QValueAxis(this)),
	seriesRaw(new QLineSeries(this)),
	chartView(new QChartView(this)),
	autoScaleBtn(new QRadioButton(this)),
	lowerSpinBox(new QSpinBox(this)),
	upperSpinBox(new QSpinBox(this)),
	QWidget(parent)
{

	chartView->chart()->addSeries(seriesRaw);
	chartView->chart()->setAxisX(axisX, seriesRaw);
	chartView->chart()->setAxisY(axisY, seriesRaw);

	chartView->chart()->setDropShadowEnabled(false);
	chartView->chart()->legend()->hide();

	autoScaleBtn->setText("Auto Scale");
	autoScaleBtn->setChecked(true);

	constexpr uint32_t min = std::numeric_limits<uint16_t>::min();
	constexpr uint32_t max = std::numeric_limits<uint16_t>::max();

	lowerSpinBox->setRange(min, max);
	lowerSpinBox->setValue(min);

	upperSpinBox->setRange(min, max);
	upperSpinBox->setValue(max);

	QPushButton* transferLimitsBtn = new QPushButton("Transfer Limits", this);
	connect(transferLimitsBtn, &QPushButton::pressed, this, [=]()
		{
			lowerSpinBox->setValue(axisY->min());
			upperSpinBox->setValue(axisY->max());
		});

	connect(lowerSpinBox, &QSpinBox::valueChanged, this, [=](int value)
		{
			if (upperSpinBox->value() < lowerSpinBox->value() + 1)
				upperSpinBox->setValue(value + 1);
		});

	connect(upperSpinBox, &QSpinBox::valueChanged, this, [=](int value)
		{
			if (lowerSpinBox->value() > upperSpinBox->value() - 1)
				lowerSpinBox->setValue(value - 1);
		});

	QHBoxLayout* hbox = new QHBoxLayout();

	hbox->addWidget(autoScaleBtn, 0, Qt::AlignmentFlag::AlignHCenter);
	hbox->addWidget(transferLimitsBtn);
	hbox->addWidget(lowerSpinBox);
	hbox->addWidget(upperSpinBox);

	QVBoxLayout* vbox = new QVBoxLayout(this);
	vbox->addWidget(chartView);
	vbox->addLayout(hbox);

	setLayout(vbox);
}

PlotWidget::~PlotWidget()
{
}

void PlotWidget::pushData(const uint16_t val)
{
	// inc. x pos.
	++xPos;
	// add raw value
	seriesRaw->append(xPos, val);
	while (seriesRaw->count() > bufferSize)
		seriesRaw->remove(0);

	// set x range
	qreal lowerX = std::max(static_cast<qreal>(xPos - bufferSize), static_cast<qreal>(0));
	axisX->setRange(lowerX, xPos);

	// set y range
	if (autoScaleBtn->isChecked())
	{
		const auto minMaxY = minMaxYValues(seriesRaw->points());
		const qreal lowerY = std::floor(minMaxY.first * 0.999);
		const qreal upperY = std::floor(minMaxY.second * 1.001);
		axisY->setRange(lowerY, upperY);
	}
	else
	{
		axisY->setRange(lowerSpinBox->value(), upperSpinBox->value());
	}
}

const std::pair<qreal, qreal> PlotWidget::minMaxYValues(const QList<QPointF>& points)
{
	qreal minY = std::numeric_limits<qreal>::max();
	qreal maxY = std::numeric_limits<qreal>::min();

	for (const auto& point : points)
	{
		const qreal y = point.y();
		if (y > maxY)
			maxY = y;
		if (y < minY)
			minY = y;
	}

	return std::make_pair(minY, maxY);
}