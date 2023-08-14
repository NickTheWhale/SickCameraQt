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
#include <FrameSourceModel.h>
#include <FrameViewerModel.h>

#include <qlayout.h>
#include <qdebug.h>

FilterEditorWidget::FilterEditorWidget(QWidget* parent) :
	GraphicsView(parent),
	graph(registerDataModels()),
	scene(new QtNodes::DataFlowGraphicsScene(graph, this)),
	validateButton(new QPushButton("Validate PLC Flow", this)),
	applyButton(new QPushButton("Apply Flow To PLC", this))
{
	connect(validateButton, &QPushButton::pressed, this, &FilterEditorWidget::validateFlow);
	connect(applyButton, &QPushButton::pressed, this, &FilterEditorWidget::applyFlow);

	this->setScene(scene);
	setButtonGeometry();
}

FilterEditorWidget::~FilterEditorWidget()
{
}

void FilterEditorWidget::resizeEvent(QResizeEvent* event)
{
	setButtonGeometry();
	GraphicsView::resizeEvent(event);
}

void FilterEditorWidget::setButtonGeometry()
{
	const QPoint bottomRight(size().width(), size().height());
	const QPoint pad(5, 5);
	const QPoint padx(5, 0);
	const QPoint applyButtonSize(applyButton->size().width(), applyButton->size().height());
	const QPoint validateButtonSize(validateButton->size().width(), validateButton->size().height());
	const QPoint validateButtonSizeX(validateButtonSize.x(), 0);
	const QPoint applyPos = bottomRight - pad - applyButtonSize;
	const QPoint validatePos = applyPos - validateButtonSizeX - padx;

	validateButton->move(validatePos);
	applyButton->move(applyPos);
}

const bool FilterEditorWidget::validateFlow()
{
	// what does it mean to be valid?
	//   1. ONE plc start node
	//   2. ONE plc end node
	//   3. plc start connected to plc end
	//      a. unique connection
	//      b. only through "filterable" nodes

	QtNodes::NodeId startId;
	QtNodes::NodeId endId;
	if (!validatePlcFlags(startId, endId))
	{
		qDebug() << "invalid plc flags";
		return false;
	}

	auto getName = [this](QtNodes::NodeId id) { return graph.nodeData(id, QtNodes::NodeRole::Type).toString(); };
	auto chain = graph.computeFilterChain(startId, endId);

	QJsonObject filters;
	for (const auto& id : chain)
	{
		auto name = getName(id);
		filters[name] = graph.saveNode(id);
	}

	qDebug() << filters;

	return true;
}

const bool FilterEditorWidget::validatePlcFlags(QtNodes::NodeId& startNodeId, QtNodes::NodeId& endNodeId) const
{
	int startCount = 0;
	int endCount = 0;
	QString startName = PlcStartModel().name();
	QString endName = PlcEndModel().name();
	for (const auto& id : graph.allNodeIds())
	{
		auto name = graph.nodeData(id, QtNodes::NodeRole::Type);
		if (name == startName)
		{
			startNodeId = id;
			++startCount;
		}
		else if (name == endName)
		{
			endNodeId = id;
			++endCount;
		}

		if (startCount > 1 || endCount > 1)
		{
			break;
		}
	}

	if (startCount != 1 || endCount != 1)
	{
		return false;
	}

	return true;
}


const bool FilterEditorWidget::applyFlow() const
{
	return false;
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
	ret->registerModel<FrameSourceModel>("Sources");
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
		qWarning() << "Filaed to load node." << e.what();
	}
	catch (...)
	{
		qWarning() << "Failed to load node(s). This may be due to invalid json format or outdated flow file";
	}
}
