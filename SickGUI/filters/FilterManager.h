/*****************************************************************//**
 * @file   FilterManager.h
 * @brief  Helper class used to create filter objects from their json representation. Used by the captureThread to 
 * create unique filter objects from the filterNodeEditor.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

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
	/**
	 * @brief Creates new filter objects from json representation in the given json array.
	 * 
	 * @param json Json array containing json representation of filters.
	 */
	void makeFilters(const QJsonArray& json);

	/**
	 * @brief Iterates through internal filter array and applys each filter to the 
	 * given mat.
	 * 
	 * @param mat Mat to be filtered.
	 * @return True if all filters were applied, false if any failed
	 */
	bool applyFilters(cv::Mat& mat);

	/**
	 * @brief Returns copy of internal filter buffer.
	 * 
	 * @return Copy of internal filter buffer.
	 */
	std::vector<std::unique_ptr<FilterBase>> filters();

	/**
	 * @brief Returns cached json from previous makeFilters call.
	 * 
	 * @return cached filter json.
	 */
	const QJsonArray filtersJson();

private:
	std::vector<std::unique_ptr<FilterBase>> _filters;
	QJsonArray previousJson;

	/**
	 * @brief Creates raw pointer to a filter of given type.
	 * 
	 * @param type Filter type string.
	 * @return Filter object or nullptr if 'type' does not exist.
	 */
	FilterBase* makeFilter(const QString type);

	QMutex filterMutex;
};

