#include "ResizeFilterModel.h"

#include <opencv2/imgproc.hpp>
#include <ResizeFilter.h>

#include <qlayout.h>
#include <qlabel.h>

ResizeFilterModel::ResizeFilterModel() :
	sizeX(10),
	sizeY(10),
	_filter(std::make_unique<ResizeFilter>())
{

}

void ResizeFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* ResizeFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject ResizeFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void ResizeFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	QJsonObject size = filterParameters["size"].toObject();

	sizeX = size["x"].toInt(sizeX);

	sizeY = size["y"].toInt(sizeY);
}

void ResizeFilterModel::syncFilterParameters() const
{
	QJsonObject size;
	size["x"] = sizeX;
	size["y"] = sizeY;

	QJsonObject parameters;
	parameters["size"] = size;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void ResizeFilterModel::applyFilter()
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

void ResizeFilterModel::createWidgets()
{
	sb_sizeX = new QSpinBox();
	sb_sizeY = new QSpinBox();

	sb_sizeX->setRange(1, 600);
	sb_sizeY->setRange(1, 600);

	sb_sizeX->setValue(sizeX);
	sb_sizeY->setValue(sizeY);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=](int value) { sizeX = value; syncFilterParameters(); applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=](int value) { sizeY = value; syncFilterParameters(); applyFilter(); });

	auto hboxTop = new QHBoxLayout();
	auto hboxBottom = new QHBoxLayout();
	hboxTop->addWidget(new QLabel("Size (px)"));
	hboxBottom->addWidget(sb_sizeX);
	hboxBottom->addWidget(new QLabel("x"));
	hboxBottom->addWidget(sb_sizeY);

	auto vbox = new QVBoxLayout();
	vbox->addLayout(hboxTop);
	vbox->addLayout(hboxBottom);

	_widget = new QWidget();
	_widget->setLayout(vbox);
	syncFilterParameters();
}
