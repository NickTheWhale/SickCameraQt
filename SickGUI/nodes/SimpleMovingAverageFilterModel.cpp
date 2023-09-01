/*****************************************************************//**
 * @file   SimpleMovingAverageFilterModel.cpp
 * @brief  Implements QtNodes::NodeDelegateModel.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "SimpleMovingAverageFilterModel.h"

#include <opencv2/imgproc.hpp>

#include <qformlayout.h>

SimpleMovingAverageFilterModel::SimpleMovingAverageFilterModel() :
	bufferSize(2),
	_filter(std::make_unique<SimpleMovingAverageFilter>())
{
}

void SimpleMovingAverageFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* SimpleMovingAverageFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject SimpleMovingAverageFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void SimpleMovingAverageFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();

	bufferSize = filterParameters["buffer-size"].toInt(bufferSize);

	if (!_widget)
		createWidgets();
	sb_bufferSize->setValue(bufferSize);
}

void SimpleMovingAverageFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["buffer-size"] = bufferSize;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void SimpleMovingAverageFilterModel::applyFilter()
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

void SimpleMovingAverageFilterModel::createWidgets()
{
	sb_bufferSize = new QSpinBox();

	sb_bufferSize->setRange(2, 10);
	sb_bufferSize->setValue(bufferSize);

	connect(sb_bufferSize, &QSpinBox::valueChanged, this, [=](int value) { bufferSize = value; syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("Buffer Size", sb_bufferSize);

	_widget = new QWidget();
	_widget->setLayout(form);
	syncFilterParameters();
}
