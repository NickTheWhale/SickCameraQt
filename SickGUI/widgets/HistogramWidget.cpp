#include "HistogramWidget.h"
#include <qdebug.h>

#include <qpainter.h>
#include <qmenu.h>
#include <qaction.h>
#include <qevent.h>
#include <qstring.h>
#include <qfiledialog.h>
#include <qpixmap.h>

HistogramWidget::HistogramWidget(QWidget* parent) : QWidget(parent)
{
	auto axis = boost::histogram::axis::regular<>(NUM_BINS, MIN_BIN_VALUE, MAX_BIN_VALUE);
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


#if HISTOGRAM_AUTO_SCALE
	uint16_t maxCount = 0;
	for (auto&& x : boost::histogram::indexed(hs.histogram, boost::histogram::coverage::inner))
	{
		auto val = static_cast<uint16_t>(*x);
		if (val > maxCount)
			maxCount = val;
	}
#else
	uint16_t maxCount = MAX_COUNT;
#endif // HISTOGRAM_AUTO_SCALE


	// graph bounding box
	painter.drawRect(X_GRAPH_OFFSET, PADDING, width() - PADDING - X_GRAPH_OFFSET, height() - PADDING - Y_GRAPH_OFFSET);

	auto graphHeight = height() - PADDING - Y_GRAPH_OFFSET;
	auto graphWidth = width() - PADDING - X_GRAPH_OFFSET;
	auto barHeightScaleFactor = graphHeight / static_cast<double>(maxCount);
	auto barWidth = graphWidth / NUM_BINS;

	// graph bars
	auto barOffset = 0;
	for (auto&& x : boost::histogram::indexed(hs.histogram, boost::histogram::coverage::inner))
	{
		auto rawCount = static_cast<uint16_t>(*x);
		if (rawCount > 0)
			auto _ = 0;
		auto scaledCount = rawCount * barHeightScaleFactor;
		auto barHeight = scaledCount < graphHeight ? scaledCount : graphHeight;
		auto barRect = QRect(QPoint(X_GRAPH_OFFSET + (barOffset * barWidth), PADDING + graphHeight - scaledCount), QSize(barWidth, barHeight));
		painter.fillRect(barRect, Qt::GlobalColor::darkGray);
		++barOffset;
	}

	// x axis labels
	painter.drawText(QPoint(width() / 2 - 20, height() - 5), "distance (10^3)");
	for (auto i = 0; i < NUM_BINS; ++i)
	{
		if (i % 10 == 0)
		{
			auto depth = MIN_BIN_VALUE + (i * (MAX_BIN_VALUE - MIN_BIN_VALUE) / NUM_BINS);
			depth /= 1'000;
			painter.drawText(i * barWidth + X_GRAPH_OFFSET + (barWidth * 10 / 2.0f), height() - 25, QString::number(depth));
		}
	}

	// y axis labels
	painter.save();
	painter.translate(10, height() / 2);
	painter.rotate(-90);
	painter.drawText(0, 0, "counts (10^3)");
	painter.restore();

	for (int i = 0; i <= 10; ++i)
	{
		auto count = maxCount * i / 10;
		auto yPos = graphHeight - (count * barHeightScaleFactor);
		painter.drawText(20, yPos, QString::number(count / 1'000));
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
