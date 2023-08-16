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

