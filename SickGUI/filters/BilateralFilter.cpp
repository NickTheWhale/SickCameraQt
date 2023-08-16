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

    cv::Mat output;
    mat.convertTo(mat, CV_8U, 255.0 / 65535.0);
    cv::normalize(mat, mat, 0, 255, cv::NormTypes::NORM_MINMAX);
    cv::bilateralFilter(mat, output, diameter, sigmaColor, sigmaSpace);
    output.convertTo(output, CV_16U, 65535.0 / 255.0);

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

    diameter = std::clamp(diameter, uint16_t(1), std::numeric_limits<uint16_t>::max());
}
