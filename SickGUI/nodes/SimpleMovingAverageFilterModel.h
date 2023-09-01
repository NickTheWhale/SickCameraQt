/*****************************************************************//**
 * @file   SimpleMovingAverageFilterModel.h
 * @brief  Implements QtNodes::NodeDelegateModel.
 *
 * @note See BilateralFilterModel method documentation since all models follow the same pattern.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <NodeDelegateModel.hpp>

#include <MatNodeData.h>
#include <SimpleMovingAverageFilter.h>

#include <qspinbox.h>


class SimpleMovingAverageFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	SimpleMovingAverageFilterModel();
	~SimpleMovingAverageFilterModel() = default;

public:
	QString caption() const override { return QString("Simple Moving Average Filter"); }

	QString name() const override { return QString("Simple Moving Average Filter"); }

public:
	virtual QString modelName() const { return QString("Simple Moving Average Filter"); }

	bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; };

	unsigned int nPorts(QtNodes::PortType const portType) const override { return 1; }

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return MatNodeData().type(); }

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	QWidget* embeddedWidget() override;

	bool resizable() const override { return false; }

	QJsonObject save() const override;

	void load(QJsonObject const& p) override;

private:
	QWidget* _widget = nullptr;
	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	std::shared_ptr<QtNodes::NodeData> _originalNodeData;

	QSpinBox* sb_bufferSize = nullptr;

	int bufferSize;

	std::unique_ptr<FilterBase> _filter;
	void syncFilterParameters() const;
	void applyFilter();
	void createWidgets();
};

