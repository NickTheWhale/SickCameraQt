#pragma once
#include <vector>
#include <FilterBase.h>
#include <memory>
#include <qjsonobject.h>

class FilterManager
{
	// 1. parse json and make/update filters
	// 2. apply filter
public:
	void updateFilters(const QJsonObject& json);

	std::vector<std::unique_ptr<FilterBase>> filters();

private:
	std::vector<std::unique_ptr<FilterBase>> _filters;
	QJsonObject previousJson;

	FilterBase* makeFilter(const QString type);
};

