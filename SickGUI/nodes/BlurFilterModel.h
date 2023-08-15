#pragma once
#include <NodeDelegateModel.hpp>

#include <MatNodeData.h>

#include <qspinbox.h>

#include <BlurFilter.h>

class BlurFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	BlurFilterModel();
	~BlurFilterModel() = default;

public:
	QString caption() const override { return QString("Blur"); }

	QString name() const override { return QString("Blur"); }

public:
	virtual QString modelName() const { return QString("Blur"); }

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

	QSpinBox* sb_sizeX = nullptr;
	QSpinBox* sb_sizeY = nullptr;

	std::unique_ptr<FilterBase> _filter;

	void syncFilterParameters() const;
	void applyFilter();
};

