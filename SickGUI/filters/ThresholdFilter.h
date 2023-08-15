#pragma once
#include "FilterBase.h"

class ThresholdFilter : public FilterBase
{
public:
    ThresholdFilter();
    ~ThresholdFilter() override;

    const QString type() const override { return "threshold-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    uint16_t upper;
    uint16_t lower;
};

