#include "FrameViewerModel.h"

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

void FrameViewerModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
    _currentNodeData = nodeData;

    if (_currentNodeData)
    {
        const auto d = std::dynamic_pointer_cast<MatNodeData>(_currentNodeData);
        if (d && !d->mat().empty())
        {
            const frameset::Frame frame = frameset::toFrame(d->mat());
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

    return root;
}

void FrameViewerModel::load(QJsonObject const& p)
{
    const int index = p["colormap"].toInt(_colorBox->currentIndex());
    _colorBox->setCurrentIndex(std::clamp(index, 0, _colorBox->count()));
}