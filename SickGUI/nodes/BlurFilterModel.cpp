#include "BlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>

#include <opencv2/imgproc.hpp>

BlurFilterModel::BlurFilterModel() :
	sizeX(1),
	sizeY(1),
	_filter(std::make_unique<BlurFilter>())
{
}

void BlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* BlurFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject BlurFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void BlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	QJsonObject size = filterParameters["kernel-size"].toObject();
	sizeX = size["x"].toInt(sizeX);
	sizeY = size["y"].toInt(sizeY);

	if (!_widget)
		createWidgets();
	sb_sizeX->setValue(sizeX);
	sb_sizeY->setValue(sizeY);
}

void BlurFilterModel::syncFilterParameters() const
{
	QJsonObject size;
	size["x"] = sizeX;
	size["y"] = sizeY;

	QJsonObject parameters;
	parameters["kernel-size"] = size;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void BlurFilterModel::applyFilter()
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

void BlurFilterModel::createWidgets()
{
	sb_sizeX = new QSpinBox();
	sb_sizeY = new QSpinBox();

	sb_sizeX->setRange(1, 99);
	sb_sizeY->setRange(1, 99);

	sb_sizeX->setValue(sizeX);
	sb_sizeY->setValue(sizeY);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=](int value) { sizeX = value; syncFilterParameters(); applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=](int value) { sizeY = value; syncFilterParameters(); applyFilter(); });

	auto hboxTop = new QHBoxLayout();
	auto hboxBottom = new QHBoxLayout();
	hboxTop->addWidget(new QLabel("Kernal Size (px)"));
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
