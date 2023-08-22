#include "FilterManager.h"

#include <BilateralFilter.h>
#include <BlurFilter.h>
#include <FastNIMeansDenoisingFilter.h>
#include <GaussianBlurFilter.h>
#include <MedianBlurFilter.h>
#include <ResizeFilter.h>
#include <StackBlurFilter.h>
#include <ThresholdFilter.h>
#include <CropFilter.h>
#include <SimpleMovingAverageFilter.h>
#include <ExponentialMovingAverageFilterModel.h>

void FilterManager::makeFilters(const QJsonArray& json)
{
	QMutexLocker locker(&filterMutex);

	if (json == previousJson)
		return;

	previousJson = json;
	_filters.clear();
	for (const auto& filterRef : previousJson)
	{
		QJsonObject filterJson = filterRef.toObject();
		QString type = filterJson["type"].toString();
		FilterBase* filter = makeFilter(type);
		if (filter)
		{
			filter->load(filterJson);
			_filters.push_back(filter->clone());
			delete filter;
			filter = nullptr;
		}
	}
}

bool FilterManager::applyFilters(cv::Mat& mat)
{
	QMutexLocker locker(&filterMutex);

	for (auto& filter : _filters)
	{
		auto type = filter->type();
		if (filter.get() && !filter->apply(mat))
			return false;
	}
	return true;
}

std::vector<std::unique_ptr<FilterBase>> FilterManager::filters()
{
	QMutexLocker locker(&filterMutex);

	std::vector<std::unique_ptr<FilterBase>> newFilters;

	for (const auto& filter : _filters)
		newFilters.push_back(filter->clone());

	return newFilters;
}

const QJsonArray FilterManager::filtersJson()
{
	QMutexLocker locker(&filterMutex);
	return previousJson;
}

FilterBase* FilterManager::makeFilter(const QString type)
{
	if (type == BilateralFilter().type())
		return new BilateralFilter();

	if (type == BlurFilter().type())
		return new BlurFilter();

	if (type == FastNIMeansDenoisingFilter().type())
		return new FastNIMeansDenoisingFilter();

	if (type == GaussianBlurFilter().type())
		return new GaussianBlurFilter();

	if (type == MedianBlurFilter().type())
		return new MedianBlurFilter();

	if (type == ResizeFilter().type())
		return new ResizeFilter();

	if (type == StackBlurFilter().type())
		return new StackBlurFilter();

	if (type == ThresholdFilter().type())
		return new ThresholdFilter();

	if (type == CropFilter().type())
		return new CropFilter();

	if (type == SimpleMovingAverageFilter().type())
		return new SimpleMovingAverageFilter();

	if (type == ExponentialMovingAverageFilter().type())
		return new ExponentialMovingAverageFilter();

	return nullptr;
}
