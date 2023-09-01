/*****************************************************************//**
 * @file   ExponentialMovingAverageFilter.cpp
 * @brief  Similar to SimpleMovingAverageFilter, weights older frames in the buffer exponentially lower.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "ExponentialMovingAverageFilter.h"

#include <opencv2/imgproc.hpp>

ExponentialMovingAverageFilter::ExponentialMovingAverageFilter() :
	bufferSize(bufferSizeMin),
	alpha(0.2)
{
}

ExponentialMovingAverageFilter::~ExponentialMovingAverageFilter()
{
}

std::unique_ptr<FilterBase> ExponentialMovingAverageFilter::clone() const
{
	return std::make_unique<ExponentialMovingAverageFilter>(*this);
}

bool ExponentialMovingAverageFilter::apply(cv::Mat& mat)
{
	if (mat.empty())
		return false;

	buffer.push_back(mat.clone());
	while (buffer.size() > bufferSize)
		buffer.pop_front();

	for (const cv::Mat& currMat : buffer)
		if (currMat.type() != mat.type() || currMat.size() != mat.size())
			return false;

	cv::Mat firstMat = buffer.front();
	cv::Size size = firstMat.size();
	cv::Mat accumMat = cv::Mat::zeros(size, CV_64F);
	size_t numMats = buffer.size();

	for (size_t i = 0; i < numMats; ++i)
	{
		cv::Mat mat64F;
		buffer[i].convertTo(mat64F, CV_64F);

		double weight = std::pow(alpha, numMats - 1 - i);
		accumMat += weight * mat64F;
	}

	cv::Mat output;
	accumMat.convertTo(output, CV_16U);

	mat = output;

	return true;
}

QJsonObject ExponentialMovingAverageFilter::save() const
{
	QJsonObject parameters;
	parameters["buffer-size"] = bufferSize;
	parameters["alpha"] = alpha;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void ExponentialMovingAverageFilter::load(QJsonObject const& p)
{
	QJsonObject parameters = p["parameters"].toObject();
	bufferSize = parameters["buffer-size"].toInt(bufferSize);
	alpha = parameters["alpha"].toDouble(alpha);

	bufferSize = std::clamp(bufferSize, bufferSizeMin, bufferSizeMax);
	alpha = std::clamp(alpha, alphaMin, alphaMax);
}
