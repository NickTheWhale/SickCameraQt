/*****************************************************************//**
 * @file   BilateralFilterModel.h
 * @brief  Implements QtNodes::NodeDelegateModel.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once
#include <NodeDelegateModel.hpp>

#include <MatNodeData.h>
#include <BilateralFilter.h>

#include <qspinbox.h>


class BilateralFilterModel : public QtNodes::NodeDelegateModel
{
	Q_OBJECT

public:
	BilateralFilterModel();
	~BilateralFilterModel() = default;

public:
	/**
	 * @brief Displayed as a title on the node.
	 * 
	 * @return 
	 */
	QString caption() const override { return QString("Bilateral Filter"); }

	/**
	 * @brief Unique name to identify node.
	 * 
	 * @return 
	 */
	QString name() const override { return QString("Bilateral Filter"); }

public:
	/**
	 * @brief Honestly not sure where this is used.
	 * 
	 * @return 
	 */
	virtual QString modelName() const { return QString("Bilateral Filter"); }

	/**
	 * @brief Controls whether a given port type and index is given a caption.
	 * 
	 * @return true.
	 */
	bool portCaptionVisible(QtNodes::PortType, QtNodes::PortIndex) const override { return true; };

	/**
	 * @brief Specifies number of in or out ports depending on given portType.
	 * 
	 * @param portType Porttype.
	 * @return 1.
	 */
	unsigned int nPorts(QtNodes::PortType const portType) const override { return 1; }

	/**
	 * @brief Specifies data type used per port type and index.
	 * 
	 * @param portType Porttype.
	 * @param portIndex Portindex.
	 * @return MatNodeData::type;
	 */
	QtNodes::NodeDataType dataType(QtNodes::PortType const portType, QtNodes::PortIndex const portIndex) const override { return MatNodeData().type(); }

	/**
	 * @brief Returns the current node data used to propagate data.
	 * 
	 * @param port Port index.
	 * @return current node data.
	 */
	std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex const port) override { return _currentNodeData; }

	/**
	 * @brief Sets the input data. Triggers the filtering.
	 * 
	 * This method is called when a connection is made to an input port 
	 * or new data is propagated to an input port.
	 * 
	 * @param nodeData Nodedata
	 * @param portIndex Port index.
	 */
	void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex const portIndex) override;

	/**
	 * @brief Return the embedded widget used to show parameter inputs.
	 * 
	 * @return Embedded widget.
	 */
	QWidget* embeddedWidget() override;

	/**
	 * @brief Specifies whether the embedded widget is resizable.
	 * 
	 * @return False.
	 */
	bool resizable() const override { return false; }

	/**
	 * @brief Saves the model name and internal filter object to json. Used to save and load nodes
	 * from disk.
	 * 
	 * @return Json
	 */
	QJsonObject save() const override;

	/**
	 * @brief Loads parameters/settings from given json. Used to save and load nodes from disk.
	 * 
	 * @param p
	 */
	void load(QJsonObject const& p) override;

private:
	QWidget* _widget = nullptr;
	std::shared_ptr<QtNodes::NodeData> _currentNodeData;
	std::shared_ptr<QtNodes::NodeData> _originalNodeData;

	QSpinBox* sb_diameter = nullptr;
	QDoubleSpinBox* sb_sigmaColor = nullptr;
	QDoubleSpinBox* sb_sigmaSpace = nullptr;

	int diameter;
	double sigmaColor;
	double sigmaSpace;

	std::unique_ptr<FilterBase> _filter;

	/**
	 * @brief Sets filter parameters from the widget controls.
	 * 
	 */
	void syncFilterParameters() const;

	/**
	 * @brief Apply filter to current node data from original node data.
	 * 
	 */
	void applyFilter();

	/**
	 * @brief Creates controls and embedded widget.
	 * 
	 */
	void createWidgets();
};

