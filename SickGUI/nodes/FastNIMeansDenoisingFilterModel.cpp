#include "FastNIMeansDenoisingFilterModel.h"

#include <qformlayout.h>

FastNIMeansDenoisingFilterModel::FastNIMeansDenoisingFilterModel() :
	_widget(new QWidget()),
	sb_h(new QDoubleSpinBox()),
	_filter(std::make_unique<FastNIMeansDenoisingFilter>())
{
	sb_h->setRange(0.0, 100.0);
	connect(sb_h, &QDoubleSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("H", sb_h);

	_widget->setLayout(form);
	syncFilterParameters();
}

void FastNIMeansDenoisingFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject FastNIMeansDenoisingFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void FastNIMeansDenoisingFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	sb_h->setValue(filterParameters["h"].toDouble(sb_h->value()));
}

void FastNIMeansDenoisingFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["h"] = sb_h->value();

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void FastNIMeansDenoisingFilterModel::applyFilter()
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
