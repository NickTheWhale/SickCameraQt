#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SickGUI.h"
#include <qstack.h>
#include <qmutex.h>
#include <chrono>

#include <boost/circular_buffer.hpp>

#include "CaptureThread.h"
#include "Image.h"

class CaptureThread;

struct image_t;

class Camera;

class SickGUI : public QMainWindow
{
    Q_OBJECT


public slots:
    void startVideo();
    void stopVideo();
    void newImage(Frame::frame_t image);
    void testButtonClick();

public:
    SickGUI(QWidget *parent = nullptr);
    ~SickGUI();

private:
    void timerEvent(QTimerEvent* event);
    void showImage(QImage image);
    bool createCamera();

    const std::string IP_ADDRESS = "169.254.49.161";

    Ui::SickGUIClass ui;

    const size_t frameBufferSize = 10;
    boost::circular_buffer<Frame::frame_t> frameBuffer;
    QMutex frameMutex;

    Camera *camera = nullptr;

    CaptureThread *captureThread = nullptr;

    int refreshDisplayTimer;
    int displayInterval = 500; /*milliseconds*/

    int plcTimer;
    const int plcInterval = 1000; /*milliseconds*/
    unsigned long long loopCount = 0;
};
