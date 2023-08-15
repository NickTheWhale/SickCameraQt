#include "GaussianBlurFilterModel.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qformlayout.h>

#include <opencv2/imgproc.hpp>

GaussianBlurFilterModel::GaussianBlurFilterModel() :
	_widget(new QWidget()),
	sb_sizeX(new QSpinBox()),
	sb_sizeY(new QSpinBox()),
	sb_sigmaX(new QDoubleSpinBox()),
	sb_sigmaY(new QDoubleSpinBox()),
	_filter(std::make_unique<GaussianBlurFilter>())
{
	sb_sizeX->setRange(1, 99);
	sb_sizeY->setRange(1, 99);
	sb_sigmaX->setRange(0.0, 100.0);
	sb_sigmaY->setRange(0.0, 100.0);

	connect(sb_sizeX, &QSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(sb_sizeY, &QSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaX, &QDoubleSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });
	connect(sb_sigmaY, &QDoubleSpinBox::valueChanged, this, [=]() { syncFilterParameters(); applyFilter(); });

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

	_widget->setLayout(vbox);
	syncFilterParameters();
}

void GaussianBlurFilterModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_originalNodeData = nodeData;
	_currentNodeData = nodeData;

	applyFilter();
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

	sb_sizeX->setValue(size["x"].toInt(sb_sizeX->value()));
	sb_sizeY->setValue(size["y"].toInt(sb_sizeY->value()));

	sb_sigmaX->setValue(sigma["x"].toDouble(sb_sigmaX->value()));
	sb_sigmaY->setValue(sigma["y"].toDouble(sb_sigmaY->value()));
}

void GaussianBlurFilterModel::syncFilterParameters() const
{
	QJsonObject size;
	size["x"] = makeOdd(sb_sizeX->value());
	size["y"] = makeOdd(sb_sizeY->value());

	QJsonObject sigma;
	sigma["x"] = sb_sigmaX->value();
	sigma["y"] = sb_sigmaY->value();

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
	if (number % 2 != 0)
		return number;

	return std::max(1, number - 1);
}