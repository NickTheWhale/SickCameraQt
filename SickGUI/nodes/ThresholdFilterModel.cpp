#include "ThresholdFilterModel.h"

#include <qformlayout.h>
#include <qspinbox.h>

ThresholdFilterModel::ThresholdFilterModel() :
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

void ThresholdFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
    _originalNodeData = nodeData;
    _currentNodeData = nodeData;

    applyFilter();
}

void ThresholdFilterModel::applyFilter()
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