/*****************************************************************//**
 * @file   ThresholdFilter.h
 * @brief  OpenCV Threshold filter.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include "FilterBase.h"

class ThresholdFilter : public FilterBase
{
public:
    ThresholdFilter();
    ~ThresholdFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "threshold-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    uint16_t upper;
    uint16_t lower;
};

