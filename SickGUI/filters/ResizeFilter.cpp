#include "ResizeFilter.h"

#include <opencv2/imgproc.hpp>

ResizeFilter::ResizeFilter() :
    sizeX(10),
    sizeY(10)
{
}

ResizeFilter::~ResizeFilter()
{
}

bool ResizeFilter::apply(cv::Mat& mat)
{
	if (mat.empty())
		return false;

    cv::Mat output;
    cv::resize(mat, output, cv::Size(sizeX, sizeY), 0.0f, 0.0f, cv::InterpolationFlags::INTER_AREA);

	mat = output;
	return true;
}

QJsonObject ResizeFilter::save() const
{
	QJsonObject size;
	size["x"] = sizeX;
	size["y"] = sizeY;

	QJsonObject parameters;
	parameters["size"] = size;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void ResizeFilter::load(QJsonObject const& p)
{
	QJsonObject parameters = p["parameters"].toObject();
	QJsonObject size = parameters["size"].toObject();

	sizeX = size["x"].toInt(sizeX);
	sizeY = size["y"].toInt(sizeY);

	sizeX = std::clamp(sizeX, uint16_t(1), std::numeric_limits<uint16_t>::max());
	sizeY = std::clamp(sizeY, uint16_t(1), std::numeric_limits<uint16_t>::max());
}
