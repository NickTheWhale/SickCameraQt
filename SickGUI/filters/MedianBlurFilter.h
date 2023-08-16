#pragma once
#include "FilterBase.h"

class MedianBlurFilter : public FilterBase
{
public:
    MedianBlurFilter();
    ~MedianBlurFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "median-blur-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    uint16_t size;
};

