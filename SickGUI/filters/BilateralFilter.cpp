#include "BilateralFilter.h"

#include <opencv2/imgproc.hpp>

BilateralFilter::BilateralFilter() :
    diameter(1),
    sigmaColor(0),
    sigmaSpace(0)
{
}

BilateralFilter::~BilateralFilter()
{
}

std::unique_ptr<FilterBase> BilateralFilter::clone() const
{
    return std::make_unique<BilateralFilter>(*this);
}

bool BilateralFilter::apply(cv::Mat& mat)
{
    if (mat.empty())
        return false;

    cv::Mat input32F;
    cv::Mat output32F;
    cv::Mat output;
    mat.convertTo(input32F, CV_32F);
    cv::bilateralFilter(input32F, output32F, diameter, sigmaColor, sigmaSpace);
    output32F.convertTo(output, CV_16U);
    mat = output;

    return true;
}

QJsonObject BilateralFilter::save() const
{
	QJsonObject parameters;
    parameters["diameter"] = diameter;
    parameters["sigma-color"] = sigmaColor;
    parameters["sigma-space"] = sigmaSpace;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void BilateralFilter::load(QJsonObject const& p)
{
    QJsonObject parameters = p["parameters"].toObject();
    diameter = parameters["diameter"].toInt(diameter);
    sigmaColor = parameters["sigma-color"].toDouble(sigmaColor);
    sigmaSpace =  parameters["sigma-space"].toDouble(sigmaSpace);

    diameter = std::clamp(diameter, diameterMin, diameterMax);
    sigmaColor = std::clamp(sigmaColor, sigmaColorMin, sigmaColorMax);
    sigmaSpace = std::clamp(sigmaSpace, sigmaSpaceMin, sigmaSpaceMax);
}
