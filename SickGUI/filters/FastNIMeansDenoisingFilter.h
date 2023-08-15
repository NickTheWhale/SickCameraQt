#pragma once
#include "FilterBase.h"

class FastNIMeansDenoisingFilter : public FilterBase
{
public:
    FastNIMeansDenoisingFilter();
    ~FastNIMeansDenoisingFilter() override;

    const QString type() const override { return "fast-non-local-means-denoising-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    float h;
};

