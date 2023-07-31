#pragma once
#include <qwidget.h>
#include "..\VisionaryFrameset.h"
#include <ImageLabel.h>
#include "..\TinyColormap.hpp"
#include <qspinbox.h>

class FrameCompareWidget :
    public QWidget
{
    Q_OBJECT

public:
    explicit FrameCompareWidget(QWidget* parent = nullptr);
    ~FrameCompareWidget();

private:
    Frameset::frameset_t fs1;
    Frameset::frameset_t fs2;
    ImageLabel* imageLabel1 = nullptr;
    ImageLabel* imageLabel2 = nullptr;
    ImageLabel* imageLabel3 = nullptr;
    QSpinBox* thresholdSpinBox = nullptr;

    tinycolormap::ColormapType colorMap = tinycolormap::ColormapType::Turbo;

    void snapshot1();
    void snapshot2();
    void compare();
};

