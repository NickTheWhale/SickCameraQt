#pragma once
#include <qwidget.h>

#include <GraphicsView.hpp>
#include <NodeDelegateModelRegistry.hpp>
#include <FilterFlowGraphModel.h>
#include <DataFlowGraphicsScene.hpp>

#include <qpushbutton.h>

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels();

class FilterEditorWidget : public QtNodes::GraphicsView
{
public slots:
	void save();
	void load();

signals:
	void updatedFilters(const QJsonArray& filters);

public:
	FilterEditorWidget(QWidget* parent = nullptr);                     
	~FilterEditorWidget();

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	FilterFlowGraphModel graph;
	QtNodes::DataFlowGraphicsScene* scene = nullptr;
	std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry;

	//QPushButton* validateButton = nullptr;
	QPushButton* applyButton = nullptr;

	void setButtonGeometry();
	const bool validatePlcFlags(QtNodes::NodeId& startNodeId, QtNodes::NodeId& endNodeId) const;
	const void applyFlow();
};

