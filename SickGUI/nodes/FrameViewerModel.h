#pragma once
#include <NodeDelegateModel.hpp>
#include <ImageLabel.h>
#include <Frameset.h>
#include <NodeData.hpp>


class FrameViewerModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	FrameViewerModel();
	~FrameViewerModel() = default;

public:
	QString caption() const override { return QString("Frame Viewer"); }

	QString name() const override { return QString("Viewer"); }

public:
	virtual QString modelName() const { return QString("Frame Viewer"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override { return 1; }

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override;

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	QWidget* embeddedWidget() override { return _widget; }

	bool resizable() const override { return true; }

private:
	QWidget* _widget;
	ImageLabel* _image;
	frameset::Frame _frame;

	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
};

