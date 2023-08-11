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

QJsonObject MedianBlurFilterModel::save() const
{
	QJsonObject parameters;
	parameters["kernel-size"] = size3->isChecked() ? 3 : 5;

	QJsonObject root;
	root["model-name"] = modelName();
	root["filter-parameters"] = parameters;

	return root;
}

void MedianBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject parameters = p["filter-parameters"].toObject();
	const int size = parameters["kernel-size"].toInt(0);
	if (size == 5)
		size5->setChecked(true);
	else
		size3->setChecked(true);
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