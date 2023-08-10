#pragma once
#include <NodeDelegateModel.hpp>

#include <FrameNodeData.h>

#include <qspinbox.h>

class FastNIMeansDenoisingFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	FastNIMeansDenoisingFilterModel();
	~FastNIMeansDenoisingFilterModel() = default;

public:
	QString caption() const override { return QString("Non-Local Means Denoising"); }

	QString name() const override { return QString("Non-Local Means Denoising"); }

public:
	virtual QString modelName() const { return QString("Non-Local Means Denoising"); }

	unsigned int nPorts(QtNodes::PortType const portType) const override { return 1; }

	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return FrameNodeData().type(); }

	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	QWidget* embeddedWidget() override { return _widget; }

	bool resizable() const override { return false; }

private:
	QWidget* _widget;
	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	std::shared_ptr<QtNodes::NodeData> _originalNodeData;

	QDoubleSpinBox* sb_h;

	void applyFilter();
};

