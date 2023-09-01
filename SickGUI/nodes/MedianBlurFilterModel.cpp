/*****************************************************************//**
 * @file   MedianBlurFilterModel.cpp
 * @brief  Implements QtNodes::NodeDelegateModel.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "MedianBlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>

#include <opencv2/imgproc.hpp>

MedianBlurFilterModel::MedianBlurFilterModel() :
	kernelSize(SMALL),
	_filter(std::make_unique<MedianBlurFilter>())
{

}

void MedianBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* MedianBlurFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject MedianBlurFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void MedianBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	const int size = filterParameters["kernel-size"].toInt(SMALL);
	if (size == LARGE)
		kernelSize = LARGE;
	else
		kernelSize = SMALL;

	if (!_widget)
		createWidgets();
	rb_size3->setChecked(kernelSize == SMALL);
	rb_size5->setChecked(kernelSize == LARGE);
}

void MedianBlurFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["kernel-size"] = kernelSize;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void MedianBlurFilterModel::applyFilter()
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

void MedianBlurFilterModel::createWidgets()
{
	rb_size3 = new QRadioButton("3");
	rb_size5 = new QRadioButton("5");

	rb_size3->setChecked(kernelSize == SMALL);
	rb_size5->setChecked(kernelSize == LARGE);

	connect(rb_size3, &QRadioButton::toggled, this, [=]()
		{
			sizeSelected();
			syncFilterParameters();
			applyFilter();
		});
	connect(rb_size5, &QRadioButton::toggled, this, [=]()
		{
			sizeSelected();
			syncFilterParameters();
			applyFilter();
		});

	auto hbox = new QHBoxLayout();
	hbox->addWidget(rb_size3);
	hbox->addWidget(rb_size5);

	auto vbox = new QVBoxLayout();
	vbox->addWidget(new QLabel("Kernal Size (px)"));
	vbox->addLayout(hbox);

	_widget = new QWidget();
	_widget->setLayout(vbox);
	syncFilterParameters();
}

void MedianBlurFilterModel::sizeSelected()
{
	if (rb_size3->isChecked())
		kernelSize = SMALL;
	else if (rb_size5->isChecked())
		kernelSize = LARGE;		
}
