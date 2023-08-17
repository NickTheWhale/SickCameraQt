#include "GaussianBlurFilterModel.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qformlayout.h>

#include <opencv2/imgproc.hpp>

GaussianBlurFilterModel::GaussianBlurFilterModel() :
	sizeX(1),
	sizeY(1),
	sigmaX(0.0),
	sigmaY(0.0),
	_filter(std::make_unique<GaussianBlurFilter>())
{

}

void GaussianBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
}

QWidget* GaussianBlurFilterModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject GaussianBlurFilterModel::save() const
{
	syncFilterParameters();
	QJsonObject root;
	root["model-name"] = name();
	root["filter"] = _filter->save();

	return root;
}

void GaussianBlurFilterModel::load(QJsonObject const& p)
{
	QJsonObject filterJson = p["filter"].toObject();
	_filter->load(filterJson);

	QJsonObject filterParameters = _filter->save()["parameters"].toObject();
	QJsonObject size = filterParameters["kernel-size"].toObject();
	QJsonObject sigma = filterParameters["sigma"].toObject();

	sizeX = size["x"].toInt(sizeX);
	sizeY = size["y"].toInt(sizeY);

	sigmaX = sigma["x"].toDouble(sigmaX);
	sigmaY = sigma["y"].toDouble(sigmaY);

	if (!_widget)
		createWidgets();
	sb_sizeX->setValue(sizeX);
	sb_sizeY->setValue(sizeY);
	sb_sigmaX->setValue(sigmaX);
	sb_sigmaY->setValue(sigmaY);
}

void GaussianBlurFilterModel::syncFilterParameters() const
{
	QJsonObject size;
	size["x"] = makeOdd(sizeX);
	size["y"] = makeOdd(sizeY);

	QJsonObject sigma;
	sigma["x"] = sigmaX;
	sigma["y"] = sigmaY;

	QJsonObject parameters;
	parameters["kernel-size"] = size;
	parameters["sigma"] = sigma;

	QJsonObject root;
	root["parameters"] = parameters;

	_filter->load(root);
}

void GaussianBlurFilterModel::applyFilter()
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

const int GaussianBlurFilterModel::makeOdd(const int number) const
{
	return number | 1;
}

void GaussianBlurFilterModel::createWidgets()
{
	sb_sizeX = new QSpinBox();
	sb_sizeY = new QSpinBox();
	sb_sigmaX = new QDoubleSpinBox();
	sb_sigmaY = new QDoubleSpinBox();

	sb_sizeX->setRange(1, 99);
	sb_sizeY->setRange(1, 99);
	sb_sigmaX->setRange(0.0, 100.0);
	sb_sigmaY->setRange(0.0, 100.0);

	sb_sizeX->setValue(sizeX);
	sb_sizeY->setValue(sizeY);
	sb_sigmaX->setValue(sigmaX);
	sb_sigmaY->setValue(sigmaY);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=](int value) { sizeX = makeOdd(value); syncFilterParameters(); applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=](int value) { sizeY = makeOdd(value); syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaX, &QDoubleSpinBox::valueChanged, this, [=](double value) { sigmaX = value; syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaY, &QDoubleSpinBox::valueChanged, this, [=](double value) { sigmaY = value; syncFilterParameters(); applyFilter(); });

	connect(sb_sizeX, &QSpinBox::editingFinished, this, [=]() { sb_sizeX->setValue(makeOdd(sb_sizeX->value())); });
	connect(sb_sizeY, &QSpinBox::editingFinished, this, [=]() { sb_sizeY->setValue(makeOdd(sb_sizeY->value())); });

	auto hbox = new QHBoxLayout();
	hbox->addWidget(sb_sizeX);
	hbox->addWidget(new QLabel("x"));
	hbox->addWidget(sb_sizeY);

	auto form = new QFormLayout();
	form->addRow("Sigma X", sb_sigmaX);
	form->addRow("Sigma Y", sb_sigmaY);

	auto vbox = new QVBoxLayout();
	vbox->addWidget(new QLabel("Kernal Size (px)"));
	vbox->addLayout(hbox);
	vbox->addLayout(form);

	_widget = new QWidget();
	_widget->setLayout(vbox);
	syncFilterParameters();
}
