/*****************************************************************//**
 * @file   FastNIMeansDenoisingFilterModel.cpp
 * @brief  Implements QtNodes::NodeDelegateModel.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "FastNIMeansDenoisingFilterModel.h"

#include <qformlayout.h>

FastNIMeansDenoisingFilterModel::FastNIMeansDenoisingFilterModel() :
	hVal(0.0),
	_filter(std::make_unique<FastNIMeansDenoisingFilter>())
{
}

void FastNIMeansDenoisingFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* FastNIMeansDenoisingFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
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
	hVal = filterParameters["h"].toDouble(hVal);

	if (!_widget)
		createWidgets();
	sb_h->setValue(hVal);
}

void FastNIMeansDenoisingFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["h"] = hVal;

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

void FastNIMeansDenoisingFilterModel::createWidgets()
{
	sb_h = new QDoubleSpinBox();

	sb_h->setRange(0.0, 100.0);

	sb_h->setValue(hVal);

	connect(sb_h, &QDoubleSpinBox::valueChanged, this, [=](double value) {hVal = value; syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("H", sb_h);

	_widget = new QWidget();
	_widget->setLayout(form);
	syncFilterParameters();
}
