/*****************************************************************//**
 * @file   BlurFilter.cpp
 * @brief  OpenCV blur filter.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "BlurFilter.h"

#include <opencv2/imgproc.hpp>

BlurFilter::BlurFilter() :
	sizeX(1),
	sizeY(1)
{
}

BlurFilter::~BlurFilter()
{
}

std::unique_ptr<FilterBase> BlurFilter::clone() const
{
	return std::make_unique<BlurFilter>(*this);
}

bool BlurFilter::apply(cv::Mat& mat)
{
	if (mat.empty())
		return false;

	cv::Mat output;

	cv::blur(mat, output, cv::Size(sizeX, sizeY));
	mat = output;

	return true;
}

QJsonObject BlurFilter::save() const
{
	QJsonObject size;
	size["x"] = sizeX;
	size["y"] = sizeY;

	QJsonObject parameters;
	parameters["kernel-size"] = size;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void BlurFilter::load(QJsonObject const& p)
{
	QJsonObject parameters = p["parameters"].toObject();
	QJsonObject size = parameters["kernel-size"].toObject();

	sizeX = size["x"].toInt(sizeX);
	sizeY = size["y"].toInt(sizeY);

	sizeX = std::clamp(sizeX, sizeXMin, sizeXMax);
	sizeY = std::clamp(sizeY, sizeYMin, sizeYMax);
}
