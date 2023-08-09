#pragma once
#include <qwidget.h>

#include <GraphicsView.hpp>
#include <NodeDelegateModelRegistry.hpp>
#include <DataFlowGraphModel.hpp>
#include <DataFlowGraphicsScene.hpp>

#include <PassthroughFilterModel.h>

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels();

class FilterEditorWidget : public QtNodes::GraphicsView
{
public:
	FilterEditorWidget(QWidget* parent = nullptr);
	~FilterEditorWidget();

private:
	QtNodes::DataFlowGraphModel graph;
	QtNodes::DataFlowGraphicsScene* scene = nullptr;
	std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry;
};

