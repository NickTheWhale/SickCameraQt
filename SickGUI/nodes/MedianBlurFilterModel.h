/*****************************************************************//**
 * @file   MedianBlurFilterModel.h
 * @brief  MedianBlurFilter node.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <NodeDelegateModel.hpp>

#include <MatNodeData.h>
#include <MedianBlurFilter.h>

#include <qradiobutton.h>

class MedianBlurFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	MedianBlurFilterModel();
	~MedianBlurFilterModel() = default;

public:
	QString caption() const override { return QString("Median Blur"); }

	QString name() const override { return QString("Median Blur"); }

public:
	virtual QString modelName() const { return QString("Median Blur"); }

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

	QRadioButton* rb_size3 = nullptr;
	QRadioButton* rb_size5 = nullptr;

	enum KERNEL_SIZE {SMALL = 3, LARGE = 5};
	KERNEL_SIZE kernelSize;

	std::unique_ptr<FilterBase> _filter;
	void syncFilterParameters() const;
	void applyFilter();
	void sizeSelected();
	void createWidgets();
};

