#pragma once
#include <NodeDelegateModel.hpp>

#include <MatNodeData.h>
#include <ResizeFilter.h>

#include <qspinbox.h>

class ResizeFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	ResizeFilterModel();
	~ResizeFilterModel() = default;

public:
	QString caption() const override { return QString("Resize"); }

	QString name() const override { return QString("Resize"); }

public:
	virtual QString modelName() const { return QString("Resize"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override { return 1; }

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return MatNodeData().type(); }

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

	QSpinBox* sb_sizeX;
	QSpinBox* sb_sizeY;

	std::unique_ptr<FilterBase> _filter;
	void syncFilterParameters() const;
	void applyFilter();
};

