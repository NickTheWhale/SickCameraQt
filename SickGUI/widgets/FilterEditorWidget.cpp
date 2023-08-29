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
	textEdit(new QTextEdit(this))
{
	textEdit->setReadOnly(true);
	textEdit->setUndoRedoEnabled(false);

	validStyle.loadJsonFile(":/SickGUI/ValidStyle.json");
	invalidStyle.loadJsonFile(":/SickGUI/InvalidStyle.json");
	defaultStyle.loadJsonFile(":/SickGUI/DefaultStyle.json");

	connect(applyButton, &QPushButton::pressed, this, &FilterEditorWidget::applyFilters);
	connect(textEditButton, &QPushButton::pressed, this, [=]()
		{
			textEditVisible = !textEditVisible;
			setAllGeometry(size());
		});
	connect(&graph, &QtNodes::FilterFlowGraphModel::connectionCreated, this, [&]()
		{
			if (!blockAutomaticColorUpdates)
				updateAllColors();
		});
	connect(&graph, &QtNodes::FilterFlowGraphModel::connectionDeleted, this, [&]()
		{
			if (!blockAutomaticColorUpdates)
				updateAllColors();
		});
	connect(&graph, &QtNodes::FilterFlowGraphModel::nodeCreated, this, [&]()
		{
			if (!blockAutomaticColorUpdates)
				updateAllColors();
		});
	connect(&graph, &QtNodes::FilterFlowGraphModel::nodeDeleted, this, [&]()
		{
			if (!blockAutomaticColorUpdates)
				updateAllColors();
		});

	this->setMinimumSize(500, 300);
	textEdit->setStyleSheet("QTextEdit { background-color: rgba(255, 255, 255, 200) }");
	this->setScene(scene);
	setAllGeometry(size());
	adjustSize();
}

FilterEditorWidget::~FilterEditorWidget()
{
}

void FilterEditorWidget::resizeEvent(QResizeEvent* event)
{
	setAllGeometry(event->size());
	GraphicsView::resizeEvent(event);
}

void FilterEditorWidget::setButtonGeometry(const QSize size)
{
	const QPoint bottomRight(size.width(), size.height());
	const QPoint pad(5, 5);
	const QPoint applyButtonSize(applyButton->size().width(), applyButton->size().height());
	const QPoint applyPos = bottomRight - pad - applyButtonSize;
	applyButton->move(applyPos);
}

void FilterEditorWidget::setTextEditGeometry(const QSize size)
{
	const QPoint pad(5, 5);
	textEdit->setVisible(textEditVisible);
	if (textEdit->isVisible())
	{
		textEdit->move(pad);
		const QSize textEditSize = QSize(400, size.height() - pad.y() - pad.y());
		textEdit->resize(textEditSize);
		textEditButton->move(QPoint(textEdit->width() + pad.x() + 3, pad.y() - 1));
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
	setButtonGeometry(size);
	setTextEditGeometry(size);
}

void FilterEditorWidget::showMessage(const QString msg)
{
	QMessageBox::information(this, "Message", msg);
}

const std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>> FilterEditorWidget::getPlcIds() const
{
	std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>> ids;

	auto isPlcStartNode = [&](const QtNodes::NodeId id) -> bool { return graph.nodeData(id, QtNodes::NodeRole::Type) == PlcStartModel().name(); };
	auto isPlcEndNode = [&](const QtNodes::NodeId id) -> bool { return graph.nodeData(id, QtNodes::NodeRole::Type) == PlcEndModel().name(); };


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
	const auto plcIds = getPlcIds();
	const size_t startIdCount = plcIds.first.size();
	const size_t endIdCount = plcIds.second.size();
	if (startIdCount != 1 || endIdCount != 1)
	{
		message = "Invalid PLC flag count, you must have 1 of each.";

		InvalidateIdSet idsToInvalidate;
		for (const auto& plcId : plcIds.first)
		{
			idsToInvalidate.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToInvalidate.insert(id);
		}

		for (const auto& plcId : plcIds.second)
		{
			idsToInvalidate.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToInvalidate.insert(id);
		}

		DefaultIdSet idsToDefault;
		const auto allIds = graph.allNodeIds();
		for (const auto& id : allIds)
			if (idsToInvalidate.count(id) == 0)
				idsToDefault.insert(id);

		return std::make_tuple(false, QJsonArray(), ValidateIdSet(), idsToDefault, idsToInvalidate, message);
	}

	const QtNodes::NodeId plcStartNode = plcIds.first.front();
	const QtNodes::NodeId plcEndNode = plcIds.second.front();

	if (!graph.nodesConnected(plcStartNode, plcEndNode))
	{
		message = "PLC flags not connected.";

		InvalidateIdSet idsToInvalidate;
		for (const auto& plcId : plcIds.first)
		{
			idsToInvalidate.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToInvalidate.insert(id);
		}

		for (const auto& plcId : plcIds.second)
		{
			idsToInvalidate.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToInvalidate.insert(id);
		}

		DefaultIdSet idsToDefault;
		const auto allIds = graph.allNodeIds();
		for (const auto& id : allIds)
			if (idsToInvalidate.count(id) == 0)
				idsToDefault.insert(id);

		return std::make_tuple(false, QJsonArray(), ValidateIdSet(), idsToDefault, idsToInvalidate, message);
	}

	if (graph.nodesUniquelyConnected(plcStartNode, plcEndNode))
	{
		message = "PLC flags are uniquely connected.";

		ValidateIdSet idsToValidate;
		for (const auto& id : plcIds.first)
			idsToValidate.insert(id);
		for (const auto& id : plcIds.second)
			idsToValidate.insert(id);

		DefaultIdSet idsToDefault;
		const auto allIds = graph.allNodeIds();
		for (const auto& id : allIds)
			if (idsToValidate.count(id) == 0)
				idsToDefault.insert(id);

		return std::make_tuple(true, QJsonArray(), idsToValidate, idsToDefault, InvalidateIdSet(), message);
	}

	const auto nonBranchingIds = graph.nonBranchingConnections(plcStartNode, plcEndNode);
	if (!nonBranchingIds.empty())
	{
		message = "PLC flags are connected";

		ValidateIdSet idsToValidate;
		QJsonArray json;
		for (const auto& id : nonBranchingIds)
		{
			QJsonObject filter = graph.saveNode(id)["internal-data"].toObject()["filter"].toObject();
			if (!filter.empty())
				json.append(filter);
			idsToValidate.insert(id);
		}

		DefaultIdSet idsToDefault;
		const auto allIds = graph.allNodeIds();
		for (const auto& id : allIds)
			if (idsToValidate.count(id) == 0)
				idsToDefault.insert(id);

		return std::make_tuple(true, json, idsToValidate, idsToDefault, InvalidateIdSet(), message);
	}

	const auto branchingIds = graph.branchingConnections(plcStartNode, plcEndNode);
	if (!branchingIds.empty())
	{
		message = "PLC flags are connected through invalid path.";

		InvalidateIdSet idsToInvalidate = branchingIds;
		idsToInvalidate.insert(plcEndNode);
		InvalidateIdSet endBranches = graph.branchingConnections(plcEndNode);
		for (const auto& id : endBranches)
			idsToInvalidate.insert(id);

		DefaultIdSet idsToDefault;
		const auto allIds = graph.allNodeIds();
		for (const auto& id : allIds)
			if (idsToInvalidate.count(id) == 0)
				idsToDefault.insert(id);

		return std::make_tuple(false, QJsonArray(), ValidateIdSet(), DefaultIdSet(), idsToInvalidate, message);
	}

	return FlowTuple{ false, QJsonArray(), ValidateIdSet(), DefaultIdSet(), InvalidateIdSet(), QString() };
}

void FilterEditorWidget::applyFilters()
{
	const FlowTuple flow = computeFilterFlow();
	const ValidJson validJson = std::get<0>(flow);
	if (validJson)
	{
		emit updatedFilters(std::get<1>(flow));
	}
	else
	{
		showMessage(std::get<5>(flow));
	}
	updateAllColors(std::get<2>(flow), std::get<3>(flow), std::get<4>(flow));
}

void FilterEditorWidget::updateAllColors(const ValidateIdSet& validateSet, const DefaultIdSet& defaultSet, const InvalidateIdSet& invalidateSet)
{
	updateNodeColors(validStyle, validateSet);
	updateNodeColors(defaultStyle, defaultSet);
	updateNodeColors(invalidStyle, invalidateSet);
}

void FilterEditorWidget::updateAllColors()
{
	const FlowTuple flow = computeFilterFlow();
	updateAllColors(std::get<2>(flow), std::get<3>(flow), std::get<4>(flow));
}

void FilterEditorWidget::updateNodeColors(const QtNodes::Style& style, const std::vector<QtNodes::NodeId>& ids)
{
	for (const auto& id : ids)
	{
		graph.setNodeData(id, QtNodes::NodeRole::Style, style.toJson());
		emit graph.nodeUpdated(id);
	}
}

void FilterEditorWidget::updateNodeColors(const QtNodes::Style& style, const std::unordered_set<QtNodes::NodeId>& ids)
{
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
	scene->save();
}

void FilterEditorWidget::load()
{
	try
	{
		blockAutomaticColorUpdates = true;
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
	blockAutomaticColorUpdates = false;
	updateAllColors();
}

void jsonObjectToHtml(const QJsonObject& obj, QString& html)
{
	QJsonObject::const_iterator it;
	for (it = obj.constBegin(); it != obj.constEnd(); ++it)
	{
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

QString jsonArrayToHtml(const QJsonArray& array)
{
	QString html;
	html += "<dl>";

	size_t filterNum = 0;
	for (const auto& filter : array)
	{
		const auto type = filter.toObject()["type"].toVariant().toString();
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
		
		if (filterNum < array.size() - 1)
			html += "<hr>";
		
		++filterNum;
	}

	html += "</dl>";
	return html;
}

void FilterEditorWidget::captureFiltersApplied(const QJsonArray& filters)
{
	if (filters == lastFilters && !filters.empty())
		return;

	lastFilters = filters;

	QString html = "<b>Current Filters:</b><br>";

	if (lastFilters.empty())
	{
		html += "<br>No filters being applied";
	}
	else
	{
		html += jsonArrayToHtml(lastFilters);
	}

	textEdit->setHtml(html);
}

void FilterEditorWidget::captureFiltersFailed()
{
	lastFilters = QJsonArray();
	textEdit->setHtml("<b>Failed to apply filters</b>");
}
