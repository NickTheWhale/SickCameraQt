#pragma once
#include <qwidget.h>

#include <GraphicsView.hpp>
#include <NodeDelegateModelRegistry.hpp>
#include <FilterFlowGraphModel.h>
#include <FilterFlowGraphicsScene.h>
#include <FilterManager.h>

#include <qpushbutton.h>
#include <qscrollarea.h>
#include <qlabel.h>

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels();

class FilterEditorWidget : public QtNodes::GraphicsView
{
	Q_OBJECT

public slots:
	void save();
	void load();
	void captureFiltersApplied(const QJsonArray& filters);

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

	QPushButton* applyButton = nullptr;

	QPushButton* scrollAreaButton = nullptr;
	QLabel* scrollAreaLabel = nullptr;
	QScrollArea* scrollArea = nullptr;

	QtNodes::NodeStyle validStyle;
	QtNodes::NodeStyle invalidStyle;

	void setButtonGeometry();
	void setScrollAreaGeometry();
	const std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>> getPlcIds() const;
	void validateAndApplyFlow();
	void updateNodeColors(const QtNodes::Style& style, const std::vector<QtNodes::NodeId>& ids);
	void updateNodeColors(const QtNodes::Style& style, const std::unordered_set<QtNodes::NodeId>& ids);
};

