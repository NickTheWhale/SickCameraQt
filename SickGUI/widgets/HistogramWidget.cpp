#include "HistogramWidget.h"
#include <qdebug.h>

#include <qpainter.h>

HistogramWidget::HistogramWidget(QWidget* parent) : QWidget(parent)
{	
	auto axis = boost::histogram::axis::regular<>(NUM_BINS, MIN_VALUE, MAX_VALUE);
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

    auto barWidth = width() / hs.histogram.axis(0).size();

#if HISTOGRAM_AUTO_SCALE
    auto maxHeight = 0;
    for (auto&& x : boost::histogram::indexed(hs.histogram, boost::histogram::coverage::inner))
    {
        auto val = static_cast<uint16_t>(*x);
        if (val > maxHeight)
            maxHeight = val;
    }
#else
    auto maxHeight = MAX_HEIGHT;
#endif // HISTOGRAM_AUTO_SCALE

    auto scaleFactor = height() / static_cast<double>(maxHeight);

    // Draw bars
    int i = 0;
    for (auto&& x : boost::histogram::indexed(hs.histogram, boost::histogram::coverage::inner)) 
    {
        auto barHeight = static_cast<uint16_t>(*x) * scaleFactor;
        QRect barRect(i * barWidth, height() - barHeight, barWidth, barHeight);
        painter.fillRect(barRect, Qt::GlobalColor::darkGray);
        ++i;
    }
}
