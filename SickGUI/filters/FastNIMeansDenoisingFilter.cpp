/*****************************************************************//**
 * @file   FastNIMeansDenoisingFilter.cpp
 * @brief  OpenCV non local means denoising filter.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "FastNIMeansDenoisingFilter.h"

#include <opencv2/photo.hpp>

FastNIMeansDenoisingFilter::FastNIMeansDenoisingFilter() :
	h(0.0)
{
}

FastNIMeansDenoisingFilter::~FastNIMeansDenoisingFilter()
{
}

std::unique_ptr<FilterBase> FastNIMeansDenoisingFilter::clone() const
{
	return std::make_unique<FastNIMeansDenoisingFilter>(*this);
}

bool FastNIMeansDenoisingFilter::apply(cv::Mat& mat)
{
	if (mat.empty())
		return false;

    cv::Mat output;
    std::vector<float> hVals;
    hVals.push_back(h);
    cv::fastNlMeansDenoising(mat, output, hVals, 7, 21, cv::NORM_L1);

    mat = output;

    return true;
}

QJsonObject FastNIMeansDenoisingFilter::save() const
{
	QJsonObject parameters;
	parameters["h"] = h;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void FastNIMeansDenoisingFilter::load(QJsonObject const& p)
{
	QJsonObject parameters = p["parameters"].toObject();
	h = parameters["h"].toDouble(h);
}
