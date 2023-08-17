#include "ThresholdFilterModel.h"

#include <qformlayout.h>
#include <qspinbox.h>

ThresholdFilterModel::ThresholdFilterModel() :
	lower(0),
	upper(std::numeric_limits<uint16_t>::max()),
	_filter(std::make_unique<ThresholdFilter>())
{
	
}

void ThresholdFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* ThresholdFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
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

	lower = filterParameters["lower"].toInt(lower);
	upper = filterParameters["upper"].toInt(upper);

	if (!_widget)
		createWidgets();
	sb_lower->setValue(lower);
	sb_upper->setValue(upper);
}

void ThresholdFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["lower"] = lower;
	parameters["upper"] = upper;

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

void ThresholdFilterModel::createWidgets()
{
	sb_lower = new QSpinBox();
	sb_upper = new QSpinBox();

	sb_lower->setRange(0, std::numeric_limits<uint16_t>::max());
	sb_upper->setRange(0, std::numeric_limits<uint16_t>::max());

	sb_lower->setValue(lower);
	sb_upper->setValue(upper);

	connect(sb_lower, &QSpinBox::valueChanged, this, [=](int value) { lower = value; syncFilterParameters(); applyFilter(); });
	connect(sb_upper, &QSpinBox::valueChanged, this, [=](int value) { upper = value; syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();

	form->addRow("Lower", sb_lower);
	form->addRow("Upper", sb_upper);

	_widget = new QWidget();
	_widget->setLayout(form);
	syncFilterParameters();
}
