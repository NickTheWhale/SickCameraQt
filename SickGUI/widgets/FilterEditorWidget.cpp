#include "FilterEditorWidget.h"

#include <PassthroughFilterModel.h>
#include <VoxelFilterModel.h>
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
    ret->registerModel<PassthroughFilterModel>("Filters");
    ret->registerModel<VoxelFilterModel>("Filters");
    return ret;
}
