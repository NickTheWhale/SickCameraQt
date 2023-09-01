/*****************************************************************//**
 * @file   CropFilterModel.cpp
 * @brief  Implements QtNodes::NodeDelegateModel.
 * 
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "CropFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>

#include <opencv2/imgproc.hpp>

CropFilterModel::CropFilterModel() :
	centerX(0.5),
	centerY(0.5),
	width(1.0),
	height(1.0),
	_filter(std::make_unique<CropFilter>())
{
}

void CropFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* CropFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject CropFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void CropFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	QJsonObject center = filterParameters["center"].toObject();
	QJsonObject size = filterParameters["size"].toObject();

	centerX = center["x"].toDouble(centerX);
	centerY = center["y"].toDouble(centerY);

	width = size["x"].toDouble(width);
	height = size["y"].toDouble(height);

	if (!_widget)
		createWidgets();

	sb_centerX->setValue(centerX);
	sb_centerY->setValue(centerY);

	sb_width->setValue(width);
	sb_height->setValue(height);
}

void CropFilterModel::syncFilterParameters() const
{
	QJsonObject center;
	center["x"] = centerX;
	center["y"] = centerY;

	QJsonObject size;
	size["x"] = width;
	size["y"] = height;

	QJsonObject parameters;
	parameters["center"] = center;
	parameters["size"] = size;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void CropFilterModel::applyFilter()
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

void CropFilterModel::createWidgets()
{
	sb_centerX = new QDoubleSpinBox();
	sb_centerY = new QDoubleSpinBox();

	sb_width = new QDoubleSpinBox();
	sb_height = new QDoubleSpinBox();

	sb_centerX->setRange(0.0, 1.0);
	sb_centerY->setRange(0.0, 1.0);

	sb_width->setRange(0.0, 1.0);
	sb_height->setRange(0.0, 1.0);

	sb_centerX->setValue(centerX);
	sb_centerY->setValue(centerY);

	sb_width->setValue(width);
	sb_height->setValue(height);

	connect(sb_centerX, &QDoubleSpinBox::valueChanged, this, [=](double value) { centerX = value; syncFilterParameters(); applyFilter(); });
	connect(sb_centerY, &QDoubleSpinBox::valueChanged, this, [=](double value) { centerY = value; syncFilterParameters(); applyFilter(); });
	connect(sb_width, &QDoubleSpinBox::valueChanged, this, [=](double value) { width = value; syncFilterParameters(); applyFilter(); });
	connect(sb_height, &QDoubleSpinBox::valueChanged, this, [=](double value) { height = value; syncFilterParameters(); applyFilter(); });

	auto hbox_size_lbl = new QHBoxLayout();
	hbox_size_lbl->addWidget(new QLabel("Size (normalized)"));

	auto hbox_size = new QHBoxLayout();
	hbox_size->addWidget(sb_width);
	hbox_size->addWidget(new QLabel("x"));
	hbox_size->addWidget(sb_height);

	auto hbox_center_lbl = new QHBoxLayout();
	hbox_center_lbl->addWidget(new QLabel("Center (normalized)"));

	auto hbox_center = new QHBoxLayout();
	hbox_center->addWidget(sb_centerX);
	hbox_center->addWidget(new QLabel("x"));
	hbox_center->addWidget(sb_centerY);

	auto vbox = new QVBoxLayout();
	vbox->addLayout(hbox_size_lbl);
	vbox->addLayout(hbox_size);
	vbox->addLayout(hbox_center_lbl);
	vbox->addLayout(hbox_center);

	_widget = new QWidget();
	_widget->setLayout(vbox);
	syncFilterParameters();
}
