#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DockingTest.h"

class DockingTest : public QMainWindow
{
    Q_OBJECT

public:
    DockingTest(QWidget *parent = nullptr);
    ~DockingTest();

private:
    Ui::DockingTestClass ui;
};
