#include "FrameViewerModel.h"

#include <FrameNodeData.h>
#include <qlayout.h>
#include <qpushbutton.h>

FrameViewerModel::FrameViewerModel() :
    _widget(new QWidget()),
    _image(new ImageLabel()),
    threadInterface(ThreadInterface::instance())
{
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(_image);
    
    _image->setMinimumSize(100, 100);
    _widget->setLayout(vbox);
}

unsigned int FrameViewerModel::nPorts(QtNodes::PortType const portType) const
{
    unsigned int result = 1;
    switch (portType)
    {
    case QtNodes::PortType::In:
        result = 1;
        break;

    case QtNodes::PortType::Out:
        result = 0;
        break;

    default:
        break;
    }

    return result;
}

QtNodes::NodeDataType FrameViewerModel::dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const
{
    return FrameNodeData().type();
}

std::shared_ptr<QtNodes::NodeData> FrameViewerModel::outData(QtNodes::PortIndex const port)
{
    return _currentNodeData;
}

void FrameViewerModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
    _currentNodeData = nodeData;

    if (_currentNodeData)
    {
        auto d = std::dynamic_pointer_cast<FrameNodeData>(_currentNodeData);

        _image->setPixmap(QPixmap::fromImage(frameset::toQImage(d->frame(), frameset::ImageOptions())));
    }

    emit dataUpdated(0);
}
