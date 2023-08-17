#include "FrameViewerModel.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qcombobox.h>

FrameViewerModel::FrameViewerModel() :
	colorIndex(0)
{
}

void FrameViewerModel::setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex)
{
	_currentNodeData = nodeData;

	if (_currentNodeData)
	{
		const auto d = std::dynamic_pointer_cast<MatNodeData>(_currentNodeData);
		if (d && !d->mat().empty())
		{
			const frameset::Frame frame = frameset::toFrame(d->mat());
			auto options = frameset::ImageOptions();
			options.colormap = colorActions[_colorBox->currentIndex()].colormapType;
			_image->setPixmap(QPixmap::fromImage(frameset::toQImage(frame, options)));
		}
	}
	else
	{
		_image->setPixmap(QPixmap());
	}

	emit dataUpdated(0);
}

QWidget* FrameViewerModel::embeddedWidget()
{
	if (!_widget)
		createWidgets();
	return _widget;
}

QJsonObject FrameViewerModel::save() const
{
	QJsonObject root;
	root["model-name"] = name();
	root["colormap"] = colorIndex;

	return root;
}

void FrameViewerModel::load(QJsonObject const& p)
{
	colorIndex = p["colormap"].toInt(colorIndex);

	if (!_widget)
		createWidgets();
	_colorBox->setCurrentIndex(std::clamp(colorIndex, 0, _colorBox->maxCount()));
}

void FrameViewerModel::createWidgets()
{
	_image = new ImageLabel();
	_colorBox = new QComboBox();

	for (const auto& action : colorActions)
		_colorBox->addItem(action.name, action.colormapType);

	_colorBox->setCurrentIndex(std::clamp(colorIndex, 0, _colorBox->maxCount()));

	connect(_colorBox, &QComboBox::currentIndexChanged, this, [=](int index) { colorIndex = index; });

	QVBoxLayout* vbox = new QVBoxLayout();
	vbox->addWidget(_image);
	vbox->addWidget(_colorBox);

	_image->setMinimumSize(100, 100);

	_widget = new QWidget();
	_widget->setLayout(vbox);
}
