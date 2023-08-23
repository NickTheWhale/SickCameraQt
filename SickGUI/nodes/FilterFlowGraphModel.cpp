#include "FilterFlowGraphModel.h"
#include "ConnectionIdHash.hpp"

#include <QJsonArray>

#include <stdexcept>
#include <deque>

namespace QtNodes {

	FilterFlowGraphModel::FilterFlowGraphModel(std::shared_ptr<NodeDelegateModelRegistry> registry)
		: _registry(std::move(registry))
		, _nextNodeId{ 0 }
	{}

	std::unordered_set<NodeId> FilterFlowGraphModel::allNodeIds() const
	{
		std::unordered_set<NodeId> nodeIds;
		for_each(_models.begin(), _models.end(), [&nodeIds](auto const& p) { nodeIds.insert(p.first); });

		return nodeIds;
	}

	std::unordered_set<ConnectionId> FilterFlowGraphModel::allConnectionIds(NodeId const nodeId) const
	{
		std::unordered_set<ConnectionId> result;

		std::copy_if(_connectivity.begin(),
			_connectivity.end(),
			std::inserter(result, std::end(result)),
			[&nodeId](ConnectionId const& cid) {
				return cid.inNodeId == nodeId || cid.outNodeId == nodeId;
			});

		return result;
	}

	std::unordered_set<ConnectionId> FilterFlowGraphModel::connections(NodeId nodeId,
		PortType portType,
		PortIndex portIndex) const
	{
		std::unordered_set<ConnectionId> result;

		std::copy_if(_connectivity.begin(),
			_connectivity.end(),
			std::inserter(result, std::end(result)),
			[&portType, &portIndex, &nodeId](ConnectionId const& cid) {
				return (getNodeId(portType, cid) == nodeId
					&& getPortIndex(portType, cid) == portIndex);
			});

		return result;
	}

	bool FilterFlowGraphModel::connectionExists(ConnectionId const connectionId) const
	{
		return (_connectivity.find(connectionId) != _connectivity.end());
	}

	NodeId FilterFlowGraphModel::addNode(QString const nodeType)
	{
		std::unique_ptr<NodeDelegateModel> model = _registry->create(nodeType);

		if (model) {
			NodeId newId = newNodeId();

			connect(model.get(),
				&NodeDelegateModel::dataUpdated,
				[newId, this](PortIndex const portIndex) {
					onOutPortDataUpdated(newId, portIndex);
				});

			connect(model.get(),
				&NodeDelegateModel::portsAboutToBeDeleted,
				this,
				[newId, this](PortType const portType, PortIndex const first, PortIndex const last) {
					portsAboutToBeDeleted(newId, portType, first, last);
				});

			connect(model.get(),
				&NodeDelegateModel::portsDeleted,
				this,
				&FilterFlowGraphModel::portsDeleted);

			connect(model.get(),
				&NodeDelegateModel::portsAboutToBeInserted,
				this,
				[newId, this](PortType const portType, PortIndex const first, PortIndex const last) {
					portsAboutToBeInserted(newId, portType, first, last);
				});

			connect(model.get(),
				&NodeDelegateModel::portsInserted,
				this,
				&FilterFlowGraphModel::portsInserted);

			_models[newId] = std::move(model);

			Q_EMIT nodeCreated(newId);

			return newId;
		}

		return InvalidNodeId;
	}

	bool FilterFlowGraphModel::connectionPossible(ConnectionId const connectionId) const
	{
		auto getDataType = [&](PortType const portType) {
			return portData(getNodeId(portType, connectionId),
				portType,
				getPortIndex(portType, connectionId),
				PortRole::DataType)
				.value<NodeDataType>();
		};

		auto portVacant = [&](PortType const portType) {
			NodeId const nodeId = getNodeId(portType, connectionId);
			PortIndex const portIndex = getPortIndex(portType, connectionId);
			auto const connected = connections(nodeId, portType, portIndex);

			auto policy = portData(nodeId, portType, portIndex, PortRole::ConnectionPolicyRole)
				.value<ConnectionPolicy>();

			return connected.empty() || (policy == ConnectionPolicy::Many);
		};

		return getDataType(PortType::Out).id == getDataType(PortType::In).id
			&& portVacant(PortType::Out) && portVacant(PortType::In);
	}

	void FilterFlowGraphModel::addConnection(ConnectionId const connectionId)
	{
		_connectivity.insert(connectionId);

		sendConnectionCreation(connectionId);

		QVariant const portDataToPropagate = portData(connectionId.outNodeId,
			PortType::Out,
			connectionId.outPortIndex,
			PortRole::Data);

		setPortData(connectionId.inNodeId,
			PortType::In,
			connectionId.inPortIndex,
			portDataToPropagate,
			PortRole::Data);
	}

	void FilterFlowGraphModel::sendConnectionCreation(ConnectionId const connectionId)
	{
		Q_EMIT connectionCreated(connectionId);

		auto iti = _models.find(connectionId.inNodeId);
		auto ito = _models.find(connectionId.outNodeId);
		if (iti != _models.end() && ito != _models.end()) {
			auto& modeli = iti->second;
			auto& modelo = ito->second;
			modeli->inputConnectionCreated(connectionId);
			modelo->outputConnectionCreated(connectionId);
		}
	}

	void FilterFlowGraphModel::sendConnectionDeletion(ConnectionId const connectionId)
	{
		Q_EMIT connectionDeleted(connectionId);

		auto iti = _models.find(connectionId.inNodeId);
		auto ito = _models.find(connectionId.outNodeId);
		if (iti != _models.end() && ito != _models.end()) {
			auto& modeli = iti->second;
			auto& modelo = ito->second;
			modeli->inputConnectionDeleted(connectionId);
			modelo->outputConnectionDeleted(connectionId);
		}
	}

	bool FilterFlowGraphModel::nodeExists(NodeId const nodeId) const
	{
		return (_models.find(nodeId) != _models.end());
	}

	bool FilterFlowGraphModel::nodesDirectlyConnected(const NodeId startNodeId, const NodeId endNodeId) const
	{
		if (!nodeExists(startNodeId) || !nodeExists(endNodeId)) {
			return false;
		}

		std::unordered_set<ConnectionId> connections = allConnectionIds(startNodeId);

		for (const ConnectionId& connection : connections)
		{
			NodeId connectedNodeId = (connection.inNodeId == startNodeId) ? connection.outNodeId : connection.inNodeId;

			if (connectedNodeId == endNodeId) {
				return true;
			}
		}

		return false;
	}

	bool FilterFlowGraphModel::nodesUniquelyConnected(const NodeId startNodeId, const NodeId endNodeId) const
	{
		return nodesDirectlyConnected(startNodeId, endNodeId) && allConnectionIds(startNodeId).size() == 1;
	}

	bool FilterFlowGraphModel::nodesConnected(const NodeId startNodeId, const NodeId endNodeId) const
	{
		// breadth-first traversal

		if (!nodeExists(startNodeId) || !nodeExists(endNodeId)) {
			return false;
		}

		std::unordered_set<NodeId> visitedNodes;

		std::deque<NodeId> nodesToVisit;
		nodesToVisit.push_back(startNodeId);

		while (!nodesToVisit.empty())
		{
			NodeId currentNodeId = nodesToVisit.front();
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

			std::unordered_set<ConnectionId> connections = allConnectionIds(currentNodeId);

			for (const ConnectionId& connection : connections)
			{
				NodeId nextNodeId = connection.inNodeId;
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

	const std::vector<NodeId> FilterFlowGraphModel::nonBranchingConnections(const NodeId startNodeId, const NodeId endNodeId) const
	{
		std::vector<NodeId> chain;
		if (!nodeExists(startNodeId) || !nodeExists(endNodeId))
			return chain;

		NodeId currentNode = endNodeId;

		while (currentNode != startNodeId)
		{
			const auto inConnections = this->connections(currentNode, PortType::In, PortIndex(0));
			const auto outConnections = this->connections(currentNode, PortType::Out, PortIndex(0));
			const QJsonObject internalJson = nodeData(currentNode, NodeRole::InternalData).toJsonObject()["internal-data"].toObject();
			const bool isFilter = internalJson.contains("filter");

			if (inConnections.size() != 1 || (outConnections.size() != 1 && currentNode != endNodeId) || (!isFilter && currentNode != endNodeId))
			{
				return std::vector<NodeId>();
			}

			chain.push_back(currentNode);

			NodeId nextNode = (inConnections.begin()->inNodeId == currentNode) ? inConnections.begin()->outNodeId : inConnections.begin()->inNodeId;

			currentNode = nextNode;
		}

		chain.push_back(startNodeId);

		std::reverse(chain.begin(), chain.end());
		return chain;
	}

	const std::unordered_set<NodeId> FilterFlowGraphModel::branchingConnections(const NodeId startNodeId, const NodeId endNodeId) const
	{
		// breadth-first traversal

		if (!nodeExists(startNodeId) || !nodeExists(endNodeId)) {
			return std::unordered_set<NodeId>();
		}

		std::unordered_set<NodeId> visitedNodes;
		std::deque<NodeId> nodesToVisit;
		nodesToVisit.push_back(startNodeId);

		while (!nodesToVisit.empty())
		{
			NodeId currentNodeId = nodesToVisit.front();
			nodesToVisit.pop_front();

			if (currentNodeId == endNodeId)
			{
				return visitedNodes;
			}

			if (visitedNodes.find(currentNodeId) != visitedNodes.end())
			{
				continue;
			}

			visitedNodes.insert(currentNodeId);

			std::unordered_set<ConnectionId> connections = allConnectionIds(currentNodeId);

			for (const ConnectionId& connection : connections)
			{
				NodeId nextNodeId = connection.inNodeId;
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
		return std::unordered_set<NodeId>();
	}

	const std::unordered_set<NodeId> FilterFlowGraphModel::branchingConnections(const NodeId inputNodeId) const
	{
		std::unordered_set<NodeId> connectedNodes;

		// Check if the input node exists
		if (!nodeExists(inputNodeId)) {
			return connectedNodes;
		}

		std::unordered_set<NodeId> visitedNodes;

		std::deque<NodeId> nodesToVisit;
		nodesToVisit.push_back(inputNodeId);

		while (!nodesToVisit.empty()) {
			NodeId currentNodeId = nodesToVisit.front();
			nodesToVisit.pop_front();

			// Skip already visited nodes
			if (visitedNodes.find(currentNodeId) != visitedNodes.end()) {
				continue;
			}

			visitedNodes.insert(currentNodeId);

			std::unordered_set<ConnectionId> connections = allConnectionIds(currentNodeId);

			for (const ConnectionId& connection : connections) {
				NodeId connectedNodeId = (connection.inNodeId == currentNodeId) ? connection.outNodeId : connection.inNodeId;

				if (visitedNodes.find(connectedNodeId) == visitedNodes.end()) {
					connectedNodes.insert(connectedNodeId);
					nodesToVisit.push_back(connectedNodeId);
				}
			}
		}

		return connectedNodes;
	}

	QVariant FilterFlowGraphModel::nodeData(NodeId nodeId, NodeRole role) const
	{
		QVariant result;

		auto it = _models.find(nodeId);
		if (it == _models.end())
			return result;

		auto& model = it->second;

		switch (role) {
		case NodeRole::Type:
			result = model->name();
			break;

		case NodeRole::Position:
			result = _nodeGeometryData[nodeId].pos;
			break;

		case NodeRole::Size:
			result = _nodeGeometryData[nodeId].size;
			break;

		case NodeRole::CaptionVisible:
			result = model->captionVisible();
			break;

		case NodeRole::Caption:
			result = model->caption();
			break;

		case NodeRole::Style: {
			//auto style = StyleCollection::nodeStyle();
			auto style = model->nodeStyle();
			result = style.toJson().toVariantMap();
		} break;

		case NodeRole::InternalData: {
			QJsonObject nodeJson;

			nodeJson["internal-data"] = _models.at(nodeId)->save();

			result = nodeJson.toVariantMap();
			break;
		}

		case NodeRole::InPortCount:
			result = model->nPorts(PortType::In);
			break;

		case NodeRole::OutPortCount:
			result = model->nPorts(PortType::Out);
			break;

		case NodeRole::Widget: {
			auto w = model->embeddedWidget();
			result = QVariant::fromValue(w);
		} break;
		}

		return result;
	}

	NodeFlags FilterFlowGraphModel::nodeFlags(NodeId nodeId) const
	{
		auto it = _models.find(nodeId);

		if (it != _models.end() && it->second->resizable())
			return NodeFlag::Resizable;

		return NodeFlag::NoFlags;
	}

	bool FilterFlowGraphModel::setNodeData(NodeId nodeId, NodeRole role, QVariant value)
	{
		bool result = false;

		switch (role) {
		case NodeRole::Type:
			break;
		case NodeRole::Position: {
			_nodeGeometryData[nodeId].pos = value.value<QPointF>();

			Q_EMIT nodePositionUpdated(nodeId);

			result = true;
		} break;

		case NodeRole::Size: {
			_nodeGeometryData[nodeId].size = value.value<QSize>();
			result = true;
		} break;

		case NodeRole::CaptionVisible:
			break;

		case NodeRole::Caption:
			break;

		case NodeRole::Style: {
			auto it = _models.find(nodeId);
			if (it != _models.end())
			{
				auto& model = it->second;
				const auto styleJson = value.value<QJsonObject>();
				const auto style = NodeStyle(styleJson);
				model->setNodeStyle(style);
				result = true;
			}
		} break;

		case NodeRole::InternalData:
			break;

		case NodeRole::InPortCount:
			break;

		case NodeRole::OutPortCount:
			break;

		case NodeRole::Widget:
			break;
		}

		return result;
	}

	QVariant FilterFlowGraphModel::portData(NodeId nodeId,
		PortType portType,
		PortIndex portIndex,
		PortRole role) const
	{
		QVariant result;

		auto it = _models.find(nodeId);
		if (it == _models.end())
			return result;

		auto& model = it->second;

		switch (role) {
		case PortRole::Data:
			if (portType == PortType::Out)
				result = QVariant::fromValue(model->outData(portIndex));
			break;

		case PortRole::DataType:
			result = QVariant::fromValue(model->dataType(portType, portIndex));
			break;

		case PortRole::ConnectionPolicyRole:
			result = QVariant::fromValue(model->portConnectionPolicy(portType, portIndex));
			break;

		case PortRole::CaptionVisible:
			result = model->portCaptionVisible(portType, portIndex);
			break;

		case PortRole::Caption:
			result = model->portCaption(portType, portIndex);

			break;
		}

		return result;
	}

	bool FilterFlowGraphModel::setPortData(
		NodeId nodeId, PortType portType, PortIndex portIndex, QVariant const& value, PortRole role)
	{
		Q_UNUSED(nodeId);

		QVariant result;

		auto it = _models.find(nodeId);
		if (it == _models.end())
			return false;

		auto& model = it->second;

		switch (role) {
		case PortRole::Data:
			if (portType == PortType::In) {
				model->setInData(value.value<std::shared_ptr<NodeData>>(), portIndex);

				// Triggers repainting on the scene.
				Q_EMIT inPortDataWasSet(nodeId, portType, portIndex);
			}
			break;

		default:
			break;
		}

		return false;
	}

	bool FilterFlowGraphModel::deleteConnection(ConnectionId const connectionId)
	{
		bool disconnected = false;

		auto it = _connectivity.find(connectionId);

		if (it != _connectivity.end()) {
			disconnected = true;

			_connectivity.erase(it);
		}

		if (disconnected) {
			sendConnectionDeletion(connectionId);

			propagateEmptyDataTo(getNodeId(PortType::In, connectionId),
				getPortIndex(PortType::In, connectionId));
		}

		return disconnected;
	}

	bool FilterFlowGraphModel::deleteNode(NodeId const nodeId)
	{
		// Delete connections to this node first.
		auto connectionIds = allConnectionIds(nodeId);
		for (auto& cId : connectionIds) {
			deleteConnection(cId);
		}

		_nodeGeometryData.erase(nodeId);
		_models.erase(nodeId);

		Q_EMIT nodeDeleted(nodeId);

		return true;
	}

	QJsonObject FilterFlowGraphModel::saveNode(NodeId const nodeId) const
	{
		QJsonObject nodeJson;

		nodeJson["id"] = static_cast<qint64>(nodeId);

		nodeJson["internal-data"] = _models.at(nodeId)->save();

		{
			QPointF const pos = nodeData(nodeId, NodeRole::Position).value<QPointF>();

			QJsonObject posJson;
			posJson["x"] = pos.x();
			posJson["y"] = pos.y();
			nodeJson["position"] = posJson;
		}

		return nodeJson;
	}

	QJsonObject FilterFlowGraphModel::save() const
	{
		QJsonObject sceneJson;

		QJsonArray nodesJsonArray;
		for (auto const nodeId : allNodeIds()) {
			nodesJsonArray.append(saveNode(nodeId));
		}
		sceneJson["nodes"] = nodesJsonArray;

		QJsonArray connJsonArray;
		for (auto const& cid : _connectivity) {
			connJsonArray.append(toJson(cid));
		}
		sceneJson["connections"] = connJsonArray;

		return sceneJson;
	}

	void FilterFlowGraphModel::loadNode(QJsonObject const& nodeJson)
	{
		// Possibility of the id clash when reading it from json and not generating a
		// new value.
		// 1. When restoring a scene from a file.
		// Conflict is not possible because the scene must be cleared by the time of
		// loading.
		// 2. When undoing the deletion command.  Conflict is not possible
		// because all the new ids were created past the removed nodes.
		NodeId restoredNodeId = nodeJson["id"].toInt();

		_nextNodeId = std::max(_nextNodeId, restoredNodeId + 1);

		QJsonObject const internalDataJson = nodeJson["internal-data"].toObject();

		QString delegateModelName = internalDataJson["model-name"].toString();

		std::unique_ptr<NodeDelegateModel> model = _registry->create(delegateModelName);

		if (model) {
			connect(model.get(),
				&NodeDelegateModel::dataUpdated,
				[restoredNodeId, this](PortIndex const portIndex) {
					onOutPortDataUpdated(restoredNodeId, portIndex);
				});

			_models[restoredNodeId] = std::move(model);

			Q_EMIT nodeCreated(restoredNodeId);

			QJsonObject posJson = nodeJson["position"].toObject();
			QPointF const pos(posJson["x"].toDouble(), posJson["y"].toDouble());

			setNodeData(restoredNodeId, NodeRole::Position, pos);

			_models[restoredNodeId]->load(internalDataJson);
		}
		else {
			throw std::logic_error(std::string("No registered model with name ")
				+ delegateModelName.toLocal8Bit().data());
		}
	}

	void FilterFlowGraphModel::load(QJsonObject const& jsonDocument)
	{
		QJsonArray nodesJsonArray = jsonDocument["nodes"].toArray();

		for (QJsonValueRef nodeJson : nodesJsonArray) {
			loadNode(nodeJson.toObject());
		}

		QJsonArray connectionJsonArray = jsonDocument["connections"].toArray();

		for (QJsonValueRef connection : connectionJsonArray) {
			QJsonObject connJson = connection.toObject();

			ConnectionId connId = fromJson(connJson);

			// Restore the connection
			addConnection(connId);
		}
	}

	void FilterFlowGraphModel::onOutPortDataUpdated(NodeId const nodeId, PortIndex const portIndex)
	{
		std::unordered_set<ConnectionId> const& connected = connections(nodeId,
			PortType::Out,
			portIndex);

		QVariant const portDataToPropagate = portData(nodeId, PortType::Out, portIndex, PortRole::Data);

		for (auto const& cn : connected) {
			setPortData(cn.inNodeId, PortType::In, cn.inPortIndex, portDataToPropagate, PortRole::Data);
		}
	}

	void FilterFlowGraphModel::propagateEmptyDataTo(NodeId const nodeId, PortIndex const portIndex)
	{
		QVariant emptyData{};

		setPortData(nodeId, PortType::In, portIndex, emptyData, PortRole::Data);
	}

} // namespace QtNodes
