/*****************************************************************//**
 * @file   ExponentialMovingAverageFilterModel.cpp
 * @brief  Implements QtNodes::NodeDelegateModel.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "ExponentialMovingAverageFilterModel.h"

#include <opencv2/imgproc.hpp>

#include <qformlayout.h>

ExponentialMovingAverageFilterModel::ExponentialMovingAverageFilterModel() :
	alpha(0.2),
	bufferSize(2),
	_filter(std::make_unique<ExponentialMovingAverageFilter>())
{
}

void ExponentialMovingAverageFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* ExponentialMovingAverageFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject ExponentialMovingAverageFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void ExponentialMovingAverageFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();

	bufferSize = filterParameters["buffer-size"].toInt(bufferSize);
	alpha = filterParameters["alpha"].toDouble(alpha);

	if (!_widget)
		createWidgets();
	sb_bufferSize->setValue(bufferSize);
	sb_alpha->setValue(alpha);
}

void ExponentialMovingAverageFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["buffer-size"] = bufferSize;
	parameters["alpha"] = alpha;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void ExponentialMovingAverageFilterModel::applyFilter()
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

void ExponentialMovingAverageFilterModel::createWidgets()
{
	sb_bufferSize = new QSpinBox();
	sb_alpha = new QDoubleSpinBox();

	sb_bufferSize->setRange(2, 10);
	sb_bufferSize->setValue(bufferSize);

	sb_alpha->setRange(0.0, 1.0);
	sb_alpha->setValue(alpha);

	connect(sb_bufferSize, &QSpinBox::valueChanged, this, [=](int value) { bufferSize = value; syncFilterParameters(); applyFilter(); });
	connect(sb_alpha, &QDoubleSpinBox::valueChanged, this, [=](double value) { alpha = value; syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("Buffer Size", sb_bufferSize);
	form->addRow("Alpha", sb_alpha);

	_widget = new QWidget();
	_widget->setLayout(form);
	syncFilterParameters();
}
