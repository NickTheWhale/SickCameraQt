#pragma once
#include "FilterBase.h"
#include <deque>

class ExponentialMovingAverageFilter : public FilterBase
{
public:
    ExponentialMovingAverageFilter();
    ~ExponentialMovingAverageFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "exponential-moving-average-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    const uint16_t bufferSizeMin = 2;
    const uint16_t bufferSizeMax = 10;
    uint16_t bufferSize;

    const double alphaMin = 0.0;
    const double alphaMax = 1.0;
    double alpha;

    std::deque<cv::Mat> buffer;
};

