#pragma once
#include "FilterBase.h"

class ResizeFilter : public FilterBase
{
public:
    ResizeFilter();
    ~ResizeFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "resize-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    uint16_t sizeX;
    uint16_t sizeY;
};

