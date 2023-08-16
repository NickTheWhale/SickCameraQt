#pragma once
#include <DataFlowGraphModel.hpp>

#include <vector>
#include <qjsonarray.h>

class FilterFlowGraphModel : public QtNodes::DataFlowGraphModel
{
	Q_OBJECT

public:
	FilterFlowGraphModel(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry);

	QtNodes::NodeId addNode(QString const nodeType) override;

	bool nodesDirectConnected(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId) const;
	bool nodesUniquelyConnected(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId) const;
	bool nodesConnected(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId) const;
	const std::vector<QtNodes::NodeId> computeFilterChainIDs(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId);
	const QJsonArray computeFilterJson(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId);
};

