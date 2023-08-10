#include "StackBlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qevent.h>

#include <opencv2/imgproc.hpp>

StackBlurFilterModel::StackBlurFilterModel() :
	_widget(new QWidget()),
	sb1(new QSpinBox()),
	sb2(new QSpinBox())
{
	sb1->setRange(1, 101);
	sb2->setRange(1, 101);

	sb1->setSingleStep(2);
	sb2->setSingleStep(2);

	connect(sb1, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb2, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });

	connect(sb1, &QSpinBox::editingFinished, this, [=]() { sb1->setValue(makeOdd(sb1->value())); });
	connect(sb2, &QSpinBox::editingFinished, this, [=]() { sb2->setValue(makeOdd(sb2->value())); });

	auto hboxTop = new QHBoxLayout();
	auto hboxBottom = new QHBoxLayout();
	hboxTop->addWidget(new QLabel("Kernal Size (px)"));
	hboxBottom->addWidget(sb1);
	hboxBottom->addWidget(new QLabel("x"));
	hboxBottom->addWidget(sb2);

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

void StackBlurFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		const frameset::Frame inputFrame = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData)->frame();
		if (frameset::isEmpty(inputFrame))
			return;
		// stack blur
		const cv::Mat inputMat = frameset::toMat(inputFrame);
		cv::Mat outputMat;
		cv::Size size(makeOdd(sb1->value()), makeOdd(sb2->value()));
		cv::stackBlur(inputMat, outputMat, size);
		const frameset::Frame outputFrame = frameset::toFrame(outputMat);

		_currentNodeData = std::make_shared<FrameNodeData>(outputFrame);
	}

	emit dataUpdated(0);
}

const int StackBlurFilterModel::makeOdd(const int number) const
{
	if (number % 2 != 0)
		return number;

	return std::max(1, number - 1);
}