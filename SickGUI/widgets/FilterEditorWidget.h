/*****************************************************************//**
 * @file   FilterEditorWidget.h
 * @brief  Adds some filtering specific functions to QtNodes::GraphicsView. 
 * Handles verifying filter paths and displaying the current filters
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once

#include <GraphicsView.hpp>
#include <NodeDelegateModelRegistry.hpp>
#include <FilterFlowGraphModel.h>
#include <FilterFlowGraphicsScene.h>
#include <FilterManager.h>

#include <qwidget.h>
#include <qpushbutton.h>
#include <qscrollarea.h>
#include <qtextedit.h>
#include <qlabel.h>

static std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registerDataModels();

class FilterEditorWidget : public QtNodes::GraphicsView
{
	Q_OBJECT

public slots:
	void save();
	void load();
	void captureFiltersApplied(const QJsonArray& filters);
	void captureFiltersFailed();

signals:
	void updatedFilters(const QJsonArray& filters);

public:
	FilterEditorWidget(QWidget* parent = nullptr);
	~FilterEditorWidget();

protected:
	void resizeEvent(QResizeEvent* event) override;

private:
	void setButtonGeometry(const QSize size);
	void setTextEditGeometry(const QSize size);
	void setAllGeometry(const QSize size);
	void showMessage(const QString msg);

	const std::pair<std::vector<QtNodes::NodeId>, std::vector<QtNodes::NodeId>> getPlcIds() const;
	using ValidJson = bool;
	using NodeIdSet = std::unordered_set<QtNodes::NodeId>;
	using ValidateIdSet = NodeIdSet;
	using DefaultIdSet = NodeIdSet;
	using InvalidateIdSet = NodeIdSet;
	using FlowTuple = std::tuple<ValidJson, QJsonArray, ValidateIdSet, DefaultIdSet, InvalidateIdSet, QString>;
	FlowTuple computeFilterFlow();

	void applyFilters();
	void updateAllColors(const ValidateIdSet& validateSet, const DefaultIdSet& defaultSet, const InvalidateIdSet& invalidateSet);
	void updateAllColors();
	void updateNodeColors(const QtNodes::Style& style, const std::vector<QtNodes::NodeId>& ids);
	void updateNodeColors(const QtNodes::Style& style, const std::unordered_set<QtNodes::NodeId>& ids);

private:
	QtNodes::FilterFlowGraphModel graph;
	QtNodes::FilterFlowGraphicsScene* scene = nullptr;
	std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry;

	FilterManager filterManager;

	QPushButton* applyButton = nullptr;

	QPushButton* textEditButton = nullptr;
	QTextEdit* textEdit = nullptr;
	bool textEditVisible = true;

	QJsonArray lastFilters;

	bool blockAutomaticColorUpdates = false;

	QtNodes::NodeStyle validStyle;
	QtNodes::NodeStyle invalidStyle;
	QtNodes::NodeStyle defaultStyle;
};

