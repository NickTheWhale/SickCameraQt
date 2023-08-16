#pragma once
#include <NodeDelegateModel.hpp>

#include <MatNodeData.h>

class PlcStartModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	PlcStartModel();
	~PlcStartModel() = default;

public:
	QString caption() const override { return QString("Plc Start"); }

	QString name() const override { return QString("Plc Start Flag"); }

public:
	virtual QString modelName() const { return QString("PlcStartModel"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override;

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return MatNodeData().type(); }

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override {};

	QWidget* embeddedWidget() override;

	bool resizable() const override { return false; }

	QJsonObject save() const override;

	void load(QJsonObject const& p) override;

private:
	QWidget* _widget = nullptr;
	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	std::shared_ptr<QtNodes::NodeData> _originalNodeData;

	void createWidgets();
};

