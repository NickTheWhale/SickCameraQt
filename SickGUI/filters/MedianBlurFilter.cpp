/*****************************************************************//**
 * @file   MedianBlurFilter.cpp
 * @brief  OpenCV median blur filter.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "MedianBlurFilter.h"

#include <opencv2/imgproc.hpp>

MedianBlurFilter::MedianBlurFilter() :
    size(3)
{
}

MedianBlurFilter::~MedianBlurFilter()
{
}

std::unique_ptr<FilterBase> MedianBlurFilter::clone() const
{
	return std::make_unique<MedianBlurFilter>(*this);
}


bool MedianBlurFilter::apply(cv::Mat& mat)
{
	if (mat.empty())
		return false;

	if (size != 3 && size != 5)
		size = 3;
    
	cv::Mat output;
    cv::medianBlur(mat, output, size);

	mat = output;

	return true;
}

QJsonObject MedianBlurFilter::save() const
{
	QJsonObject parameters;
	parameters["kernel-size"] = size;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void MedianBlurFilter::load(QJsonObject const& p)
{
	QJsonObject parameters = p["parameters"].toObject();
	size = parameters["kernel-size"].toInt(size);

	if (size <= 3)
		size = 3;
	else
		size = 5;
}
