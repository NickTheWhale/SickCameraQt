#pragma once
#include <NodeDelegateModel.hpp>

#include <FrameNodeData.h>

class SubtractFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	SubtractFilterModel();
	~SubtractFilterModel() = default;

public:
	QString caption() const override { return QString("Subtract"); }

	QString name() const override { return QString("Subtract"); }

public:
	virtual QString modelName() const { return QString("Subtract"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override;

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return FrameNodeData().type(); }

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	QWidget* embeddedWidget() override { return _widget; }

	bool resizable() const override { return false; }

private:
	QWidget* _widget;
	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	std::shared_ptr<QtNodes::NodeData> _portNodeData0;
	std::shared_ptr<QtNodes::NodeData> _portNodeData1;

	void applyFilter();
};

