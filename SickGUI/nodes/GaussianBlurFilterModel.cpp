#include "GaussianBlurFilterModel.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qformlayout.h>

#include <opencv2/imgproc.hpp>

GaussianBlurFilterModel::GaussianBlurFilterModel() :
	_widget(new QWidget()),
	sb_sizeX(new QSpinBox()),
	sb_sizeY(new QSpinBox()),
	sb_sigmaX(new QDoubleSpinBox()),
	sb_sigmaY(new QDoubleSpinBox())
{
	sb_sizeX->setRange(1, 99);
	sb_sizeY->setRange(1, 99);
	sb_sigmaX->setRange(0.0, 100.0);
	sb_sigmaY->setRange(0.0, 100.0);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb_sigmaX, &QDoubleSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb_sigmaY, &QDoubleSpinBox::valueChanged, this, [=]() { applyFilter(); });

	connect(sb_sizeX, &QSpinBox::editingFinished, this, [=]() { sb_sizeX->setValue(makeOdd(sb_sizeX->value())); });
	connect(sb_sizeY, &QSpinBox::editingFinished, this, [=]() { sb_sizeY->setValue(makeOdd(sb_sizeY->value())); });

	auto hbox = new QHBoxLayout();
	hbox->addWidget(sb_sizeX);
	hbox->addWidget(new QLabel("x"));
	hbox->addWidget(sb_sizeY);

	auto form = new QFormLayout();
	form->addRow("Sigma X", sb_sigmaX);
	form->addRow("Sigma Y", sb_sigmaY);

	auto vbox = new QVBoxLayout();
	vbox->addWidget(new QLabel("Kernal Size (px)"));
	vbox->addLayout(hbox);

	_widget->setLayout(vbox);
}

void GaussianBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject GaussianBlurFilterModel::save() const
{
	QJsonObject size;
	size["x"] = sb_sizeX->value();
	size["y"] = sb_sizeY->value();

	QJsonObject sigma;
	sigma["x"] = sb_sigmaX->value();
	sigma["y"] = sb_sigmaY->value();

	QJsonObject parameters;
	parameters["kernel-size"] = size;
	parameters["sigma"] = sigma;

	QJsonObject root;
	root["model-name"] = modelName();
	root["filter-parameters"] = parameters;

	return root;
}

void GaussianBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject parameters = p["filter-parameters"].toObject();
	QJsonObject size = parameters["kernel-size"].toObject();
	QJsonObject sigma = parameters["sigma"].toObject();

	sb_sizeX->setValue(size["x"].toInt(sb_sizeX->value()));
	sb_sizeY->setValue(size["y"].toInt(sb_sizeY->value()));

	sb_sigmaX->setValue(sigma["x"].toDouble(sb_sigmaX->value()));
	sb_sigmaY->setValue(sigma["y"].toDouble(sb_sigmaY->value()));
}

void GaussianBlurFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		const auto d = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData);
		if (d && !frameset::isEmpty(d->frame()))
		{
			const frameset::Frame inputFrame = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData)->frame();
			
			const cv::Mat inputMat = frameset::toMat(inputFrame);
			cv::Mat outputMat;
			cv::Size size(makeOdd(sb_sizeX->value()), makeOdd(sb_sizeY->value()));

			cv::GaussianBlur(inputMat, outputMat, size, sb_sigmaX->value(), sb_sigmaY->value());
			
			const frameset::Frame outputFrame = frameset::toFrame(outputMat);

			_currentNodeData = std::make_shared<FrameNodeData>(outputFrame);
		}
	}

	emit dataUpdated(0);
}

const int GaussianBlurFilterModel::makeOdd(const int number) const
{
	if (number % 2 != 0)
		return number;

	return std::max(1, number - 1);
}