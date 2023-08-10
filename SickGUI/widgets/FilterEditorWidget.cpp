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
