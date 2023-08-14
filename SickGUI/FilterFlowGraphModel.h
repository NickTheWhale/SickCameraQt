#pragma once
#include <DataFlowGraphModel.hpp>

class FilterFlowGraphModel : public QtNodes::DataFlowGraphModel
{
	Q_OBJECT

public:
	FilterFlowGraphModel(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry);

	QtNodes::NodeId addNode(QString const nodeType) override;

	bool nodesConnected(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId) const;
	const std::vector<QtNodes::NodeId> computeFilterChain(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId);
//
//private:
//	std::vector<QtNodes::NodeId> pruneFilterChain(std::vector<QtNodes::NodeId>& chain);
};

