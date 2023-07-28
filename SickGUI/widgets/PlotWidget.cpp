#include "PlotWidget.h"
#include <algorithm>

PlotWidget::PlotWidget(QWidget* parent) : 
	axisX(new QValueAxis(this)),
	axisY(new QValueAxis(this)),
	seriesRaw(new QLineSeries(this)),
	QChartView(new QChart(), parent)
{
	chart()->addSeries(seriesRaw);
	chart()->setAxisX(axisX, seriesRaw);
	chart()->setAxisY(axisY, seriesRaw);

	chart()->setDropShadowEnabled(false);
	chart()->legend()->hide();
}

PlotWidget::~PlotWidget()
{
}

void PlotWidget::pushData(const uint32_t val)
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
	const auto minMaxY = minMaxYValues(seriesRaw->points());
	const qreal lowerY = std::floor(minMaxY.first * 0.99);
	const qreal upperY = std::floor(minMaxY.second * 1.01);
	axisY->setRange(lowerY, upperY);
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