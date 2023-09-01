/*****************************************************************//**
 * @file   StackBlurFilterModel.cpp
 * @brief  Implements QtNodes::NodeDelegateModel.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "StackBlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qevent.h>

#include <opencv2/imgproc.hpp>

StackBlurFilterModel::StackBlurFilterModel() :
	sizeX(1),
	sizeY(1),
	_filter(std::make_unique<StackBlurFilter>())
{

}

void StackBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* StackBlurFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject StackBlurFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void StackBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	QJsonObject size = filterParameters["kernel-size"].toObject();

	sizeX = makeOdd(size["x"].toInt(sizeX));
	sizeY = makeOdd(size["y"].toInt(sizeY));

	if (!_widget)
		createWidgets();
	sb_sizeX->setValue(sizeX);
	sb_sizeY->setValue(sizeY);
}

void StackBlurFilterModel::syncFilterParameters() const
{
	QJsonObject size;
	size["x"] = sizeX;
	size["y"] = sizeY;

	QJsonObject parameters;
	parameters["kernel-size"] = size;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void StackBlurFilterModel::applyFilter()
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

const int StackBlurFilterModel::makeOdd(const int number) const
{
	return number | 1;
}


void StackBlurFilterModel::createWidgets()
{
	sb_sizeX = new QSpinBox();
	sb_sizeY = new QSpinBox();

	sb_sizeX->setRange(1, 99);
	sb_sizeY->setRange(1, 99);

	sb_sizeX->setSingleStep(2);
	sb_sizeY->setSingleStep(2);

	sb_sizeX->setValue(sizeX);
	sb_sizeY->setValue(sizeY);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=](int value) { sizeX = makeOdd(value); syncFilterParameters(); applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=](int value) { sizeY = makeOdd(value); syncFilterParameters(); applyFilter(); });

	connect(sb_sizeX, &QSpinBox::editingFinished, this, [=]() { sb_sizeX->setValue(makeOdd(sb_sizeX->value())); });
	connect(sb_sizeY, &QSpinBox::editingFinished, this, [=]() { sb_sizeY->setValue(makeOdd(sb_sizeY->value())); });

	auto hboxTop = new QHBoxLayout();
	auto hboxBottom = new QHBoxLayout();
	hboxTop->addWidget(new QLabel("Kernal Size (px)"));
	hboxBottom->addWidget(sb_sizeX);
	hboxBottom->addWidget(new QLabel("x"));
	hboxBottom->addWidget(sb_sizeY);

	auto vbox = new QVBoxLayout();
	vbox->addLayout(hboxTop);
	vbox->addLayout(hboxBottom);

	_widget = new QWidget();
	_widget->setLayout(vbox);
	syncFilterParameters();
}