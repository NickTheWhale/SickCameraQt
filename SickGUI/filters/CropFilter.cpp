/*****************************************************************//**
 * @file   CropFilter.cpp
 * @brief  Simple crop filter. Uses normalized dimensions.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "CropFilter.h"

CropFilter::CropFilter() :
    centerX(0.5),
    centerY(0.5),
    width(1.0),
    height(1.0)
{
}

CropFilter::~CropFilter()
{
}

std::unique_ptr<FilterBase> CropFilter::clone() const
{
    return std::make_unique<CropFilter>(*this);
}

bool CropFilter::apply(cv::Mat& mat)
{
    if (mat.empty())
        return false;

    cv::Mat output;

    // crop start

    int x = static_cast<int>(mat.cols * centerX - width * mat.cols / 2);
    int y = static_cast<int>(mat.rows * centerY - height * mat.rows / 2);
    int cropWidth = static_cast<int>(mat.cols * width);
    int cropHeight = static_cast<int>(mat.rows * height);

    // ensure the crop region is within the image bounds
    x = std::max(x, 0);
    y = std::max(y, 0);
    cropWidth = std::min(cropWidth, mat.cols - x);
    cropHeight = std::min(cropHeight, mat.rows - y);

    // limit smallest roi
    cropWidth = std::max(1, cropWidth);
    cropHeight = std::max(1, cropHeight);

    cv::Rect roi(x, y, cropWidth, cropHeight);
    output = mat(roi);

    // crop end

    mat = output;

    return true;
}

QJsonObject CropFilter::save() const
{
    QJsonObject center;
    center["x"] = centerX;
    center["y"] = centerY;

    QJsonObject size;
    size["x"] = width;
    size["y"] = height;

    QJsonObject parameters;
    parameters["center"] = center;
    parameters["size"] = size;

    QJsonObject root;
    root["type"] = type();
    root["parameters"] = parameters;

    return root;
}

void CropFilter::load(QJsonObject const& p)
{
    QJsonObject parameters = p["parameters"].toObject();
    QJsonObject center = parameters["center"].toObject();
    QJsonObject size = parameters["size"].toObject();

    centerX = normClamp(center["x"].toDouble(centerX));
    centerY = normClamp(center["y"].toDouble(centerY));
    
    width = normClamp(size["x"].toDouble(width));
    height = normClamp(size["y"].toDouble(height));
}

double CropFilter::normClamp(const double n)
{
    return std::clamp(n, 0.0, 1.0);
}
