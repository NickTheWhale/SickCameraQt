#pragma once
#include <NodeDelegateModel.hpp>
#include <ImageLabel.h>
#include <Frameset.h>
#include <ThreadInterface.h>

#include <MatNodeData.h>

class PlcSourceModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	PlcSourceModel();
	~PlcSourceModel() = default;

public:
	QString caption() const override { return QString("Plc Source"); }

	QString name() const override { return QString("Plc Source"); }

public:
	virtual QString modelName() const { return QString("Plc Source"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override;

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return MatNodeData().type(); }

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override;

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override {};

	QWidget* embeddedWidget() override;

	bool resizable() const override { return false; }

	QJsonObject save() const override;

	void load(QJsonObject const& p) override;

private:
	QWidget* _widget = nullptr;

	ThreadInterface& threadInterface;

	frameset::Frame _frame;
	void createWidgets();
};
