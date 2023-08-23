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

FilterEditorWidget::FilterEditorWidget(QWidget* parent) :
	GraphicsView(parent),
	graph(registerDataModels()),
	scene(new QtNodes::FilterFlowGraphicsScene(graph, this)),
	applyButton(new QPushButton("Apply Flow To PLC Stream", this)),
	scrollAreaButton(new QPushButton("-", this)),
	scrollAreaLabel(new QLabel(this)),
	scrollArea(new QScrollArea(this))
{
	validStyle.loadJsonFile(":/SickGUI/ValidStyle.json");
	invalidStyle.loadJsonFile(":/SickGUI/InvalidStyle.json");

	connect(applyButton, &QPushButton::pressed, this, &FilterEditorWidget::validateAndApplyFlow);
	connect(scrollAreaButton, &QPushButton::pressed, this, [=]()
		{
			scrollArea->setVisible(!scrollArea->isVisible());
			if (scrollArea->isVisible())
				scrollAreaButton->move(QPoint(scrollArea->width() + 5, 5));
			else
				scrollAreaButton->move(QPoint(5, 5));
		});

	scrollArea->setWidget(scrollAreaLabel);

	this->setScene(scene);
	setButtonGeometry();
}

FilterEditorWidget::~FilterEditorWidget()
{
}

void FilterEditorWidget::resizeEvent(QResizeEvent* event)
{
	setButtonGeometry();
	setScrollAreaGeometry();
	GraphicsView::resizeEvent(event);
}

void FilterEditorWidget::setButtonGeometry()
{
	const QPoint bottomRight(size().width(), size().height());
	const QPoint pad(5, 5);
	const QPoint applyButtonSize(applyButton->size().width(), applyButton->size().height());
	const QPoint applyPos = bottomRight - pad - applyButtonSize;
	applyButton->move(applyPos);
}

void FilterEditorWidget::setScrollAreaGeometry()
{
	const QPoint pad(5, 5);
	scrollArea->move(pad);
	scrollArea->resize((size().width() / 5) - pad.x(), size().height() - pad.y() - pad.y());
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

void FilterEditorWidget::validateAndApplyFlow()
{
	// 1. do we have exactly 2 plc flags?
	//     yes - go to 2
	//     no - apply invalid style to all plc flags
	// 2. are the plc flags uniquely connected?
	//     yes - emit an empty filter chain, apply valid style
	//     no - go to 3
	// 3. are the flags connected through a single path?
	//     yes - emit the path filters, apply valid style to path
	//     no - go to 4
	// 4. are the flags connected thourgh a path with branches?
	//     yes - apply invalid style to all nodes in path
	//     no - go to 5
	// 5. are the flags connected at all?

	const auto plcIds = getPlcIds();
	const auto startIdCount = plcIds.first.size();
	const auto endIdCount = plcIds.second.size();
	if (startIdCount != 1 || endIdCount != 1)
	{
		qDebug() << __FUNCTION__ << "must have exactly 1 start and end flag. You have" << startIdCount << "start flag(s) and" << endIdCount << "end flag(s)";

		std::unordered_set<QtNodes::NodeId> idsToColor;
		for (const auto& plcId : plcIds.first)
		{
			idsToColor.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToColor.insert(id);
		}

		for (const auto& plcId : plcIds.second)
		{
			idsToColor.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToColor.insert(id);
		}

		updateNodeColors(invalidStyle, idsToColor);

		return;
	}

	const auto plcStartNode = plcIds.first.front();
	const auto plcEndNode = plcIds.second.front();

	if (!graph.nodesConnected(plcStartNode, plcEndNode))
	{
		qDebug() << __FUNCTION__ << "you have 1 start and end flag, but they are not connected";

		std::unordered_set<QtNodes::NodeId> idsToColor;
		for (const auto& plcId : plcIds.first)
		{
			idsToColor.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToColor.insert(id);
		}

		for (const auto& plcId : plcIds.second)
		{
			idsToColor.insert(plcId);
			const auto plcBranchIds = graph.branchingConnections(plcId);
			for (const auto& id : plcBranchIds)
				idsToColor.insert(id);
		}

		updateNodeColors(invalidStyle, idsToColor);

		return;
	}

	if (graph.nodesUniquelyConnected(plcStartNode, plcEndNode))
	{
		qDebug() << __FUNCTION__ << "plc nodes are uniquely connected";
		updateNodeColors(validStyle, plcIds.first);
		updateNodeColors(validStyle, plcIds.second);

		emit updatedFilters(QJsonArray());

		return;
	}

	const auto nonBranchingIds = graph.nonBranchingConnections(plcStartNode, plcEndNode);
	if (!nonBranchingIds.empty())
	{
		qDebug() << __FUNCTION__ << "plc nodes are connected through a non-branching path";
		updateNodeColors(validStyle, nonBranchingIds);

		QJsonArray json;
		for (const auto& id : nonBranchingIds)
		{
			QJsonObject filter = graph.saveNode(id)["internal-data"].toObject()["filter"].toObject();
			if (!filter.empty())
				json.append(filter);
		}

		emit updatedFilters(json);

		return;
	}

	const auto branchingIds = graph.branchingConnections(plcStartNode, plcEndNode);
	if (!branchingIds.empty())
	{
		qDebug() << __FUNCTION__ << "plc nodes are connected through a branching path";
		updateNodeColors(invalidStyle, branchingIds);

		return;
	}

	qDebug() << __FUNCTION__ << "got through all the ' if's '";
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
}

void FilterEditorWidget::captureFiltersApplied(const QJsonArray& filters)
{
	
	QString filterString;

	for (const auto& filterRef : filters)
	{
		QJsonObject filterJson = filterRef.toObject();
		QString type = filterJson["type"].toString();
		filterString += type;
		filterString += "\n";
	}

	qDebug() << __FUNCTION__ << filterString;

	scrollAreaLabel->setText(filterString);
	scrollAreaLabel->adjustSize();
}
