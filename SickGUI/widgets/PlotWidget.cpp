#include "PlotWidget.h"
#include <algorithm>

PlotWidget::PlotWidget(QWidget* parent) : 
	axisX(new QValueAxis(this)),
	axisY(new QValueAxis(this)),
	seriesRaw(new QLineSeries(this)),
	//seriesAverage(new QLineSeries(this)),
	QChartView(new QChart(), parent)
{
	chart()->addSeries(seriesRaw);
	chart()->setAxisX(axisX, seriesRaw);
	chart()->setAxisY(axisY, seriesRaw);
	
	//chart()->addSeries(seriesAverage);
	//chart()->setAxisX(axisX, seriesAverage);
	//chart()->setAxisY(axisY, seriesAverage);

	chart()->setDropShadowEnabled(false);
	chart()->legend()->hide();
}

PlotWidget::~PlotWidget()
{
}

void PlotWidget::pushData(const uint32_t val)
{
	// add raw value
	seriesRaw->append(xPos, val);
	while (seriesRaw->count() > bufferSize)
		seriesRaw->remove(0);

	//// add average value
	//const qreal averageY = smoothYValues(seriesRaw->points(), 0.5);
	//seriesAverage->append(xPos, averageY);
	//while (seriesAverage->count() > bufferSize)
	//	seriesAverage->remove(0);

	// set x range
	qreal lowerX = std::max(static_cast<qreal>(xPos - bufferSize), static_cast<qreal>(0));
	axisX->setRange(lowerX, xPos);

	// set y range
	const auto minMaxY = minMaxYValues(seriesRaw->points());
	const qreal lowerY = std::floor(minMaxY.first * 0.99);
	const qreal upperY = std::floor(minMaxY.second * 1.01);
	axisY->setRange(lowerY, upperY);

	// inc. x pos.
	++xPos;
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

//const qreal PlotWidget::smoothYValues(const QList<QPointF>& points, const qreal alpha)
//{
//	//qreal accum = 0;
//	//const int depth_ = std::max(1, std::min(depth, bufferSize));
//	//const qsizetype count = points.count();
//	//
//	//int i;
//	//for (i = count - 1; i > depth_; --i)
//	//{
//	//	accum += points.at(i).y();
//	//}
//
//	//const qreal average = accum / (count - i);
//	//return average;
//
//
//	// Calculate the average value of seriesRaw (simple average)
//	qreal sum = 0;
//	for (const auto& point : seriesRaw->points())
//	{
//		sum += point.y();
//	}
//	qreal average = sum / seriesRaw->count();
//
//	// Return the smoothed average (you can adjust the smoothing factor as needed)
//	const qreal alpha_ = std::min(std::max(alpha, 0.0), 1.0);
//	return (1.0 - alpha) * average + alpha * seriesRaw->at(seriesRaw->count() - 1).y();
//}
