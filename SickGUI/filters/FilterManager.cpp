#include "FilterManager.h"

void FilterManager::updateFilters(const QJsonObject& json)
{
	if (json == previousJson)
		return;
	
	previousJson = json;
	_filters.clear();
	for (const auto& filterJson : previousJson)
	{
		auto filter = filterJson.toObject();
		
	}
}

std::vector<std::unique_ptr<FilterBase>> FilterManager::filters()
{
	return std::vector<std::unique_ptr<FilterBase>>();
}

FilterBase* FilterManager::makeFilter(const QString type)
{
	return nullptr;
}
