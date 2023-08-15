#include "GaussianBlurFilter.h"

#include <opencv2/imgproc.hpp>

GaussianBlurFilter::GaussianBlurFilter()
{
}

GaussianBlurFilter::~GaussianBlurFilter()
{
}

bool GaussianBlurFilter::apply(cv::Mat& mat)
{
	if (mat.empty())
		return false;

    cv::Mat output;
	cv::Size size(makeOdd(sizeX), makeOdd(sizeY));
    cv::GaussianBlur(mat, output, size, sigmaX, sigmaY);

	mat = output;
	return true;
}

QJsonObject GaussianBlurFilter::save() const
{
	QJsonObject size;
	size["x"] = makeOdd(sizeX);
	size["y"] = makeOdd(sizeY);

	QJsonObject sigma;
	sigma["x"] = sigmaX;
	sigma["y"] = sigmaY;

	QJsonObject parameters;
	parameters["kernel-size"] = size;
	parameters["sigma"] = sigma;

	QJsonObject root;
	root["type"] = type();
	root["parameters"] = parameters;

	return root;
}

void GaussianBlurFilter::load(QJsonObject const& p)
{
	QJsonObject parameters = p["parameters"].toObject();
	QJsonObject size = parameters["kernel-size"].toObject();
	QJsonObject sigma = parameters["sigma"].toObject();

	sizeX = makeOdd(size["x"].toInt(sizeX));
	sizeY = makeOdd(size["y"].toInt(sizeY));

	sigmaX = sigma["x"].toDouble(sigmaX);
	sigmaY = sigma["y"].toDouble(sigmaY);
}
