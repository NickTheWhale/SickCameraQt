#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SickViewer.h"

class SickViewer : public QMainWindow
{
    Q_OBJECT

public:
    SickViewer(QWidget *parent = nullptr);
    ~SickViewer();

private:
    Ui::SickViewerClass ui;
};
