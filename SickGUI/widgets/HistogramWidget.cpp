#include "HistogramWidget.h"
#include <qdebug.h>

#include <qpainter.h>
#include <qmenu.h>
#include <qaction.h>
#include <qevent.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qpixmap.h>

HistogramWidget::HistogramWidget(size_t minVal, size_t maxVal, size_t numBins, size_t maxCount, QWidget* parent)
	: QWidget(parent), minVal(minVal), maxVal(maxVal), numBins(numBins), maxCount(maxCount)
{
	auto axis = boost::histogram::axis::regular<>(numBins, minVal, maxVal);
	hs.histogram = boost::histogram::make_histogram(axis);
}

HistogramWidget::~HistogramWidget()
{
}

void HistogramWidget::updateHistogram(const std::vector<uint16_t>& data)
{
	hs.histogram.reset();

	std::vector<uint16_t> dataCopy = data;

	dataCopy.erase(std::remove(dataCopy.begin(), dataCopy.end(), 0), dataCopy.end());

	hs.histogram.fill(dataCopy);
}

void HistogramWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	setGraphXOffsetForXAxis(showXAxis);
	setGraphYOffsetForYAxis(showYAxis);

	auto graphHeight = height() - PADDING - Y_GRAPH_OFFSET;
	auto graphWidth = width() - PADDING - X_GRAPH_OFFSET;
	auto barHeightScaleFactor = graphHeight / static_cast<double>(maxCount);
	auto barWidth = graphWidth / static_cast<double>(numBins);

	// x axis labels
	if (showXAxis)
	{
		painter.drawText(QPoint(width() / 2 - 20, height() - 5), xAxisLabel.c_str());


		for (auto i = 0; i < xAxis.size(); ++i)
		{
			auto xPos = i * (graphWidth / static_cast<double>(xAxis.size())) + X_GRAPH_OFFSET;
			auto yPos = height() - 25;
			painter.drawText(xPos, yPos, QString::number(xAxis[i]));
		}

	}

	// y axis labels
	if (showYAxis)
	{
		painter.save();
		painter.translate(10, height() / 2);
		painter.rotate(-90);
		painter.drawText(0, 0, yAxisLabel.c_str());
		painter.restore();

		for (auto i = 0; i < yAxis.size(); ++i)
		{
			auto xPos = 19;
			auto yPos = i * (graphHeight / static_cast<double>(yAxis.size())) + 12;
			painter.drawText(xPos, yPos, QString::number(yAxis[yAxis.size() - i - 1]));
		}

	}


	// graph bounding box
	painter.drawRect(X_GRAPH_OFFSET, PADDING, width() - PADDING - X_GRAPH_OFFSET, height() - PADDING - Y_GRAPH_OFFSET);

	// graph bars
	auto barOffset = 0;
	for (auto&& x : boost::histogram::indexed(hs.histogram, boost::histogram::coverage::inner))
	{
		auto rawCount = static_cast<uint16_t>(*x);
		if (rawCount > 0)
			auto _ = 0;
		auto scaledCount = rawCount * barHeightScaleFactor;
		auto barHeight = scaledCount < graphHeight ? scaledCount : graphHeight;
		auto xPos = X_GRAPH_OFFSET + (barOffset * barWidth);
		auto yPos = PADDING + graphHeight - scaledCount;
		auto barRect = QRect(QPoint(xPos, yPos), QSize(barWidth * 0.75, barHeight));
		painter.fillRect(barRect, Qt::GlobalColor::darkGray);
		++barOffset;
	}
}

void HistogramWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QPainter painter(this);
	painter.end();

	QMenu menu(this);
	QAction* saveAction = menu.addAction("Save as Image");
	QAction* selectedAction = menu.exec(event->globalPos());
	if (selectedAction == saveAction)
	{
		QString filePath = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG Image (*.png)");

		if (!filePath.isEmpty())
		{
			QPixmap pixmap(this->size());
			this->render(&pixmap);
			pixmap.save(filePath);
		}
	}
}

void HistogramWidget::setGraphXOffsetForXAxis(bool forLabel)
{
	if (forLabel)
		Y_GRAPH_OFFSET = 50;
	else
		Y_GRAPH_OFFSET = PADDING;
}

void HistogramWidget::setGraphYOffsetForYAxis(bool forLabel)
{
	if (forLabel)
		X_GRAPH_OFFSET = 53;
	else
		X_GRAPH_OFFSET = PADDING;
}