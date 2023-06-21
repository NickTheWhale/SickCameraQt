#pragma once

#include <qwidget.h>
#include <qchart.h>
#include <qbarseries.h>
#include <qchartview.h>
#include <qbarset.h>
#include <qstring.h>
#include <vector>
#include <boost/histogram.hpp>

class HistogramWidget : public QChart
{
	Q_OBJECT

public:
	explicit HistogramWidget(QWidget* parent = nullptr);
	void updateHistogram(const std::vector<uint16_t>& data);
	~HistogramWidget();

private:
	struct HolderOfStaticHistogram {
		// put axis types here
		using axes_t = std::tuple<boost::histogram::axis::regular<>>;
		using hist_t = boost::histogram::histogram<axes_t>;
		hist_t histogram;
	};

	HolderOfStaticHistogram hs;

	QBarSet* barSet = nullptr;
	QBarSeries* series = nullptr;
};
