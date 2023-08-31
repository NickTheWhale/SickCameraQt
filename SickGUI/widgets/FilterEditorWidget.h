/*****************************************************************//**
 * @file   FilterEditorWidget.h
 * @brief  Adds some filtering specific functions to QtNodes::GraphicsView. 
 * Handles verifying filter paths and displaying the current filters
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once

#include <GraphicsView.hpp>
#include <NodeDelegateModelRegistry.hpp>
#include <FilterFlowGraphModel.h>
#include <FilterFlowGraphicsScene.h>
#include <FilterManager.h>

#include <qwidget.h>
#include <qpushbutton.h>
#include <qscrollarea.h>
#include <qtextedit.h>
#include <qlabel.h>

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels();

class FilterEditorWidget : public QtNodes::GraphicsView
{
	Q_OBJECT

public slots:
	/**
	 * @brief Saves current flow to disk.
	 */
	void save();

	/**
	 * @brief Loads flow from disk.
	 */
	void load();

	/**
	 * @brief Updates the left hand panel that shows what filters are currently being applied by the capture thread.
	 * 
	 * @param filters Json representation of the filter applied objects.
	 */
	void captureFiltersApplied(const QJsonArray& filters);

	/**
	 * @brief Called when the capture thread fails to apply a filter(s).
	 * 
	 */
	void captureFiltersFailed();

signals:
	/**
	 * @brief Used to pass new filters to the capture thread.
	 * @param filters Json representation of the filter objects to apply.
	 */
	void updatedFilters(const QJsonArray& filters);

public:
	FilterEditorWidget(QWidget* parent = nullptr);
	~FilterEditorWidget();

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	/**
	 * @brief Sets 'applyButton' geometry and position based on 'filterTextEdit' visibility.
	 * 
	 * @param size this->size()
	 */
	void setButtonGeometry(const QSize size);

	/**
	 * @brief Sets 'filterTextEdit' geometry and position based on visibility.
	 * 
	 * @param size this->size()
	 */
	void setTextEditGeometry(const QSize size);

	/**
	 * @brief Calls 'setButtonGeometry' and 'setTextEditGeometry'.
	 * 
	 * @param size this->size()
	 */
	void setAllGeometry(const QSize size);

	/**
	 * @brief Shows popup message.
	 * 
	 * @param msg Message to show
	 */
	void showMessage(const QString msg);

	/**
	 * @brief Searches the graph and finds the id's of every plc start and end node.
	 * 
	 * @return vector of plc start and end nodes
	 */
	using PlcIds = std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>>;
	const PlcIds getPlcIds() const;

	// all these 'using' keywords are just used to save typing in the methods
	using ValidJson = bool;
	using NodeIdSet = std::unordered_set<QtNodes::NodeId>;
	using ValidateIdSet = NodeIdSet;
	using DefaultIdSet = NodeIdSet;
	using InvalidateIdSet = NodeIdSet;
	using FlowTuple = std::tuple<ValidJson, QJsonArray, ValidateIdSet, DefaultIdSet, InvalidateIdSet, QString>;
	
	/**
	 * @brief Computes the filter flow based on PLC flags and graph connections.
	 * 
	 * This method computes the filter flow based on the plc flag nodes and the connections within the
	 * graph. If checks if there are exactly 1 start and 1 end plc flag node, and if they are connected
	 * through a non branching chain of 'filterable' nodes.
	 * 
	 * @return A tuple containing information about the computed filter flow.
	 *          - The first element indicates whether the filter flow is valid.
	 *          - The second element is a json array containing the json representation of the filter objects.
	 *          - The third element is a set of node id's to set style to valid.
	 *          - The fourth element is a set of node id's to set style to default.
	 *          - The fifth element is a set of node id's to set style to invalid.
	 *          - The sixth element is a message describing the outcome of the computation
	 */
	FlowTuple computeFilterFlow();
	FlowTuple handleInvalidFlagCount(const PlcIds& plcIds);
	FlowTuple handleDisconnectedFlags(const PlcIds& plcIds);
	FlowTuple handleUniquelyConnectedFlags(const PlcIds& plcIds);
	FlowTuple handleConnectedFlags(const std::vector<QtNodes::NodeId>& nonBranchingIds);
	FlowTuple handleInvalidConnectedFlags(const QtNodes::NodeId plcEndNode, const NodeIdSet& branchingIds);

	void applyFilters();
	void updateAllColors(const ValidateIdSet& validateSet, const DefaultIdSet& defaultSet, const InvalidateIdSet& invalidateSet);
	void updateAllColors();
	void updateNodeColors(const QtNodes::Style& style, const std::vector<QtNodes::NodeId>& ids);
	void updateNodeColors(const QtNodes::Style& style, const std::unordered_set<QtNodes::NodeId>& ids);

private:
	QtNodes::FilterFlowGraphModel graph;
	QtNodes::FilterFlowGraphicsScene* scene = nullptr;
	std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry;

	FilterManager filterManager;

	QPushButton* applyButton = nullptr;

	QPushButton* textEditButton = nullptr;
	QTextEdit* filterTextEdit = nullptr;
	bool textEditVisible = true;

	QJsonArray lastFilters;

	bool blockAutomaticColorUpdates = false;

	QtNodes::NodeStyle validStyle;
	QtNodes::NodeStyle invalidStyle;
	QtNodes::NodeStyle defaultStyle;
};

