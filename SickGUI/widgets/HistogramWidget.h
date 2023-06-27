#pragma once

#include <qwidget.h>
#include <vector>
#include <boost/histogram.hpp>
#include <algorithm>

/**
 * @brief The HistogramWidget class represents a widget for displaying histograms.
 *
 * The HistogramWidget class provides a custom widget for displaying histograms.
 * It supports updating the histogram data and customizing the x-axis and y-axis.
 */
class HistogramWidget : public QWidget
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs a HistogramWidget object with the specified parameters.
	 * @param minVal The minimum value for the histogram.
	 * @param maxVal The maximum value for the histogram.
	 * @param bins The number of bins for the histogram.
	 * @param maxCount The maximum count for the histogram.
	 * @param parent The parent widget of the histogram widget (optional).
	 */
	explicit HistogramWidget(size_t minVal, size_t maxVal, size_t bins, size_t maxCount, QWidget* parent = nullptr);

	/**
	 * @brief Destructor for the HistogramWidget class.
	 */
	~HistogramWidget();

	/**
	 * @brief Updates the histogram with the provided data.
	 * @param data The vector of uint16_t data to update the histogram.
	 */
	void updateHistogram(const std::vector<uint16_t>& data);

	/**
	 * @brief Sets the x-axis for the histogram.
	 * @note This method does not respect the minVal, maxVal, or bins set during construction. This
	 * means its possible to create nonsensical axis labels.
	 * @tparam T The data type for the x-axis.
	 * @param start The start value of the x-axis.
	 * @param step The step value for each bin on the x-axis.
	 * @param stop The stop value of the x-axis.
	 * @param label The label for the x-axis (optional).
	 * @param reverse Flag indicating whether to reverse the x-axis (optional).
	 */
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

	/**
	 * @brief Sets the y-axis for the histogram.
	 * @note This method does not respect the maxCount or bins set during construction. This
	 * means its possible to create nonsensical axis labels.
	 * @tparam T The data type for the y-axis.
	 * @param start The start value of the y-axis.
	 * @param step The step value for each bin on the y-axis.
	 * @param stop The stop value of the y-axis.
	 * @param label The label for the y-axis (optional).
	 * @param reverse Flag indicating whether to reverse the y-axis (optional).
	 */
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
	/**
	 * @brief Overridden method that is called when a paint event occurs.
	 * @param event The paint event object.
	 */
	void paintEvent(QPaintEvent* event) override;

	/**
	 * @brief Overridden method that is called when a context menu event occurs.
	 *
	 * Used to allow saving the histogram as an image.
	 * @param event The context menu event object.
	 */
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

	struct HolderOfStaticHistogram
	{
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
