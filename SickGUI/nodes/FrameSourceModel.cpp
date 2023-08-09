#include "FrameSourceModel.h"

#include <FrameNodeData.h>
#include <qlayout.h>
#include <qpushbutton.h>

FrameSourceModel::FrameSourceModel() :
    _widget(new QWidget()),
    _image(new ImageLabel()),
    threadInterface(ThreadInterface::instance())
{
    auto snapshotButton = new QPushButton("Snapshot");
    connect(snapshotButton, &QPushButton::pressed, this, [=]() 
        { 
            _frame = threadInterface.peekGuiFrame().depth;
            _image->setPixmap(QPixmap::fromImage(frameset::toQImage(_frame, frameset::ImageOptions())));
            emit dataUpdated(0);
        });
    
    QVBoxLayout* vbox = new QVBoxLayout();
    
    vbox->addWidget(_image);
    vbox->addWidget(snapshotButton);
    
    _image->setMinimumSize(100, 100);
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

