#pragma once
#include <NodeDelegateModel.hpp>

#include <MatNodeData.h>

#include <qspinbox.h>

#include <CropFilter.h>

class CropFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	CropFilterModel();
	~CropFilterModel() = default;

public:
	QString caption() const override { return QString("Crop"); }

	QString name() const override { return QString("Crop"); }

public:
	virtual QString modelName() const { return QString("Crop"); }

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

	QDoubleSpinBox* sb_width = nullptr;
	QDoubleSpinBox* sb_height = nullptr;
	
	QDoubleSpinBox* sb_centerX = nullptr;
	QDoubleSpinBox* sb_centerY = nullptr;

	double width;
	double height;

	double centerX;
	double centerY;

	std::unique_ptr<FilterBase> _filter;

	void syncFilterParameters() const;
	void applyFilter();
	void createWidgets();
};
