#pragma once
#include <NodeDelegateModel.hpp>
#include <ImageLabel.h>
#include <Frameset.h>
#include <NodeData.hpp>

#include <MatNodeData.h>

#include <qcombobox.h>

namespace 
{
	struct ColorMapActionInfo
	{
		QString name;
		tinycolormap::ColormapType colormapType;
	};

	ColorMapActionInfo colorActions[] =
	{
		{"Gray",      tinycolormap::ColormapType::Gray},
		{"Jet",	      tinycolormap::ColormapType::Jet},
		{"Heat",      tinycolormap::ColormapType::Heat},
		{"Hot",       tinycolormap::ColormapType::Hot},
		{"Github",    tinycolormap::ColormapType::Github},
		{"Turbo",     tinycolormap::ColormapType::Turbo},
		{"TurboFast", tinycolormap::ColormapType::TurboFast}
	};
}

class FrameViewerModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	FrameViewerModel();
	~FrameViewerModel() = default;

public:
	QString caption() const override { return QString("Frame Viewer"); }

	QString name() const override { return QString("Frame Viewer"); }

public:
	virtual QString modelName() const { return QString("Frame Viewer"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override { return 1; }

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return MatNodeData().type(); }

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	QWidget* embeddedWidget() override;

	bool resizable() const override { return true; }

	QJsonObject save() const override;

	void load(QJsonObject const& p) override;

private:
	QWidget* _widget = nullptr;
	ImageLabel* _image = nullptr;

	QComboBox* _colorBox = nullptr;
	int colorIndex;

	frameset::Frame _frame;

	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	void createWidgets();
};

