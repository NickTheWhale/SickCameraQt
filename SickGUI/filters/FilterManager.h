#pragma once
#include <vector>
#include <FilterBase.h>
#include <memory>
#include <qjsonarray.h>
#include <opencv2/core.hpp>

#include <qmutex.h>

class FilterManager
{
public:
	void makeFilters(const QJsonArray& json);
	bool applyFilters(cv::Mat& mat);

	std::vector<std::unique_ptr<FilterBase>> filters();

private:
	std::vector<std::unique_ptr<FilterBase>> _filters;
	QJsonArray previousJson;

	FilterBase* makeFilter(const QString type);

	QMutex filterMutex;
};

