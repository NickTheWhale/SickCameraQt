#include "ThresholdFilter.h"

#include <opencv2/imgproc.hpp>

ThresholdFilter::ThresholdFilter() :
    lower(std::numeric_limits<uint16_t>::min()),
    upper(std::numeric_limits<uint16_t>::max())
{
}

ThresholdFilter::~ThresholdFilter()
{
}

bool ThresholdFilter::apply(cv::Mat& mat)
{
    if (mat.empty())
        return false;

    cv::Mat output;
    cv::threshold(mat, output, upper, 0, cv::THRESH_TOZERO_INV);
    mat = output;
    
    cv::threshold(mat, output, lower, 0, cv::THRESH_TOZERO);
    mat = output;

    return true;
}

QJsonObject ThresholdFilter::save() const
{
    QJsonObject parameters;
    parameters["lower"] = lower;
    parameters["upper"] = upper;
    
    QJsonObject root;
    root["type"] = type();
    root["parameters"] = parameters;

    return root;
}

void ThresholdFilter::load(QJsonObject const& p)
{
    lower = p["parameters"].toObject()["lower"].toInt(lower);
    upper = p["parameters"].toObject()["upper"].toInt(upper);

    if (upper < lower)
        std::swap(upper, lower);
}
