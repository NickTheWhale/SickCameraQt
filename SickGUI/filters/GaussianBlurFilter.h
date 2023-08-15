#pragma once
#include "FilterBase.h"

class GaussianBlurFilter : public FilterBase
{
public:
    GaussianBlurFilter();
    ~GaussianBlurFilter() override;

    const QString type() const override { return "gaussian-blur-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    uint16_t sizeX;
    uint16_t sizeY;

    double sigmaX;
    double sigmaY;
};

