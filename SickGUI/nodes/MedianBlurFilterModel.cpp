#include "MedianBlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>

#include <opencv2/imgproc.hpp>

MedianBlurFilterModel::MedianBlurFilterModel() :
	_widget(new QWidget()),
	size3(new QRadioButton("3")),
	size5(new QRadioButton("5"))
{
	size3->setChecked(true);
	size5->setChecked(false);

	connect(size3, &QRadioButton::toggled, this, [=]() { applyFilter(); });
	connect(size5, &QRadioButton::toggled, this, [=]() { applyFilter(); });

	auto hbox = new QHBoxLayout();
	hbox->addWidget(size3);
	hbox->addWidget(size5);

	auto vbox = new QVBoxLayout();
	vbox->addWidget(new QLabel("Kernal Size (px)"));
	vbox->addLayout(hbox);

	_widget->setLayout(vbox);
}

void MedianBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

void MedianBlurFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		const auto d = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData);
		if (d && !frameset::isEmpty(d->frame()))
		{
			const frameset::Frame inputFrame = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData)->frame();

			const cv::Mat inputMat = frameset::toMat(inputFrame);
			cv::Mat outputMat;

			int size = 3;
			if (size5->isChecked())
				size = 5;

			cv::medianBlur(inputMat, outputMat, size);

			const frameset::Frame outputFrame = frameset::toFrame(outputMat);

			_currentNodeData = std::make_shared<FrameNodeData>(outputFrame);
		}
	}

	emit dataUpdated(0);
}