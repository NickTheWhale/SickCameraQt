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

	sizeX = std::clamp(sizeX, uint16_t(1), std::numeric_limits<uint16_t>::max());
	sizeY = std::clamp(sizeY, uint16_t(1), std::numeric_limits<uint16_t>::max());
}
