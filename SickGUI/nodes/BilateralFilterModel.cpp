#include "BilateralFilterModel.h"

#include <opencv2/imgproc.hpp>

#include <qformlayout.h>

BilateralFilterModel::BilateralFilterModel() :
	diameter(1),
	sigmaColor(0.0),
	sigmaSpace(0.0),
	_filter(std::make_unique<BilateralFilter>())
{
}

void BilateralFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* BilateralFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject BilateralFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void BilateralFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();

	diameter = filterParameters["diameter"].toInt(diameter);
	sigmaColor = filterParameters["sigma-color"].toDouble(sigmaColor);
	sigmaSpace = filterParameters["sigma-space"].toDouble(sigmaSpace);
}

void BilateralFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["diameter"] = diameter;
	parameters["sigma-color"] = sigmaColor;
	parameters["sigma-space"] = sigmaSpace;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void BilateralFilterModel::applyFilter()
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

void BilateralFilterModel::createWidgets()
{
	sb_diameter = new QSpinBox();
	sb_sigmaColor = new QDoubleSpinBox();
	sb_sigmaSpace = new QDoubleSpinBox();

	sb_diameter->setRange(1, 10);
	sb_sigmaColor->setRange(0.0, 200.0);
	sb_sigmaSpace->setRange(0.0, 200.0);

	sb_diameter->setValue(diameter);
	sb_sigmaColor->setValue(sigmaColor);
	sb_sigmaSpace->setValue(sigmaSpace);

	connect(sb_diameter, &QSpinBox::valueChanged, this, [=](int value) { diameter = value; syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaColor, &QDoubleSpinBox::valueChanged, this, [=](double value) { sigmaColor = value; syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaSpace, &QDoubleSpinBox::valueChanged, this, [=](double value) { sigmaSpace = value; syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("Diameter", sb_diameter);
	form->addRow("Color Sigma", sb_sigmaColor);
	form->addRow("Space Sigma", sb_sigmaSpace);

	_widget = new QWidget();
	_widget->setLayout(form);
	syncFilterParameters();
}
