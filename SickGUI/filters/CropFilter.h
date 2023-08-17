#pragma once
#include "FilterBase.h"

class CropFilter : public FilterBase
{
public: 
	CropFilter();
	~CropFilter() override;

	std::unique_ptr<FilterBase> clone() const override;

	const QString type() const override { return "crop-filter"; }
	bool apply(cv::Mat& mat) override;

	QJsonObject save() const override;
	void load(QJsonObject const& p) override;

private:
	double centerX;
	double centerY;

	double width;
	double height;

	double normClamp(const double n);
};

