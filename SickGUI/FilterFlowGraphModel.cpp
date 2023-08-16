#include "FilterFlowGraphModel.h"
#include <PlcStartModel.h>
#include <PlcEndModel.h>
#include <deque>
#include <qjsonarray.h>

FilterFlowGraphModel::FilterFlowGraphModel(std::shared_ptr<QtNodes::NodeDelegateModelRegistry> registry) :
	QtNodes::DataFlowGraphModel(registry)
{
}

QtNodes::NodeId FilterFlowGraphModel::addNode(QString const nodeType)
{
	return QtNodes::DataFlowGraphModel::addNode(nodeType);
	//if (nodeType == PlcStartModel().name())
	//{
	//	return QtNodes::InvalidNodeId;
	//}
	//else if (nodeType == PlcEndModel().name())
	//{
	//	return QtNodes::InvalidNodeId;
	//}
	//else
	//{
	//	return QtNodes::DataFlowGraphModel::addNode(nodeType);
	//}
}

bool FilterFlowGraphModel::nodesDirectConnected(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId) const
{
	if (!nodeExists(startNodeId) || !nodeExists(endNodeId)) {
		return false;
	}

	std::unordered_set<QtNodes::ConnectionId> connections = allConnectionIds(startNodeId);

	for (const QtNodes::ConnectionId& connection : connections)
	{
		QtNodes::NodeId connectedNodeId = (connection.inNodeId == startNodeId) ? connection.outNodeId : connection.inNodeId;

		if (connectedNodeId == endNodeId) {
			return true;
		}
	}

	return false;
}

bool FilterFlowGraphModel::nodesUniquelyConnected(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId) const
{
	return nodesDirectConnected(startNodeId, endNodeId) && allConnectionIds(startNodeId).size() == 1;
}

bool FilterFlowGraphModel::nodesConnected(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId) const
{
	// breadth-first traversal

	if (!nodeExists(startNodeId) || !nodeExists(endNodeId)) {
		return false;
	}

	std::unordered_set<QtNodes::NodeId> visitedNodes;

	std::deque<QtNodes::NodeId> nodesToVisit;
	nodesToVisit.push_back(startNodeId);

	while (!nodesToVisit.empty())
	{
		QtNodes::NodeId currentNodeId = nodesToVisit.front();
		nodesToVisit.pop_front();

		if (currentNodeId == endNodeId)
		{
			return true;
		}

		if (visitedNodes.find(currentNodeId) != visitedNodes.end())
		{
			continue;
		}

		visitedNodes.insert(currentNodeId);

		std::unordered_set<QtNodes::ConnectionId> connections = allConnectionIds(currentNodeId);

		for (const QtNodes::ConnectionId& connection : connections)
		{
			QtNodes::NodeId nextNodeId = connection.inNodeId;
			if (nextNodeId == currentNodeId) 
			{
				nextNodeId = connection.outNodeId;
			}

			if (visitedNodes.find(nextNodeId) == visitedNodes.end())
			{
				nodesToVisit.push_back(nextNodeId);
			}
		}
	}
	return false;
}

const std::vector<QtNodes::NodeId> FilterFlowGraphModel::computeFilterChainIDs(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId)
{
	std::vector<QtNodes::NodeId> chain;
	if (!nodeExists(startNodeId) || !nodeExists(endNodeId))
		return chain;

	QtNodes::NodeId currentNode = endNodeId;

	while (currentNode != startNodeId)
	{
		auto getName = [this](QtNodes::NodeId id) { return nodeData(id, QtNodes::NodeRole::Type).toString(); };
		auto name = getName(currentNode);
		auto connections = this->connections(currentNode, QtNodes::PortType::In, QtNodes::PortIndex(0));
		QJsonObject internalJson = nodeData(currentNode, QtNodes::NodeRole::InternalData).toJsonObject()["internal-data"].toObject();
		bool isFilter = internalJson.contains("filter");

		if (connections.size() != 1 || (!isFilter && currentNode != endNodeId))
		{
			return std::vector<QtNodes::NodeId>();
		}

		chain.push_back(currentNode);

		QtNodes::NodeId nextNode = (connections.begin()->inNodeId == currentNode) ? connections.begin()->outNodeId : connections.begin()->inNodeId;


		currentNode = nextNode;
	}
	
	chain.push_back(startNodeId);

	std::reverse(chain.begin(), chain.end());
	return chain;
}

const QJsonArray FilterFlowGraphModel::computeFilterJson(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId)
{
	QJsonArray json;

	auto ids = computeFilterChainIDs(startNodeId, endNodeId);
	for (const auto& id : ids)
	{
		QJsonObject filter = saveNode(id)["internal-data"].toObject()["filter"].toObject();
		if (!filter.empty())
			json.append(filter);
	}

	return json;
}
