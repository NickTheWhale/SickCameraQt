/*****************************************************************//**
 * @file   StackBlurFilter.h
 * @brief  OpenCV stack blur filter.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include "FilterBase.h"

class StackBlurFilter : public FilterBase
{
public:
	StackBlurFilter();
	~StackBlurFilter() override;

	std::unique_ptr<FilterBase> clone() const override;

	const QString type() const override { return "stack-blur-filter"; }
	bool apply(cv::Mat& mat) override;

	QJsonObject save() const override;
	void load(QJsonObject const& p) override;

private:
	uint16_t sizeX;
	uint16_t sizeY;
};

