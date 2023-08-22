#pragma once
#include <NodeDelegateModel.hpp>

#include <qspinbox.h>

#include <MatNodeData.h>
#include <StackBlurFilter.h>

class StackBlurFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	StackBlurFilterModel();
	~StackBlurFilterModel() = default;

public:
	QString caption() const override { return QString("Stack Blur"); }

	QString name() const override { return QString("Stack Blur"); }

public:
	virtual QString modelName() const { return QString("Stack Blur"); }

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

	QSpinBox* sb_sizeX = nullptr;
	QSpinBox* sb_sizeY = nullptr;

	int sizeX;
	int sizeY;

	std::unique_ptr<FilterBase> _filter;
	void syncFilterParameters() const;
	void applyFilter();
	const int makeOdd(const int number) const;
	void createWidgets();
};

