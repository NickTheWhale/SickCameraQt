/*****************************************************************//**
 * @file   FilterEditorWidget.h
 * @brief  Adds some filtering specific functions to QtNodes::GraphicsView.
 * Handles verifying filter paths and displaying the current filters
 *
 * @author Nicholas Loehrke
 * @date   September 2023
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


/**
 * @brief Registers nodes so they can be added to the node editor.
 */
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
	void setApplyButtonGeometry(const QSize size);

	/**
	 * @brief Sets 'filterTextEdit' geometry and position based on visibility.
	 *
	 * @param size this->size()
	 */
	void setTextEditGeometry(const QSize size);

	/**
	 * @brief Calls 'setApplyButtonGeometry' and 'setTextEditGeometry'.
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

	using PlcIds = std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>>;
	/**
	 * @brief Searches the graph and finds the id's of every plc start and end node.
	 *
	 * @return vector of plc start and end nodes
	 */
	const PlcIds getPlcIds() const;

	// type aliases to clarity
	using ValidJson = bool;
	using NodeIdSet = std::unordered_set<QtNodes::NodeId>;
	using ValidateIdSet = NodeIdSet;
	using DefaultIdSet = NodeIdSet;
	using InvalidateIdSet = NodeIdSet;
	using FlowTuple = std::tuple<ValidJson, QJsonArray, ValidateIdSet, DefaultIdSet, InvalidateIdSet, QString>;

	/**
	 * @brief Computes the filter flow based on PLC flags and graph connections. A filter flow is the
	 * 'filterable' nodes connected between the start and end PLC flags.
	 *
	 * This method computes the filter flow based on the plc flag nodes and the connections within the
	 * graph. It checks if there are exactly 1 start and 1 end plc flag node, and if they are connected
	 * through a non branching chain of 'filterable' nodes. It's also used to determine what nodes
	 * should be validated, defaulted, or invalidated. These three states are then used to color code
	 * the nodes so the user can see what they need to fix to apply the filters to the capture thread.
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

	/**
	 * @brief Handles the internal computeFilterFlow case where there is an invalid number of
	 * PLC nodes. It's considered invalid if there is not exactly 1 start plc node and
	 * 1 end plc node.
	 *
	 * @param plcIds Pair of start and end plc node vectors.
	 * @return FlowTuple
	 */
	FlowTuple handleInvalidFlagCount(const PlcIds& plcIds);

	/**
	 * @brief Handles the internal computeFilterFlow case where there is a valid plc node
	 * count, but the nodes are not connected.
	 *
	 * @param plcIds Pair of start and end plc node vectors.
	 * @return FlowTuple
	 */
	FlowTuple handleDisconnectedFlags(const PlcIds& plcIds);

	/**
	 * @brief Handles the internal computeFilterFlow case where there is a valid plc node
	 * count and the plc nodes are connected directly to each other (not through any other
	 * nodes).
	 *
	 * @param plcIds Pair of start and end plc node vectors.
	 * @return FlowTuple
	 */
	FlowTuple handleUniquelyConnectedFlags(const PlcIds& plcIds);

	/**
	 * @brief Handles the internal computeFilterFlow case where there is a valid plc node
	 * count and the plc nodes are connected through a single path of filterable nodes. A
	 * node is filterable if the json representation of the node contains 'filter'
	 *
	 * @param nonBranchingIds Node id's that are connected between start and end plc nodes.
	 * @return FlowTuple
	 */
	FlowTuple handleConnectedFlags(const std::vector<QtNodes::NodeId>& nonBranchingIds);

	/**
	 * @brief Handles the internal computeFilterFlow case where there is a valid plc node
	 * count and the plc nodes are connected through either a branching path, or through
	 * non-filterable nodes.
	 *
	 * @param plcEndNode Node id of the plc end flag node.
	 * @param branchingIds Set of all the ids connected between the start and end plc flags.
	 * @return FlowTuple
	 */
	FlowTuple handleInvalidConnectedFlags(const QtNodes::NodeId plcEndNode, const NodeIdSet& branchingIds);

	/**
	 * @brief Convience method equivalent to
	 * @code
	 * {
	 *     updateNodeStyles(validStyle, validateSet);
	 *     updateNodeStyles(defaultStyle, defaultSet);
	 *     updateNodeStyles(invalidStyle, invalidateSet);
	 * }
	 * @endcode
	 *
	 * @param validateSet
	 * @param defaultSet
	 * @param invalidateSet
	 */
	void updateAllNodeStyles(const ValidateIdSet& validateSet, const DefaultIdSet& defaultSet, const InvalidateIdSet& invalidateSet);

	/**
	 * @brief Identical to updateAllNodeStyles(validateSet, defaultSet, invalidateSet) but the id sets are
	 * computed within the function.
	 *
	 */
	void updateAllNodeStyles();

	/**
	 * @brief Sets the given node id's to the given style.
	 *
	 * @param style Style to set.
	 * @param ids Nodes to set.
	 */
	void updateNodeStyles(const QtNodes::Style& style, const std::vector<QtNodes::NodeId>& ids);

	/**
	 * @brief Sets the given node id's to the given style.
	 *
	 * @param style Style to set.
	 * @param ids Nodes to set.
	 */
	void updateNodeStyles(const QtNodes::Style& style, const std::unordered_set<QtNodes::NodeId>& ids);

	/**
	 * @brief Recursively generates html from a json object.
	 * 
	 * @param obj Json object. Recursion allows nested objects.
	 * @param html String to append generated html to.
	 */
	void jsonObjectToHtml(const QJsonObject& obj, QString& html);

	/**
	 * @brief Iterates array and generates html using jsonObjectToHtml.
	 * 
	 * @param array Input filter json array.
	 * @return html string.
	 */
	QString filterJsonArrayToHtml(const QJsonArray& array);

private slots:
	/**
	 * @brief Apply button callback.
	 * 
	 * The method first computes the filter flow with computeFilterFlow(). It then emits the filter json if
	 * the flow is valid and updates the node styles. It also shows a popup message if the flow is invalid.
	 */
	void applyFilters();


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

	// flag to block automatic style updates when loading flows from disk
	bool blockAutomaticStyleUpdates = false;

	QtNodes::NodeStyle validStyle;
	QtNodes::NodeStyle invalidStyle;
	QtNodes::NodeStyle defaultStyle;
};

