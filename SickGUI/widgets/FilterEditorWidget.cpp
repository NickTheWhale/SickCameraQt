#include "FilterEditorWidget.h"

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
	validateButton(new QPushButton("Validate Flow", this)),
	applyButton(new QPushButton("Apply Flow", this))
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

const bool FilterEditorWidget::validateFlow() const
{
	// what does it mean to be valid?
	//   1. ONE plc start node
	//   2. ONE plc end node
	//   3. plc start connected to plc end

	for (const auto& id : graph.allNodeIds())
		qDebug() << id;
}

const bool FilterEditorWidget::applyFlow() const
{
	return false;
}

std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels()
{
	auto ret = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
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
	catch (...)
	{
	    qWarning() << "Failed to load filters, most likely due to invalid file format";
	}
}
