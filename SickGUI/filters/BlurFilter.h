#pragma once
#include "FilterBase.h"

class BlurFilter : public FilterBase
{
public:
    BlurFilter();
    ~BlurFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "blur-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    uint16_t sizeXMin = 1;
    uint16_t sizeXMax = std::numeric_limits<uint16_t>::max();
    uint16_t sizeX;

    uint16_t sizeYMin = 1;
    uint16_t sizeYMax = std::numeric_limits<uint16_t>::max();
    uint16_t sizeY;
};

