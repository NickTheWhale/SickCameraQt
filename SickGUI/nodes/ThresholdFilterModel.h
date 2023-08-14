#pragma once
#include <NodeDelegateModel.hpp>

#include <FrameNodeData.h>

#include <qspinbox.h>

class ThresholdFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	ThresholdFilterModel();
	~ThresholdFilterModel() = default;

public:
	QString caption() const override { return QString("Threshold Filter"); }

	QString name() const override { return QString("Threshold Filter"); }

public:
	virtual QString modelName() const { return QString("Threshold Filter"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override { return 1; }

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return FrameNodeData().type(); }

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	QWidget* embeddedWidget() override { return _widget; }

	bool resizable() const override { return false; }

	QJsonObject save() const override;

	void load(QJsonObject const& p) override;

private:
	QWidget* _widget;
	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	std::shared_ptr<QtNodes::NodeData> _originalNodeData;
	
	QSpinBox* sb_lower;
	QSpinBox* sb_upper;

	void applyFilter();
};

