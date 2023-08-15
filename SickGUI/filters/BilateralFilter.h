#pragma once
#include "FilterBase.h"

class BilateralFilter :  public FilterBase
{
public:
    BilateralFilter();
    ~BilateralFilter() override;

    const QString type() const override { return "bilateral-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    uint16_t diameter;
    double sigmaColor;
    double sigmaSpace;
};

