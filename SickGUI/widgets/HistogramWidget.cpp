#include "HistogramWidget.h"
#include <qdebug.h>
#include <qstringlist.h>
#include <string>
#include <qbarcategoryaxis.h>
#include <qvalueaxis.h>

HistogramWidget::HistogramWidget(QWidget* parent)
{	
	auto axis = boost::histogram::axis::regular<>(100, 0, 65535);
	hs.histogram = boost::histogram::make_histogram(axis);
	
	barSet = new QBarSet("", this);
	series = new QBarSeries(this);

	this->setAnimationOptions(QChart::AnimationOption::NoAnimation);
}

HistogramWidget::~HistogramWidget()
{
}

void HistogramWidget::updateHistogram(const std::vector<uint16_t>& data)
{
	hs.histogram.reset();
	hs.histogram.fill(data);

	//series->clear();
	//this->removeAllSeries();
	for (auto&& x : indexed(hs.histogram)) 
	{
		const auto value = static_cast<uint16_t>(*x);
		auto isNull = barSet == nullptr;
		barSet->insert(x.index(0), value);
	}

	series->append(barSet);

	this->addSeries(series);
}
