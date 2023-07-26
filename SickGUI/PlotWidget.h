#pragma once
#include <qchartview.h>
#include <qlineseries.h>
#include <qvalueaxis.h>

class PlotWidget : public QChartView
{
	Q_OBJECT

public:
	explicit PlotWidget(QWidget* parent = nullptr);
	~PlotWidget();

	void pushData(const uint32_t val);

private:
	const int bufferSize = 100;
	uint64_t xPos = 0;
	QLineSeries* seriesRaw = nullptr;
	QLineSeries* seriesAverage = nullptr;
	QValueAxis* axisX = nullptr;
	QValueAxis* axisY = nullptr;

	const std::pair<qreal, qreal> minMaxYValues(const QList<QPointF>& points);
	const qreal smoothYValues(const QList<QPointF>& points, const qreal alpha);
};

