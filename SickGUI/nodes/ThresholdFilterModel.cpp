#include "ThresholdFilterModel.h"

#include <qformlayout.h>
#include <qspinbox.h>

ThresholdFilterModel::ThresholdFilterModel() :
	_widget(new QWidget()),
	sb_lower(new QSpinBox()),
	sb_upper(new QSpinBox()),
	_filter(std::make_unique<ThresholdFilter>())
{
	sb_lower->setRange(0, std::numeric_limits<uint16_t>::max());
	sb_upper->setRange(0, std::numeric_limits<uint16_t>::max());
	sb_upper->setValue(sb_upper->maximum());
	connect(sb_lower, &QSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(sb_upper, &QSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();

	form->addRow("Lower", sb_lower);
	form->addRow("Upper", sb_upper);

	_widget->setLayout(form);
	syncFilterParameters();
}

void ThresholdFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject ThresholdFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void ThresholdFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();

	sb_lower->setValue(filterParameters["lower"].toInt(sb_lower->value()));
	sb_upper->setValue(filterParameters["upper"].toInt(sb_upper->value()));
}

void ThresholdFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["lower"] = sb_lower->value();
	parameters["upper"] = sb_upper->value();

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void ThresholdFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		auto d = std::dynamic_pointer_cast<MatNodeData>(_originalNodeData);
		if (d && !d->mat().empty())
		{
			cv::Mat mat = d->mat();
			_filter->apply(mat);

			_currentNodeData = std::make_shared<MatNodeData>(mat);
		}
	}
	emit dataUpdated(0);
}