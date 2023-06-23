#pragma once

#include <qwidget.h>
#include <vector>
#include <boost/histogram.hpp>

#define HISTOGRAM_AUTO_SCALE (false)

class HistogramWidget : public QWidget
{
	Q_OBJECT

public:
	explicit HistogramWidget(QWidget* parent = nullptr);
	void updateHistogram(const std::vector<uint16_t>& data);
	~HistogramWidget();

protected:
	void paintEvent(QPaintEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	struct HolderOfStaticHistogram {
		// put axis types here
		using axes_t = std::tuple<boost::histogram::axis::regular<>>;
		using hist_t = boost::histogram::histogram<axes_t>;
		hist_t histogram;
	};

	HolderOfStaticHistogram hs;

	const size_t MAX_BIN_VALUE = 20'000;
	const size_t MIN_BIN_VALUE = 1'000;
	const size_t NUM_BINS = 100;

	const uint16_t MAX_COUNT = 20'000;

	const size_t PADDING = 3;
	const size_t X_GRAPH_OFFSET = 50;
	const size_t Y_GRAPH_OFFSET = 50;
};
