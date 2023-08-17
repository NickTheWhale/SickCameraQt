#include "SimpleMovingAverageFilter.h"

#include <opencv2/imgproc.hpp>

SimpleMovingAverageFilter::SimpleMovingAverageFilter() :
	bufferSize(1)
{
}

SimpleMovingAverageFilter::~SimpleMovingAverageFilter()
{
}

std::unique_ptr<FilterBase> SimpleMovingAverageFilter::clone() const
{
	return std::make_unique<SimpleMovingAverageFilter>(*this);
}

bool SimpleMovingAverageFilter::apply(cv::Mat& mat)
{
	if (mat.empty())
		return false;

	// add to buffer
	buffer.push_back(mat.clone());
	while (buffer.size() > bufferSize)
		buffer.pop_front();

	// average
	cv::Mat firstMat = buffer.front();
	cv::Size size = firstMat.size();
	cv::Mat accumMat = cv::Mat::zeros(size, CV_64F);
	size_t numMats = buffer.size();

	for (const cv::Mat& currMat : buffer)
	{
		cv::Mat mat64F;
		currMat.convertTo(mat64F, CV_64F);

		accumMat += mat64F;
	}

	cv::Mat meanMat = accumMat / numMats;

	cv::Mat output;
	meanMat.convertTo(output, CV_16U);

	mat = output;

	return true;
}

QJsonObject SimpleMovingAverageFilter::save() const
{
	QJsonObject parameters;
	parameters["buffer-size"] = bufferSize;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void SimpleMovingAverageFilter::load(QJsonObject const& p)
{
	QJsonObject parameters = p["parameters"].toObject();
	bufferSize = parameters["buffer-size"].toInt(bufferSize);

	bufferSize = std::clamp(bufferSize, minBufferSize, maxBufferSize);
}
