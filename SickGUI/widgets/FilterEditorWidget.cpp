#include "FilterEditorWidget.h"

#include <ThresholdFilterModel.h>
#include <StackBlurFilterModel.h>
#include <ResizeFilterModel.h>
#include <SubtractFilterModel.h>
#include <BilateralFilterModel.h>
#include <FrameSourceModel.h>
#include <FrameViewerModel.h>

FilterEditorWidget::FilterEditorWidget(QWidget* parent) :
    GraphicsView(parent),
    graph(registerDataModels()),
	scene(new QtNodes::DataFlowGraphicsScene(graph, this))
{

	this->setScene(scene);
}

FilterEditorWidget::~FilterEditorWidget()
{
}

std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels()
{
    auto ret = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    ret->registerModel<FrameSourceModel>("Sources");
    ret->registerModel<FrameViewerModel>("Viewers");
    ret->registerModel<ThresholdFilterModel>("Filters");
    ret->registerModel<ResizeFilterModel>("Filters");
    ret->registerModel<StackBlurFilterModel>("Filters");
    ret->registerModel<SubtractFilterModel>("Filters");
    ret->registerModel<BilateralFilterModel>("Filters");

    return ret;
}
