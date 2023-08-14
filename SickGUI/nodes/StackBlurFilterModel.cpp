#include "StackBlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qevent.h>

#include <opencv2/imgproc.hpp>

StackBlurFilterModel::StackBlurFilterModel() :
	_widget(new QWidget()),
	sb_sizeX(new QSpinBox()),
	sb_sizeY(new QSpinBox())
{
	sb_sizeX->setRange(1, 99);
	sb_sizeY->setRange(1, 99);

	sb_sizeX->setSingleStep(2);
	sb_sizeY->setSingleStep(2);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });

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

	_widget->setLayout(vbox);
}

void StackBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject StackBlurFilterModel::save() const
{
	QJsonObject size;
	size["x"] = sb_sizeX->value();
	size["y"] = sb_sizeY->value();

	QJsonObject parameters;
	parameters["kernel-size"] = size;

	QJsonObject root;
	root["model-name"] = name();
	root["filter-parameters"] = parameters;
	root["filterable"] = true;

	return root;
}

void StackBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject parameters = p["filter-parameters"].toObject();
	QJsonObject size = parameters["kernel-size"].toObject();

	sb_sizeX->setValue(makeOdd(size["x"].toInt(sb_sizeX->value())));
	sb_sizeY->setValue(makeOdd(size["y"].toInt(sb_sizeX->value())));
}

void StackBlurFilterModel::applyFilter()
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
			cv::stackBlur(inputMat, outputMat, size);
			const frameset::Frame outputFrame = frameset::toFrame(outputMat);

			_currentNodeData = std::make_shared<FrameNodeData>(outputFrame);
		}
	}

	emit dataUpdated(0);
}

const int StackBlurFilterModel::makeOdd(const int number) const
{
	if (number % 2 != 0)
		return number;

	return std::max(1, number - 1);
}