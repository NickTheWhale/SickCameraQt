/*****************************************************************//**
 * @file   FilterBase.h
 * @brief  Filter base class useful for polymorphism.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <qjsonobject.h>
#include <opencv2/core.hpp>

#include <Serializable.hpp>

class FilterBase : public QtNodes::Serializable
{
public:
	virtual ~FilterBase() {};
	
	/**
	 * @brief Clones internal parameters and returns a std::unique_ptr to the new filter object.
	 * 
	 * @return std::unique_ptr to the clone filter object.
	 */
	virtual std::unique_ptr<FilterBase> clone() const = 0;

	/**
	 * @brief Unique string identifier.
	 * 
	 * @return type
	 */
	virtual const QString type() const = 0;

	/**
	 * @brief Applies filter to the given mat.
	 * 
	 * @param mat to be filtered.
	 * @return True if filtered, false otherwise.
	 */
	virtual bool apply(cv::Mat&) = 0;

	/**
	 * @brief Saves a json representation of the filter object.
	 * 
	 * @return json filter object.
	 */
	virtual QJsonObject save() const override = 0;

	/**
	 * @brief Loads a json representation of the filter object.
	 * 
	 * @param json filter object.
	 */
	virtual void load(QJsonObject const&) override = 0;

protected:
	/**
	 * @brief Makes a number odd.
	 * 
	 * @param n number to make odd.
	 * @return odd number.
	 */
	constexpr uint16_t makeOdd(const uint16_t n) const { return n | 1; }
};

