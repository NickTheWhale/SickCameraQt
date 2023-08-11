#pragma once
#include <qwidget.h>

#include <GraphicsView.hpp>
#include <NodeDelegateModelRegistry.hpp>
#include <DataFlowGraphModel.hpp>
#include <DataFlowGraphicsScene.hpp>

#include <qpushbutton.h>

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels();

class FilterEditorWidget : public QtNodes::GraphicsView
{
public slots:
	void save();
	void load();

public:
	FilterEditorWidget(QWidget* parent = nullptr);
	~FilterEditorWidget();

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	QtNodes::DataFlowGraphModel graph;
	QtNodes::DataFlowGraphicsScene* scene = nullptr;
	std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry;

	QPushButton* validateButton = nullptr;
	QPushButton* applyButton = nullptr;

	void setButtonGeometry();
	const bool validateFlow() const;
	const bool applyFlow() const;
};

