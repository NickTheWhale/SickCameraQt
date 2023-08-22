#pragma once
#include <qwidget.h>

#include <GraphicsView.hpp>
#include <NodeDelegateModelRegistry.hpp>
#include <FilterFlowGraphModel.h>
#include <FilterFlowGraphicsScene.h>
#include <FilterManager.h>

#include <qpushbutton.h>

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels();

class FilterEditorWidget : public QtNodes::GraphicsView
{
	Q_OBJECT

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
	QtNodes::FilterFlowGraphModel graph;
	QtNodes::FilterFlowGraphicsScene* scene = nullptr;
	std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry;

	FilterManager filterManager;

	//QPushButton* validateButton = nullptr;
	QPushButton* applyButton = nullptr;

	void setButtonGeometry();
	const bool validatePlcFlags(QtNodes::NodeId& startNodeId, QtNodes::NodeId& endNodeId) const;
	const void applyFlow();
};

