#include "BilateralFilterModel.h"

#include <opencv2/imgproc.hpp>

#include <qformlayout.h>

BilateralFilterModel::BilateralFilterModel() :
	_widget(new QWidget()),
	sb_diameter(new QSpinBox()),
	sb_sigmaColor(new QDoubleSpinBox()),
	sb_sigmaSpace(new QDoubleSpinBox())
{
	sb_diameter->setRange(0, 10);
	sb_sigmaColor->setRange(0.0, 200.0);
	sb_sigmaSpace->setRange(0.0, 200.0);

	connect(sb_diameter, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb_sigmaColor, &QDoubleSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb_sigmaSpace, &QDoubleSpinBox::valueChanged, this, [=]() { applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("Diameter", sb_diameter);
	form->addRow("Color Sigma", sb_sigmaColor);
	form->addRow("Space Sigma", sb_sigmaSpace);

	_widget->setLayout(form);
}

void BilateralFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject BilateralFilterModel::save() const
{
	QJsonObject parameters;
	parameters["diameter"] = sb_diameter->value();
	parameters["sigma-color"] = sb_sigmaColor->value();
	parameters["sigma-space"] = sb_sigmaSpace->value();

	QJsonObject root;
	root["model-name"] = name();
	root["filter-parameters"] = parameters;
	root["filterable"] = true;

	return root;
}

void BilateralFilterModel::load(QJsonObject const& p)
{
	QJsonObject parameters = p["filter-parameters"].toObject();
	sb_diameter->setValue(parameters["diameter"].toInt(sb_diameter->value()));
	sb_sigmaColor->setValue(parameters["sigma-color"].toDouble(sb_sigmaColor->value()));
	sb_sigmaSpace->setValue(parameters["sigma-space"].toDouble(sb_sigmaSpace->value()));
}

void BilateralFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		const frameset::Frame inputFrame = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData)->frame();
		if (frameset::isEmpty(inputFrame))
			return;

		cv::Mat inputMat;
		frameset::toMat(inputFrame).convertTo(inputMat, CV_8U, 255.0 / 65535.0);
		cv::normalize(inputMat, inputMat, 0, 255, cv::NormTypes::NORM_MINMAX);
		cv::Mat filteredMat;
		cv::Mat outputMat;

		cv::bilateralFilter(
			inputMat,
			filteredMat,
			sb_diameter->value(),
			sb_sigmaColor->value(),
			sb_sigmaSpace->value());

		filteredMat.convertTo(outputMat, CV_16U, 65535.0 / 255.0);

		const frameset::Frame outputFrame = frameset::toFrame(outputMat);

		_currentNodeData = std::make_shared<FrameNodeData>(outputFrame);
	}
	emit dataUpdated(0);
}
