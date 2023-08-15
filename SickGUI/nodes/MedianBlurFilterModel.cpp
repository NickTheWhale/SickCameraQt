#include "MedianBlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>

#include <opencv2/imgproc.hpp>

MedianBlurFilterModel::MedianBlurFilterModel() :
	_widget(new QWidget()),
	size3(new QRadioButton("3")),
	size5(new QRadioButton("5")),
	_filter(std::make_unique<MedianBlurFilter>())
{
	size3->setChecked(true);
	connect(size3, &QRadioButton::toggled, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(size5, &QRadioButton::toggled, this, [=]() { syncFilterParameters(); applyFilter(); });

	auto hbox = new QHBoxLayout();
	hbox->addWidget(size3);
	hbox->addWidget(size5);

	auto vbox = new QVBoxLayout();
	vbox->addWidget(new QLabel("Kernal Size (px)"));
	vbox->addLayout(hbox);

	_widget->setLayout(vbox);
	syncFilterParameters();
}

void MedianBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject MedianBlurFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void MedianBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	const int size = filterParameters["kernel-size"].toInt(0);
	if (size == 5)
		size5->setChecked(true);
	else
		size3->setChecked(true);
}

void MedianBlurFilterModel::syncFilterParameters() const
{
	QJsonObject parameters;
	parameters["kernel-size"] = size3->isChecked() ? 3 : 5;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void MedianBlurFilterModel::applyFilter()
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