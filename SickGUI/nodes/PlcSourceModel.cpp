#include "PlcSourceModel.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtimer.h>

PlcSourceModel::PlcSourceModel() : 
    threadInterface(ThreadInterface::instance())
{

}

unsigned int PlcSourceModel::nPorts(QtNodes::PortType const portType) const
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

std::shared_ptr<QtNodes::NodeData> PlcSourceModel::outData(QtNodes::PortIndex const port)
{
    cv::Mat mat = frameset::toMat(_frame);
    return std::make_shared<MatNodeData>(mat);
}

QJsonObject PlcSourceModel::save() const
{
    QJsonObject root;
    root["model-name"] = name();

    return root;
}

QWidget* PlcSourceModel::embeddedWidget()
{
    if (!_widget)
        createWidgets();
    return _widget;
}

void PlcSourceModel::load(QJsonObject const& p)
{
}

void PlcSourceModel::createWidgets()
{
    auto snapshotButton = new QPushButton("Snapshot");
    connect(snapshotButton, &QPushButton::pressed, this, [=]()
        {
            _frame = threadInterface.peekFilteredFrame().depth;
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
            _frame = threadInterface.peekFilteredFrame().depth;
            emit dataUpdated(0);
        });

    auto vbox = new QVBoxLayout();
    vbox->addWidget(continuousCheckBox);
    vbox->addWidget(snapshotButton);

    _widget = new QWidget();
    _widget->setLayout(vbox);
}