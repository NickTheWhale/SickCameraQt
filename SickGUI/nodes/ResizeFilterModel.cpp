#include "ResizeFilterModel.h"

#include <opencv2/imgproc.hpp>

#include <qlayout.h>
#include <qlabel.h>

ResizeFilterModel::ResizeFilterModel() :
	_widget(new QWidget()),
	sb1(new QSpinBox()),
	sb2(new QSpinBox())
{
	sb1->setRange(1, 600);
	sb2->setRange(1, 600);

	connect(sb1, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });
	connect(sb2, &QSpinBox::valueChanged, this, [=]() { applyFilter(); });

	auto hboxTop = new QHBoxLayout();
	auto hboxBottom = new QHBoxLayout();
	hboxTop->addWidget(new QLabel("Size (px)"));
	hboxBottom->addWidget(sb1);
	hboxBottom->addWidget(new QLabel("x"));
	hboxBottom->addWidget(sb2);

	auto vbox = new QVBoxLayout();
	vbox->addLayout(hboxTop);
	vbox->addLayout(hboxBottom);

	_widget->setLayout(vbox);
}

void ResizeFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

void ResizeFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		const auto d = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData);
		if (d && !frameset::isEmpty(d->frame()))
		{
			const frameset::Frame inputFrame = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData)->frame();
			const cv::Mat inputMat = frameset::toMat(inputFrame);
			cv::Mat outputMat;
			cv::Size size(sb1->value(), sb2->value());
			cv::resize(inputMat, outputMat, size, 0.0f, 0.0f, cv::InterpolationFlags::INTER_AREA);

			const frameset::Frame outputFrame = frameset::toFrame(outputMat);

			_currentNodeData = std::make_shared<FrameNodeData>(outputFrame);
		}
	}
	emit dataUpdated(0);
}
