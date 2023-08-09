#include "PassthroughFilterModel.h"

#include <FrameNodeData.h>

#include <qformlayout.h>
#include <qspinbox.h>

PassthroughFilterModel::PassthroughFilterModel() :
	_widget(new QWidget()),
    lower(0),
    upper(std::numeric_limits<uint16_t>::max())
{
    auto lowerSpinBox = new QSpinBox();
    auto upperSpinBox = new QSpinBox();
    lowerSpinBox->setRange(0, std::numeric_limits<uint16_t>::max());
    upperSpinBox->setRange(0, std::numeric_limits<uint16_t>::max());
    lowerSpinBox->setValue(lower);
    upperSpinBox->setValue(upper);
    connect(lowerSpinBox, &QSpinBox::valueChanged, this, [=]()
        {
            lower = lowerSpinBox->value();
            upper = upperSpinBox->value();
            applyFilter();
        });
    connect(upperSpinBox, &QSpinBox::valueChanged, this, [=]()
        {
            lower = lowerSpinBox->value();
            upper = upperSpinBox->value();
            applyFilter();
        });

    auto form = new QFormLayout();

    form->addRow("Lower", lowerSpinBox);
    form->addRow("Upper", upperSpinBox);

    _widget->setLayout(form);
}

unsigned int PassthroughFilterModel::nPorts(QtNodes::PortType const portType) const
{
    return 1;
}

QtNodes::NodeDataType PassthroughFilterModel::dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const
{
    return FrameNodeData().type();
}

std::shared_ptr<QtNodes::NodeData> PassthroughFilterModel::outData(QtNodes::PortIndex const port)
{
    return _currentNodeData;
}

void PassthroughFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
    _originalNodeData = nodeData;
    _currentNodeData = nodeData;

    applyFilter();
}

void PassthroughFilterModel::applyFilter()
{
    if (_originalNodeData)
    {
        auto d = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData);
        frameset::Frame clipped(d->frame());
        frameset::clip(clipped, lower, upper);
        _currentNodeData = std::make_shared<FrameNodeData>(clipped);
    }

    emit dataUpdated(0);
}

