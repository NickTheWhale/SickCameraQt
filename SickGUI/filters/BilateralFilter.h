#pragma once
#include "FilterBase.h"

class BilateralFilter :  public FilterBase
{
public:
    BilateralFilter();
    ~BilateralFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "bilateral-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    const uint16_t diameterMin = 1;
    const uint16_t diameterMax = std::numeric_limits<uint16_t>::max();
    uint16_t diameter;

    const double sigmaColorMin = 0.0;
    const double sigmaColorMax = std::numeric_limits<double>::max();
    double sigmaColor;


    const double sigmaSpaceMin = 0.0;
    const double sigmaSpaceMax = std::numeric_limits<double>::max();
    double sigmaSpace;
};

