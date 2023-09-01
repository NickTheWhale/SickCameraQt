/*****************************************************************//**
 * @file   FilterEditorWidget.cpp
 * @brief  Adds some filtering specific functions to QtNodes::GraphicsView.
 * Handles verifying filter paths and displaying the current filters
 *
 * @author Nicholas Loehrke
 * @date   September 2023
 *********************************************************************/

#include "FilterEditorWidget.h"

#include <PlcStartModel.h>
#include <PlcEndModel.h>
#include <BilateralFilterModel.h>
#include <BlurFilterModel.h>
#include <FastNIMeansDenoisingFilterModel.h>
#include <GaussianBlurFilterModel.h>
#include <MedianBlurFilterModel.h>
#include <ThresholdFilterModel.h>
#include <StackBlurFilterModel.h>
#include <ResizeFilterModel.h>
#include <SubtractFilterModel.h>
#include <CropFilterModel.h>
#include <SimpleMovingAverageFilterModel.h>
#include <ExponentialMovingAverageFilterModel.h>
#include <FrameSourceModel.h>
#include <FrameViewerModel.h>
#include <PlcSourceModel.h>

#include <qlayout.h>
#include <qdebug.h>
#include <qstack.h>
#include <qtimer.h>
#include <qmessagebox.h>

#include <algorithm>

FilterEditorWidget::FilterEditorWidget(QWidget* parent) :
	GraphicsView(parent),
	graph(registerDataModels()),
	scene(new QtNodes::FilterFlowGraphicsScene(graph, this)),
	applyButton(new QPushButton("Apply Flow To PLC Stream", this)),
	textEditButton(new QPushButton(this)),
	filterTextEdit(new QTextEdit(this))
{
	// disable some controls for the current filter display
	filterTextEdit->setReadOnly(true);
	filterTextEdit->setUndoRedoEnabled(false);

	// cache node styles
	validStyle.loadJsonFile(":/SickGUI/ValidStyle.json");
	invalidStyle.loadJsonFile(":/SickGUI/InvalidStyle.json");
	defaultStyle.loadJsonFile(":/SickGUI/DefaultStyle.json");

	// apply filter button connection
	connect(applyButton, &QPushButton::pressed, this, &FilterEditorWidget::applyFilters);

	// show/hide current filter display
	connect(textEditButton, &QPushButton::pressed, this, [=]()
		{
			textEditVisible = !textEditVisible;
			setAllGeometry(size());
		});

	// the following connections are used to update the node style anytime you change nodes

	connect(&graph, &QtNodes::FilterFlowGraphModel::connectionCreated, this, [&]()
		{
			if (!blockAutomaticStyleUpdates)
				updateAllNodeStyles();
		});
	connect(&graph, &QtNodes::FilterFlowGraphModel::connectionDeleted, this, [&]()
		{
			if (!blockAutomaticStyleUpdates)
				updateAllNodeStyles();
		});
	connect(&graph, &QtNodes::FilterFlowGraphModel::nodeCreated, this, [&]()
		{
			if (!blockAutomaticStyleUpdates)
				updateAllNodeStyles();
		});
	connect(&graph, &QtNodes::FilterFlowGraphModel::nodeDeleted, this, [&]()
		{
			if (!blockAutomaticStyleUpdates)
				updateAllNodeStyles();
		});

	// make the current filter display translucent
	filterTextEdit->setStyleSheet("QTextEdit { background-color: rgba(255, 255, 255, 200) }");

	// layout geometry stuff
	this->setMinimumSize(500, 300);
	this->setScene(scene);
	setAllGeometry(size());
	adjustSize();
}

FilterEditorWidget::~FilterEditorWidget()
{
}

void FilterEditorWidget::resizeEvent(QResizeEvent* event)
{
	// updates free floating widgets (filter display, filter display button, apply button)
	setAllGeometry(event->size());
	GraphicsView::resizeEvent(event);
}

void FilterEditorWidget::setApplyButtonGeometry(const QSize size)
{
	// calculates apply button position based on widget size

	const QPoint bottomRight(size.width(), size.height());
	const QPoint pad(5, 5);
	const QPoint applyButtonSize(applyButton->size().width(), applyButton->size().height());
	const QPoint applyPos = bottomRight - pad - applyButtonSize;
	applyButton->move(applyPos);
}

void FilterEditorWidget::setTextEditGeometry(const QSize size)
{
	// calculates filter display and button positions based on filter display
	//   visibility and widget size

	const QPoint pad(5, 5);
	filterTextEdit->setVisible(textEditVisible);
	if (filterTextEdit->isVisible())
	{
		filterTextEdit->move(pad);
		const QSize textEditSize = QSize(400, size.height() - pad.y() - pad.y());
		filterTextEdit->resize(textEditSize);
		textEditButton->move(QPoint(filterTextEdit->width() + pad.x() + 3, pad.y() - 1));
		textEditButton->setIcon(QIcon(":/SickGUI/icons/chevron_left_FILL0_wght400_GRAD0_opsz40.png"));
	}
	else
	{
		textEditButton->move(pad);
		textEditButton->setIcon(QIcon(":/SickGUI/icons/chevron_right_FILL0_wght400_GRAD0_opsz40.png"));
	}
}

void FilterEditorWidget::setAllGeometry(const QSize size)
{
	setApplyButtonGeometry(size);
	setTextEditGeometry(size);
}

void FilterEditorWidget::showMessage(const QString msg)
{
	QMessageBox::information(this, "Message", msg);
}

const std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>> FilterEditorWidget::getPlcIds() const
{
	std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>> ids;

	// checks if given id is a plc start node based on node name
	auto isPlcStartNode = [&](const QtNodes::NodeId id) -> bool { return graph.nodeData(id, QtNodes::NodeRole::Type) == PlcStartModel().name(); };
	// checks if given id is a plc end node based on node name
	auto isPlcEndNode = [&](const QtNodes::NodeId id) -> bool { return graph.nodeData(id, QtNodes::NodeRole::Type) == PlcEndModel().name(); };

	// iterate all node id's and check if its a plc start or end node
	for (const auto& id : graph.allNodeIds())
	{
		if (isPlcStartNode(id))
			ids.first.push_back(id);
		else if (isPlcEndNode(id))
			ids.second.push_back(id);
	}

	return ids;
}

FilterEditorWidget::FlowTuple FilterEditorWidget::computeFilterFlow()
{
	QString message;
	// get all of the plc node ids
	const PlcIds plcIds = getPlcIds();
	// number of start nodes
	const size_t startIdCount = plcIds.first.size();
	// number of end nodes
	const size_t endIdCount = plcIds.second.size();

	// check if there is a valid amount of plc nodes (1 start and 1 end)
	if (startIdCount != 1 || endIdCount != 1)
	{
		return handleInvalidFlagCount(plcIds);
	}

	// if there is a valid amount, then we can get the start node id and end node id
	const QtNodes::NodeId plcStartNode = plcIds.first.front();
	const QtNodes::NodeId plcEndNode = plcIds.second.front();

	// check if the plc nodes are connected
	if (!graph.nodesConnected(plcStartNode, plcEndNode))
	{
		return handleDisconnectedFlags(plcIds);
	}

	// check if the plc nodes are connected directly to each other (not through any filters)
	if (graph.nodesUniquelyConnected(plcStartNode, plcEndNode))
	{
		return handleUniquelyConnectedFlags(plcIds);
	}

	// check if the plc nodes are connected through a non-branching path, only through filter nodes
	const auto nonBranchingIds = graph.nonBranchingFilterableConnections(plcStartNode, plcEndNode);
	if (!nonBranchingIds.empty())
	{
		return handleConnectedFlags(nonBranchingIds);
	}

	// check if the plc nodes are connected through a branching path with any node type
	const NodeIdSet branchingIds = graph.branchingConnections(plcStartNode, plcEndNode);
	if (!branchingIds.empty())
	{
		return handleInvalidConnectedFlags(plcEndNode, branchingIds);
	}

	// base case
	return FlowTuple{ false, QJsonArray(), ValidateIdSet(), DefaultIdSet(), InvalidateIdSet(), QString() };
}

FilterEditorWidget::FlowTuple FilterEditorWidget::handleInvalidFlagCount(const PlcIds& plcIds)
{
	const bool validFilterJson = false;
	const QJsonArray filterJson = QJsonArray();
	const ValidateIdSet idsToValidate = ValidateIdSet();
	DefaultIdSet idsToDefault;
	InvalidateIdSet idsToInvalidate;
	const QString message = "Invalid PLC flag count, you must have 1 of each.";

	// all of the start plc id's should be invalidated and any nodes connected to each of the start plc nodes
	for (const auto& plcId : plcIds.first)
	{
		idsToInvalidate.insert(plcId);
		// invalidate any connected nodes connected to the plc start node
		const auto plcBranchIds = graph.branchingConnections(plcId);
		for (const auto& id : plcBranchIds)
			idsToInvalidate.insert(id);
	}

	// all of the end plc id's should be invalidated and any nodes connected to each of the end plc nodes
	for (const auto& plcId : plcIds.second)
	{
		idsToInvalidate.insert(plcId);
		// invalidate any connected nodes connected to the plc end node
		const auto plcBranchIds = graph.branchingConnections(plcId);
		for (const auto& id : plcBranchIds)
			idsToInvalidate.insert(id);
	}

	// any non plc node id should be defaulted
	const auto allIds = graph.allNodeIds();
	for (const auto& id : allIds)
		if (idsToInvalidate.count(id) == 0)
			idsToDefault.insert(id);

	return std::make_tuple(validFilterJson, filterJson, idsToValidate, idsToDefault, idsToInvalidate, message);
}

FilterEditorWidget::FlowTuple FilterEditorWidget::handleDisconnectedFlags(const PlcIds& plcIds)
{
	const bool validFilterJson = false;
	const QJsonArray filterJson = QJsonArray();
	const ValidateIdSet idsToValidate = ValidateIdSet();
	DefaultIdSet idsToDefault;
	InvalidateIdSet idsToInvalidate;
	const QString message = "PLC flags not connected.";

	// all of the start plc id's should be invalidated and any nodes connected to each of the start plc nodes
	for (const auto& plcId : plcIds.first)
	{
		idsToInvalidate.insert(plcId);
		// invalidate any connected nodes connected to the plc start node
		const auto plcBranchIds = graph.branchingConnections(plcId);
		for (const auto& id : plcBranchIds)
			idsToInvalidate.insert(id);
	}

	// all of the end plc id's should be invalidated and any nodes connected to each of the end plc nodes
	for (const auto& plcId : plcIds.second)
	{
		idsToInvalidate.insert(plcId);
		// invalidate any connected nodes connected to the plc end node
		const auto plcBranchIds = graph.branchingConnections(plcId);
		for (const auto& id : plcBranchIds)
			idsToInvalidate.insert(id);
	}

	// any non plc node id should be defaulted
	const auto allIds = graph.allNodeIds();
	for (const auto& id : allIds)
		if (idsToInvalidate.count(id) == 0)
			idsToDefault.insert(id);

	return std::make_tuple(validFilterJson, filterJson, idsToValidate, idsToDefault, idsToInvalidate, message);
}

FilterEditorWidget::FlowTuple FilterEditorWidget::handleUniquelyConnectedFlags(const PlcIds& plcIds)
{
	const bool filterJsonValid = true;
	const QJsonArray filterJson = QJsonArray();
	ValidateIdSet idsToValidate;
	DefaultIdSet idsToDefault;
	const InvalidateIdSet idsToInvalidate = InvalidateIdSet();
	const QString message = "PLC flags are uniquely connected.";

	// "all" start ids should be validated. I say "all" because there really should only be 1 at this point.
	for (const auto& id : plcIds.first)
		idsToValidate.insert(id);
	// "all" end ids should be validated. I say "all" because there really should only be 1 at this point.
	for (const auto& id : plcIds.second)
		idsToValidate.insert(id);

	// any non plc node id should be defaulted
	const auto allIds = graph.allNodeIds();
	for (const auto& id : allIds)
		if (idsToValidate.count(id) == 0)
			idsToDefault.insert(id);

	return std::make_tuple(filterJsonValid, filterJson, idsToValidate, idsToDefault, idsToInvalidate, message);
}

FilterEditorWidget::FlowTuple FilterEditorWidget::handleConnectedFlags(const std::vector<QtNodes::NodeId>& nonBranchingIds)
{
	const bool filterJsonValid = true;
	QJsonArray filterJson;
	ValidateIdSet idsToValidate;
	DefaultIdSet idsToDefault;
	const InvalidateIdSet idsToInvalidate = InvalidateIdSet();
	const QString message = "PLC flags are connected";

	// for all node id's in the connection path, we validate it and add it to the filterJson if its a filter
	for (const auto& id : nonBranchingIds)
	{
		// get the json representation of the filter object
		QJsonObject filter = graph.saveNode(id)["internal-data"].toObject()["filter"].toObject();
		if (!filter.empty())
			filterJson.append(filter);
		idsToValidate.insert(id);
	}

	// any id thats not to be validated should be defaulted
	const auto allIds = graph.allNodeIds();
	for (const auto& id : allIds)
		if (idsToValidate.count(id) == 0)
			idsToDefault.insert(id);

	return std::make_tuple(filterJsonValid, filterJson, idsToValidate, idsToDefault, idsToInvalidate, message);
}

FilterEditorWidget::FlowTuple FilterEditorWidget::handleInvalidConnectedFlags(const QtNodes::NodeId plcEndNode, const NodeIdSet& branchingIds)
{
	const bool filterJsonValid = false;
	const QJsonArray filterJson = QJsonArray();
	const ValidateIdSet idsToValidate = ValidateIdSet();
	DefaultIdSet idsToDefault;
	InvalidateIdSet idsToInvalidate = branchingIds;
	const QString message = "PLC flags are connected through invalid path.";

	// all nodes connected to the end node should be invalidated
	idsToInvalidate.insert(plcEndNode);
	InvalidateIdSet endBranches = graph.branchingConnections(plcEndNode);
	for (const auto& id : endBranches)
		idsToInvalidate.insert(id);

	// any id thats not to be validated should be defaulted
	const auto allIds = graph.allNodeIds();
	for (const auto& id : allIds)
		if (idsToInvalidate.count(id) == 0)
			idsToDefault.insert(id);

	return std::make_tuple(filterJsonValid, filterJson, idsToValidate, idsToDefault, idsToInvalidate, message);
}

void FilterEditorWidget::applyFilters()
{
	const FlowTuple flow = computeFilterFlow();
	const ValidJson validJson = std::get<0>(flow);
	// if the filter json if valid, emit the filter json array
	if (validJson)
	{
		emit updatedFilters(std::get<1>(flow));
	}
	// otherwise, show a message
	else
	{
		showMessage(std::get<5>(flow));
	}
	// update node styles based on the computed flow
	updateAllNodeStyles(std::get<2>(flow), std::get<3>(flow), std::get<4>(flow));
}

void FilterEditorWidget::updateAllNodeStyles(const ValidateIdSet& validateSet, const DefaultIdSet& defaultSet, const InvalidateIdSet& invalidateSet)
{
	updateNodeStyles(validStyle, validateSet);
	updateNodeStyles(defaultStyle, defaultSet);
	updateNodeStyles(invalidStyle, invalidateSet);
}

void FilterEditorWidget::updateAllNodeStyles()
{
	const FlowTuple flow = computeFilterFlow();
	updateAllNodeStyles(std::get<2>(flow), std::get<3>(flow), std::get<4>(flow));
}

void FilterEditorWidget::updateNodeStyles(const QtNodes::Style& style, const std::vector<QtNodes::NodeId>& ids)
{
	// for each node, update the style and signal that the node has been updated. This triggers a repaint
	for (const auto& id : ids)
	{
		graph.setNodeData(id, QtNodes::NodeRole::Style, style.toJson());
		emit graph.nodeUpdated(id);
	}
}

void FilterEditorWidget::updateNodeStyles(const QtNodes::Style& style, const std::unordered_set<QtNodes::NodeId>& ids)
{
	// for each node, update the style and signal that the node has been updated. This triggers a repaint
	for (const auto& id : ids)
	{
		graph.setNodeData(id, QtNodes::NodeRole::Style, style.toJson());
		emit graph.nodeUpdated(id);
	}
}

std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels()
{
	auto ret = std::make_shared<QtNodes::NodeDelegateModelRegistry>();

	ret->registerModel<PlcStartModel>("Plc");
	ret->registerModel<PlcEndModel>("Plc");
	ret->registerModel<BilateralFilterModel>("Filters");
	ret->registerModel<BlurFilterModel>("Filters");
	ret->registerModel<FastNIMeansDenoisingFilterModel>("Filters");
	ret->registerModel<GaussianBlurFilterModel>("Filters");
	ret->registerModel<MedianBlurFilterModel>("Filters");
	ret->registerModel<ThresholdFilterModel>("Filters");
	ret->registerModel<StackBlurFilterModel>("Filters");
	ret->registerModel<ResizeFilterModel>("Filters");
	ret->registerModel<SubtractFilterModel>("Filters");
	ret->registerModel<CropFilterModel>("Filters");
	ret->registerModel<SimpleMovingAverageFilterModel>("Filters");
	ret->registerModel<ExponentialMovingAverageFilterModel>("Filters");
	ret->registerModel<FrameSourceModel>("Sources");
	ret->registerModel<PlcSourceModel>("Sources");
	ret->registerModel<FrameViewerModel>("Viewers");

	return ret;
}

void FilterEditorWidget::save()
{
	// saves nodes
	scene->save();
}

void FilterEditorWidget::load()
{
	try
	{
		// block style since updateNodeStyles() can be kinda slow when adding a bunch of nodes
		blockAutomaticStyleUpdates = true;
		scene->load();
	}
	catch (const std::logic_error& e)
	{
		qWarning() << "Failed to load node." << e.what();
	}
	catch (...)
	{
		qWarning() << "Failed to load node(s). This may be due to invalid json format or outdated flow file";
	}
	blockAutomaticStyleUpdates = false;
	// update node styles once everything has loaded
	updateAllNodeStyles();
}

void FilterEditorWidget::jsonObjectToHtml(const QJsonObject& obj, QString& html)
{
	QJsonObject::const_iterator it;
	// iterate through object
	for (it = obj.constBegin(); it != obj.constEnd(); ++it)
	{
		// if the value is an object, recur
		if (it.value().isObject())
		{
			html += "<dt>";
			html += "<div style=\"font-size:16px\">";
			html += it.key();
			html += "</div>";
			html += "</dt>";

			html += "<dd>";
			html += "<dl>";
			jsonObjectToHtml(it.value().toObject(), html);
			html += "</dl>";
			html += "</dd>";
		}
		// otherwise, show value
		else
		{
			html += "<dt>";
			html += "<div style=\"font-size:16px\">";
			html += it.key() + ": " + it.value().toVariant().toString();
			html += "</div>";
			html += "</dt>";
		}
	}
}

QString FilterEditorWidget::filterJsonArrayToHtml(const QJsonArray& array)
{
	QString html;
	html += "<dl>";

	size_t filterNum = 0;
	for (const auto& filter : array)
	{
		// type or name of the filter
		const auto type = filter.toObject()["type"].toVariant().toString();
		// filter parameters
		const auto parameters = filter.toObject()["parameters"].toObject();

		html += "<dt>";
		html += "<div style=\"font-size:20px;\">";
		html += QString::number(filterNum + 1) + ". " + type;
		html += "</div>";
		html += "</dt>";

		html += "<dd>";
		html += "<dl>";
		jsonObjectToHtml(parameters, html);
		html += "</dl>";
		html += "</dd>";

		// add a horizontal line
		if (filterNum < array.size() - 1)
			html += "<hr>";

		++filterNum;
	}

	html += "</dl>";
	return html;
}

void FilterEditorWidget::captureFiltersApplied(const QJsonArray& filters)
{
	// prevent unnecessary updates
	if (filters == lastFilters && !filters.empty())
		return;

	lastFilters = filters;

	QString html = "<b>Current Filters:</b><br>";

	// if the filters are empty, say thath
	if (lastFilters.empty())
	{
		html += "<br>No filters being applied";
	}
	// otherwise, generate html 
	else
	{
		html += filterJsonArrayToHtml(lastFilters);
	}

	filterTextEdit->setHtml(html);
}

void FilterEditorWidget::captureFiltersFailed()
{
	// reset filter cache
	lastFilters = QJsonArray();
	filterTextEdit->setHtml("<b>Failed to apply filters</b>");
}
