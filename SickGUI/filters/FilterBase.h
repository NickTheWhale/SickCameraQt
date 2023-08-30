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
	virtual std::unique_ptr<FilterBase> clone() const = 0;
	virtual const QString type() const = 0;
	virtual bool apply(cv::Mat&) = 0;

	virtual QJsonObject save() const override = 0;
	virtual void load(QJsonObject const&) override = 0;

protected:
	constexpr uint16_t makeOdd(const uint16_t n) const { return n | 1; }
};

