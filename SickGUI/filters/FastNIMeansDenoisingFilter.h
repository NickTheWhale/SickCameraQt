/*****************************************************************//**
 * @file   FastNIMeansDenoisingFilter.h
 * @brief  OpenCV non local means denoising filter.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include "FilterBase.h"

class FastNIMeansDenoisingFilter : public FilterBase
{
public:
    FastNIMeansDenoisingFilter();
    ~FastNIMeansDenoisingFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "fast-non-local-means-denoising-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    float h;
};

