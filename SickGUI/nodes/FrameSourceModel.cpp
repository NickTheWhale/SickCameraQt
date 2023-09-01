/*****************************************************************//**
 * @file   FrameSourceModel.cpp
 * @brief  Retrieves raw (unfiltered) frames from ThreadInterface.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "FrameSourceModel.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtimer.h>

FrameSourceModel::FrameSourceModel() :
    threadInterface(ThreadInterface::instance())
{
}

unsigned int FrameSourceModel::nPorts(QtNodes::PortType const portType) const
{
    unsigned int result = 1;
    switch (portType)
    {
    case QtNodes::PortType::In:
        result = 0;
        break;

    case QtNodes::PortType::Out:
        result = 1;
        break;

    default:
        break;
    }

    return result;
}

std::shared_ptr<QtNodes::NodeData> FrameSourceModel::outData(QtNodes::PortIndex const port)
{
    cv::Mat mat = frameset::toMat(_frame);
    return std::make_shared<MatNodeData>(mat);
}

QWidget* FrameSourceModel::embeddedWidget()
{
    if (!_widget)
        createWidgets();
    return _widget;
}

QJsonObject FrameSourceModel::save() const
{
    QJsonObject root;
    root["model-name"] = name();

    return root;
}

void FrameSourceModel::load(QJsonObject const& p)
{
}

void FrameSourceModel::createWidgets()
{
    _widget = new QWidget();
    auto snapshotButton = new QPushButton("Snapshot");
    connect(snapshotButton, &QPushButton::pressed, this, [=]()
        {
            _frame = threadInterface.peekRawFrameset().depth;
            emit dataUpdated(0);
        });
    auto continuousCheckBox = new QCheckBox("Continuous");
    auto timer = new QTimer(_widget);
    connect(continuousCheckBox, &QCheckBox::stateChanged, this, [=]()
        {
            bool enabled = continuousCheckBox->isChecked();
            snapshotButton->setEnabled(!enabled);
            enabled ? timer->start(100) : timer->stop();
        });
    connect(timer, &QTimer::timeout, this, [=]()
        {
            _frame = threadInterface.peekRawFrameset().depth;
            emit dataUpdated(0);
        });

    auto vbox = new QVBoxLayout();
    vbox->addWidget(continuousCheckBox);
    vbox->addWidget(snapshotButton);

    _widget->setLayout(vbox);
}
