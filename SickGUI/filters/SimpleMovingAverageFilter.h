/*****************************************************************//**
 * @file   SimpleMovingAverageFilter.h
 * @brief  Moving average filter. Maintains internal buffer of size [2, 10]. When apply() is called, the mat is added to the 
 * buffer and then all mats within the buffer are averaged.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include "FilterBase.h"
#include <deque>

class SimpleMovingAverageFilter : public FilterBase
{
public:
    SimpleMovingAverageFilter();
    ~SimpleMovingAverageFilter() override;

    std::unique_ptr<FilterBase> clone() const override;

    const QString type() const override { return "simple-moving-average-filter"; }
    bool apply(cv::Mat& mat) override;

    QJsonObject save() const override;
    void load(QJsonObject const& p) override;

private:
    const uint16_t minBufferSize = 2;
    const uint16_t maxBufferSize = 10;
    uint16_t bufferSize;

    std::deque<cv::Mat> buffer;
};

