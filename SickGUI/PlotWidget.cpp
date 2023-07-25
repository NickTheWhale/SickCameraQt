#include "PlotWidget.h"

PlotWidget::PlotWidget(QWidget* parent) : 
	axisX(new QValueAxis(this)),
	axisY(new QValueAxis(this)),
	series(new QLineSeries(this)),
	QChartView(new QChart(), parent)
{
	chart()->addSeries(series);
	chart()->setAxisX(axisX, series);
	chart()->setAxisY(axisY, series);

	chart()->setDropShadowEnabled(false);
	chart()->legend()->hide();
}

PlotWidget::~PlotWidget()
{
}

void PlotWidget::pushData(const uint32_t val)
{
	series->append(xPos++, val);
	qreal lower = std::max(static_cast<qreal>(xPos - bufferSize), static_cast<qreal>(0));
	axisX->setRange(lower, xPos);
	
	if (series->count() > bufferSize)
		series->remove(0);
	
	if (val > maxY)
		maxY = val;
	
	if (val < minY)
		minY = val;

	axisY->setRange(minY * 0.9, maxY * 1.1);
}
