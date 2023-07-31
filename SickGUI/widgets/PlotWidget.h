#pragma once
#include <qchartview.h>
#include <qlineseries.h>
#include <qvalueaxis.h>
#include <qradiobutton.h>
#include <qspinbox.h>

class PlotWidget : public QWidget
{
	Q_OBJECT

public:
	explicit PlotWidget(QWidget* parent = nullptr);
	~PlotWidget();

	void pushData(const uint16_t val);

private:
	const int bufferSize = 80;
	int64_t xPos = 0;
	QLineSeries* seriesRaw = nullptr;
	QValueAxis* axisX = nullptr;
	QValueAxis* axisY = nullptr;
	QChartView* chartView = nullptr;
	QRadioButton* autoScaleBtn = nullptr;
	QSpinBox* lowerSpinBox = nullptr;
	QSpinBox* upperSpinBox = nullptr;

	const std::pair<qreal, qreal> minMaxYValues(const QList<QPointF>& points);
};