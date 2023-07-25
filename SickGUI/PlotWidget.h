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
	QLineSeries* series = nullptr;
	QValueAxis* axisX = nullptr;
	QValueAxis* axisY = nullptr;
	uint32_t maxY = 0;
	uint32_t minY = std::numeric_limits<uint32_t>::max();
};

