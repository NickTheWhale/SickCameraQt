#include "FilterFlowGraphModel.h"
#include <PlcStartModel.h>
#include <PlcEndModel.h>
#include <deque>    
#include <stack>

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
			for (const auto& cid : visitedNodes)
				qDebug() << this->nodeData(cid, QtNodes::NodeRole::Type);

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
			if (nextNodeId == currentNodeId) {
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

//const std::vector<QtNodes::NodeId> FilterFlowGraphModel::computeFilterChain(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId)
//{
//    std::vector<QtNodes::NodeId> emptyPathNodes;
//    if (!nodeExists(startNodeId) || !nodeExists(endNodeId)) {
//        return emptyPathNodes;
//    }
//
//    std::unordered_set<QtNodes::NodeId> visitedNodes;
//    std::deque<QtNodes::NodeId> nodesToVisit;
//    nodesToVisit.push_back(startNodeId);
//
//    while (!nodesToVisit.empty())
//    {
//        QtNodes::NodeId currentNodeId = nodesToVisit.front();
//        nodesToVisit.pop_front();
//
//        if (visitedNodes.find(currentNodeId) != visitedNodes.end())
//        {
//            continue;
//        }
//
//        visitedNodes.insert(currentNodeId);
//
//        if (currentNodeId == endNodeId)
//        {
//            std::vector<QtNodes::NodeId> pathNodes(visitedNodes.begin(), visitedNodes.end());
//            bool isValidPath = true;
//            for (const QtNodes::NodeId& nodeId : pathNodes)
//            {
//                auto name = this->nodeData(nodeId, QtNodes::NodeRole::Type).toString();
//                QJsonObject nodeData = this->nodeData(nodeId, QtNodes::NodeRole::InternalData).toJsonObject();
//                QJsonObject internalData = nodeData["internal-data"].toObject();
//                const bool filterable = internalData["filterable"].toBool(false);
//
//                if (!filterable)
//                { 
//                    if (name != PlcStartModel().name() && name != PlcEndModel().name())
//                    {
//                        isValidPath = false;
//                        break;
//                    }
//                }
//            }
//
//            if (isValidPath)
//            {
//                return pruneFilterChain(pathNodes);
//            }
//        }
//
//        std::unordered_set<QtNodes::ConnectionId> connections = allConnectionIds(currentNodeId);
//
//        for (const QtNodes::ConnectionId& connection : connections)
//        {
//            QtNodes::NodeId nextNodeId = connection.inNodeId;
//            if (nextNodeId == currentNodeId) {
//                nextNodeId = connection.outNodeId;
//            }
//
//            if (visitedNodes.find(nextNodeId) == visitedNodes.end())
//            {
//                nodesToVisit.push_back(nextNodeId);
//            }
//        }
//    }
//    return emptyPathNodes;
//}
//
//std::vector<QtNodes::NodeId> FilterFlowGraphModel::pruneFilterChain(std::vector<QtNodes::NodeId>& chain)
//{
//    std::vector<QtNodes::NodeId> pruned;
//
//    for (auto rit = chain.rbegin(); rit != chain.rend(); ++rit) 
//    {
//        auto getName = [this](QtNodes::NodeId id) { return nodeData(id, QtNodes::NodeRole::Type).toString(); };
//        auto connections = allConnectionIds(*rit);
//        auto name = getName(*rit);
//        qDebug() << name;
//        for (const auto& cid : connections)
//        {
//            qDebug() << "in:" << getName(cid.inNodeId) << "out:" << getName(cid.outNodeId);
//        }
//    }
//
//    return pruned;
//}

const std::vector<QtNodes::NodeId> FilterFlowGraphModel::computeFilterChain(QtNodes::NodeId startNodeId, QtNodes::NodeId endNodeId)
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
		bool filterable = nodeData(currentNode, QtNodes::NodeRole::InternalData).toJsonObject()["internal-data"].toObject()["filterable"].toBool(false);

		if (connections.size() != 1 || (!filterable && currentNode != endNodeId))
		{
				qDebug() << "early return";
				return std::vector<QtNodes::NodeId>();
		}

		chain.push_back(currentNode);

		QtNodes::NodeId nextNode = (connections.begin()->inNodeId == currentNode) ? connections.begin()->outNodeId : connections.begin()->inNodeId;


		currentNode = nextNode;
	}

	return chain;
}
