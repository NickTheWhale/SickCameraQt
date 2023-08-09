#pragma once
#include <NodeDelegateModel.hpp>
#include <ImageLabel.h>
#include <Frameset.h>
#include <ThreadInterface.h>

class FrameSourceModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT
		
public:
	FrameSourceModel();
	~FrameSourceModel() = default;

public:
	QString caption() const override { return QString("Frame Source"); }

	QString name() const override { return QString("Source"); }

public:
	virtual QString modelName() const { return QString("Frame Source"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override;

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override;

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override;

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override {};

	QWidget* embeddedWidget() override { return _widget; }

	bool resizable() const override { return true; }

private:
	QWidget* _widget;
	ImageLabel* _image;

	ThreadInterface& threadInterface;

	frameset::Frame _frame;
};

