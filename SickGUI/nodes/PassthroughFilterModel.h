#pragma once
#include <NodeDelegateModel.hpp>

class PassthroughFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	PassthroughFilterModel();
	~PassthroughFilterModel() = default;

public:
	QString caption() const override { return QString("Passthrough Filter"); }

	QString name() const override { return QString("Passthrough"); }

public:
	virtual QString modelName() const { return QString("Passthrough Filter"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override;

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override;

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override;

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	QWidget* embeddedWidget() override { return _widget; }

	bool resizable() const override { return true; }

private:
	QWidget* _widget;
	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	std::shared_ptr<QtNodes::NodeData> _originalNodeData;
	
	uint16_t lower;
	uint16_t upper;

	void applyFilter();
};

