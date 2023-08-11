#include "FrameSourceModel.h"

#include <FrameNodeData.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtimer.h>

FrameSourceModel::FrameSourceModel() :
    _widget(new QWidget()),
    threadInterface(ThreadInterface::instance())
{
    auto snapshotButton = new QPushButton("Snapshot");
    connect(snapshotButton, &QPushButton::pressed, this, [=]() 
        { 
            _frame = threadInterface.peekGuiFrame().depth;
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
            _frame = threadInterface.peekGuiFrame().depth;
            emit dataUpdated(0);
        });
    
    auto vbox = new QVBoxLayout();
    vbox->addWidget(continuousCheckBox);
    vbox->addWidget(snapshotButton);

    _widget->setLayout(vbox);
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

QtNodes::NodeDataType FrameSourceModel::dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const
{
    return FrameNodeData().type();
}

std::shared_ptr<QtNodes::NodeData> FrameSourceModel::outData(QtNodes::PortIndex const port)
{
    return std::make_shared<FrameNodeData>(_frame);
}

QJsonObject FrameSourceModel::save() const
{
    QJsonObject root;
    root["model-name"] = modelName();

    return root;
}

void FrameSourceModel::load(QJsonObject const& p)
{
}