#include "ThresholdFilterModel.h"

#include <qformlayout.h>
#include <qspinbox.h>

ThresholdFilterModel::ThresholdFilterModel() :
	_widget(new QWidget()),
	sb_lower(new QSpinBox()),
	sb_upper(new QSpinBox())
{
	sb_lower->setRange(0, std::numeric_limits<uint16_t>::max());
	sb_upper->setRange(0, std::numeric_limits<uint16_t>::max());
	connect(sb_lower, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb_upper, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });

	auto form = new QFormLayout();

	form->addRow("Lower", sb_lower);
	form->addRow("Upper", sb_upper);

	_widget->setLayout(form);
}

void ThresholdFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject ThresholdFilterModel::save() const
{
	QJsonObject range;
	range["lower"] = sb_lower->value();
	range["upper"] = sb_upper->value();

	QJsonObject parameters;
	parameters["range"] = range;

	QJsonObject root;
	root["model-name"] = modelName();
	root["filter-parameters"] = parameters;

	return root;
}

void ThresholdFilterModel::load(QJsonObject const& p)
{
	QJsonObject parameters = p["filter-parameters"].toObject();
	QJsonObject range = parameters["range"].toObject();

	sb_lower->setValue(range["lower"].toInt(sb_lower->value()));
	sb_upper->setValue(range["upper"].toInt(sb_upper->value()));
}

void ThresholdFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		auto d = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData);
		frameset::Frame clipped(d->frame());
		frameset::clip(clipped, sb_lower->value(), sb_upper->value());
		_currentNodeData = std::make_shared<FrameNodeData>(clipped);
	}

	emit dataUpdated(0);
}