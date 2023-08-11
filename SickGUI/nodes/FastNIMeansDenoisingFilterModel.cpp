#include "FastNIMeansDenoisingFilterModel.h"

#include <qformlayout.h>

#include <opencv2/photo.hpp>

FastNIMeansDenoisingFilterModel::FastNIMeansDenoisingFilterModel() :
	_widget(new QWidget()),
	sb_h(new QDoubleSpinBox())
{
	sb_h->setRange(0.0, 100.0);
	connect(sb_h, &QDoubleSpinBox::valueChanged, this, [=]() { applyFilter(); });

	auto form = new QFormLayout();
	form->addRow("H", sb_h);

	_widget->setLayout(form);
}

void FastNIMeansDenoisingFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject FastNIMeansDenoisingFilterModel::save() const
{
	QJsonObject parameters;
	parameters["h"] = sb_h->value();

	QJsonObject root;
	root["model-name"] = modelName();
	root["filter-parameters"] = parameters;

	return root;
}

void FastNIMeansDenoisingFilterModel::load(QJsonObject const& p)
{
	QJsonObject parameters = p["filter-parameters"].toObject();
	sb_h->setValue(parameters["h"].toDouble(sb_h->value()));
}

void FastNIMeansDenoisingFilterModel::applyFilter()
{
	if (_originalNodeData)
	{
		const auto d = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData);
		if (d && !frameset::isEmpty(d->frame()))
		{
			const frameset::Frame inputFrame = std::dynamic_pointer_cast<FrameNodeData>(_originalNodeData)->frame();
			
			const cv::Mat inputMat = frameset::toMat(inputFrame);
			cv::Mat outputMat;
			
			std::vector<float> hVals;
			hVals.push_back(sb_h->value());
			cv::fastNlMeansDenoising(inputMat, outputMat, hVals, 7, 21, cv::NORM_L1);

			const frameset::Frame outputFrame = frameset::toFrame(outputMat);

			_currentNodeData = std::make_shared<FrameNodeData>(outputFrame);
		}
	}

	emit dataUpdated(0);
}
