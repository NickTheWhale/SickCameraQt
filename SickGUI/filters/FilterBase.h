#pragma once
#include <qjsonobject.h>
#include <opencv2/core.hpp>

#include <Serializable.hpp>

class FilterBase : public QtNodes::Serializable
{
public:
	virtual ~FilterBase() {};
	virtual const QString type() const = 0;
	virtual bool apply(cv::Mat&) = 0;

	virtual QJsonObject save() const override = 0;
	virtual void load(QJsonObject const&) override = 0;

protected:
	const uint16_t makeOdd(const uint16_t n) const { return n | 1; }
};

