#include "StackBlurFilter.h"

#include <opencv2/imgproc.hpp>

StackBlurFilter::StackBlurFilter() :
    sizeX(1),
    sizeY(1)
{
}

StackBlurFilter::~StackBlurFilter()
{
}

bool StackBlurFilter::apply(cv::Mat& mat)
{
    if (mat.empty())
        return false;

    cv::Mat output;
    sizeX = makeOdd(sizeX);
    sizeY = makeOdd(sizeY);
    cv::stackBlur(mat, output, cv::Size(sizeX, sizeY));

    mat = output;

    return true;
}

QJsonObject StackBlurFilter::save() const
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

void StackBlurFilter::load(QJsonObject const& p)
{
    auto parameters = p["parameters"].toObject();
    auto x = parameters["kernel-size"].toObject()["x"].toInt(sizeX);
    auto y = parameters["kernel-size"].toObject()["y"].toInt(sizeY);

    sizeX = makeOdd(x);
    sizeY = makeOdd(y);
}