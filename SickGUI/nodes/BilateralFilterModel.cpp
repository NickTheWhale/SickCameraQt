#include "BilateralFilterModel.h"

#include <opencv2/imgproc.hpp>

#include <qformlayout.h>

BilateralFilterModel::BilateralFilterModel() :
	_widget(new QWidget()),
	sb_diameter(new QSpinBox()),
	sb_sigmaColor(new QDoubleSpinBox()),
	sb_sigmaSpace(new QDoubleSpinBox()),
	_filter(std::make_unique<BilateralFilter>())
{
	sb_diameter->setRange(1, 10);
	sb_sigmaColor->setRange(0.0, 200.0);
	sb_sigmaSpace->setRange(0.0, 200.0);

	connect(sb_diameter, &QSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaColor, &QDoubleSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaSpace, &QDoubleSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("Diameter", sb_diameter);
	form->addRow("Color Sigma", sb_sigmaColor);
	form->addRow("Space Sigma", sb_sigmaSpace);

	_widget->setLayout(form);
	syncFilterParameters();
}

void BilateralFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
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
	sb_diameter->setValue(filterParameters["diameter"].toInt(sb_diameter->value()));
	sb_sigmaColor->setValue(filterParameters["sigma-color"].toDouble(sb_sigmaColor->value()));
	sb_sigmaSpace->setValue(filterParameters["sigma-space"].toDouble(sb_sigmaSpace->value()));
}

void BilateralFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["diameter"] = sb_diameter->value();
	parameters["sigma-color"] = sb_sigmaColor->value();
	parameters["sigma-space"] = sb_sigmaSpace->value();

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
