#include "StackBlurFilterModel.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qevent.h>

#include <opencv2/imgproc.hpp>

StackBlurFilterModel::StackBlurFilterModel() :
	_widget(new QWidget()),
	sb_sizeX(new QSpinBox()),
	sb_sizeY(new QSpinBox()),
	_filter(std::make_unique<StackBlurFilter>())
{
	sb_sizeX->setRange(1, 99);
	sb_sizeY->setRange(1, 99);

	sb_sizeX->setSingleStep(2);
	sb_sizeY->setSingleStep(2);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });

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
	syncFilterParameters();
}

void StackBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QJsonObject StackBlurFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void StackBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	QJsonObject size = filterParameters["kernel-size"].toObject();

	sb_sizeX->setValue(makeOdd(size["x"].toInt(sb_sizeX->value())));
	sb_sizeY->setValue(makeOdd(size["y"].toInt(sb_sizeX->value())));
}

void StackBlurFilterModel::syncFilterParameters() const
{
	QJsonObject size;
	size["x"] = sb_sizeX->value();
	size["y"] = sb_sizeY->value();

	QJsonObject parameters;
	parameters["kernel-size"] = size;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void StackBlurFilterModel::applyFilter()
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

const int StackBlurFilterModel::makeOdd(const int number) const
{
	if (number % 2 != 0)
		return number;

	return std::max(1, number - 1);
}