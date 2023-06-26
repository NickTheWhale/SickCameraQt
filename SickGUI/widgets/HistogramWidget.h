#pragma once

#include <qwidget.h>
#include <vector>
#include <boost/histogram.hpp>
#include <algorithm>

class HistogramWidget : public QWidget
{
	Q_OBJECT

public:
	explicit HistogramWidget(size_t minVal, size_t maxVal, size_t bins, size_t maxCount, QWidget* parent = nullptr);
	~HistogramWidget();
	void updateHistogram(const std::vector<uint16_t>& data);

	template<typename T>
	void setXAxis(T start, T step, T stop, std::string label = "", bool reverse = false)
	{
		showXAxis = true;
		xAxisLabel = label;
		size_t size = (stop - start) / step;
		xAxis.clear();
		for (auto i = 0; i < size; ++i)
		{
			xAxis.push_back(start + i * step);
		}
		if (reverse)
			std::reverse(xAxis.begin(), xAxis.end());
	}

	template<typename T>
	void setYAxis(T start, T step, T stop, std::string label = "", bool reverse = false)
	{
		showYAxis = true;
		yAxisLabel = label;
		size_t size = (stop - start) / step;
		yAxis.clear();
		for (auto i = 0; i < size; ++i)
		{
			yAxis.push_back(start + i * step);
		}
		if (reverse)
			std::reverse(xAxis.begin(), xAxis.end());
	}

protected:
	void paintEvent(QPaintEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

private:
	bool showXAxis = false;
	bool showYAxis = false;

	std::string xAxisLabel = "";
	std::string yAxisLabel = "";

	void setGraphXOffsetForXAxis(bool forLabel);
	void setGraphYOffsetForYAxis(bool forLabel);

	std::vector<double> xAxis;
	std::vector<double> yAxis;

	struct HolderOfStaticHistogram {
		// put axis types here
		using axes_t = std::tuple<boost::histogram::axis::regular<>>;
		using hist_t = boost::histogram::histogram<axes_t>;
		hist_t histogram;
	};

	HolderOfStaticHistogram hs;

	const size_t minVal;
	const size_t maxVal;
	const size_t numBins;

	const size_t maxCount;

	const int PADDING = 3;
	int X_GRAPH_OFFSET = 50;
	int Y_GRAPH_OFFSET = 50;
};
