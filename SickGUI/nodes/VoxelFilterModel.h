#pragma once
#include <NodeDelegateModel.hpp>

#include <qspinbox.h>

class VoxelFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	VoxelFilterModel();
	~VoxelFilterModel() = default;

public:
	QString caption() const override { return QString("Voxel Grid Filter"); }

	QString name() const override { return QString("Voxel Grid"); }

public:
	virtual QString modelName() const { return QString("Voxel Grid Filter"); }

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

	QSpinBox* sb1;
	QSpinBox* sb2;
	QSpinBox* sb3;

	void applyFilter();
};

