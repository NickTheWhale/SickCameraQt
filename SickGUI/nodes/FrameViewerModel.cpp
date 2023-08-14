#include "FrameViewerModel.h"

#include <FrameNodeData.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qcombobox.h>

FrameViewerModel::FrameViewerModel() :
    _widget(new QWidget()),
    _image(new ImageLabel()),
    _colorBox(new QComboBox())
{
    for (const auto& action : colorActions)
        _colorBox->addItem(action.name, action.colormapType);

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(_image);
    vbox->addWidget(_colorBox);
    
    _image->setMinimumSize(100, 100);
    _widget->setLayout(vbox);
}

QtNodes::NodeDataType FrameViewerModel::dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const
{
    return FrameNodeData().type();
}

void FrameViewerModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
    _currentNodeData = nodeData;

    if (_currentNodeData)
    {
        const auto d = std::dynamic_pointer_cast<FrameNodeData>(_currentNodeData);
        if (d && !frameset::isEmpty(d->frame()))
        {
            const frameset::Frame frame = std::dynamic_pointer_cast<FrameNodeData>(_currentNodeData)->frame();
            auto options = frameset::ImageOptions();
            options.colormap = colorActions[_colorBox->currentIndex()].colormapType;
            _image->setPixmap(QPixmap::fromImage(frameset::toQImage(frame, options)));
        }
    }
    else
    {
        _image->setPixmap(QPixmap());
    }

    emit dataUpdated(0);
}

QJsonObject FrameViewerModel::save() const
{

    QJsonObject root;
    root["model-name"] = name();
    root["colormap"] = _colorBox->currentIndex();
    root["filterable"] = true;

    return root;
}

void FrameViewerModel::load(QJsonObject const& p)
{
    const int index = p["colormap"].toInt(_colorBox->currentIndex());
    _colorBox->setCurrentIndex(std::clamp(index, 0, _colorBox->count()));
}