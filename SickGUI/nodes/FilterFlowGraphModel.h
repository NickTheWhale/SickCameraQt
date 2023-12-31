/*****************************************************************//**
 * @file   FilterFlowGraphModel.h
 * @brief  QtNodes::AbstractGraphModel implementation for data flow. Contains functions used to verify filter flows.
 * 
 * @author Nicholas Loehrke
 * @date   August 2023
 *********************************************************************/

#pragma once

#include "AbstractGraphModel.hpp"
#include "ConnectionIdUtils.hpp"
#include "NodeDelegateModelRegistry.hpp"
#include "Serializable.hpp"
#include "StyleCollection.hpp"

#include "Export.hpp"

#include <QJsonObject>

namespace QtNodes {

    class NODE_EDITOR_PUBLIC FilterFlowGraphModel : public AbstractGraphModel, public Serializable
    {
        Q_OBJECT

    public:
        struct NodeGeometryData
        {
            QSize size;
            QPointF pos;
        };

    public:
        FilterFlowGraphModel(std::shared_ptr<NodeDelegateModelRegistry> registry);

        std::shared_ptr<NodeDelegateModelRegistry> dataModelRegistry() { return _registry; }

    public:
        bool nodesDirectlyConnected(const NodeId startNodeId, const NodeId endNodeId) const;
       
        bool nodesUniquelyConnected(const NodeId startNodeId, const NodeId endNodeId) const;
       
        bool nodesConnected(const NodeId startNodeId, const NodeId endNodeId) const;

        const std::vector<NodeId> nonBranchingFilterableConnections(NodeId startNodeId, NodeId endNodeId) const;

        const std::unordered_set<NodeId> branchingConnections(const NodeId startNodeId, const NodeId endNodeId) const;

        const std::unordered_set<NodeId> branchingConnections(const NodeId inputNodeId) const;

    public:
        std::unordered_set<NodeId> allNodeIds() const override;

        std::unordered_set<ConnectionId> allConnectionIds(NodeId const nodeId) const override;

        std::unordered_set<ConnectionId> connections(NodeId nodeId,
            PortType portType,
            PortIndex portIndex) const override;

        bool connectionExists(ConnectionId const connectionId) const override;

        NodeId addNode(QString const nodeType) override;

        bool connectionPossible(ConnectionId const connectionId) const override;

        void addConnection(ConnectionId const connectionId) override;

        bool nodeExists(NodeId const nodeId) const override;

        QVariant nodeData(NodeId nodeId, NodeRole role) const override;

        NodeFlags nodeFlags(NodeId nodeId) const override;

        bool setNodeData(NodeId nodeId, NodeRole role, QVariant value) override;

        QVariant portData(NodeId nodeId,
            PortType portType,
            PortIndex portIndex,
            PortRole role) const override;

        bool setPortData(NodeId nodeId,
            PortType portType,
            PortIndex portIndex,
            QVariant const& value,
            PortRole role = PortRole::Data) override;

        bool deleteConnection(ConnectionId const connectionId) override;

        bool deleteNode(NodeId const nodeId) override;

        QJsonObject saveNode(NodeId const) const override;

        QJsonObject save() const override;

        void loadNode(QJsonObject const& nodeJson) override;

        void load(QJsonObject const& json) override;

        /**
       * Fetches the NodeDelegateModel for the given `nodeId` and tries to cast the
       * stored pointer to the given type
       */
        template<typename NodeDelegateModelType>
        NodeDelegateModelType* delegateModel(NodeId const nodeId)
        {
            auto it = _models.find(nodeId);
            if (it == _models.end())
                return nullptr;

            auto model = dynamic_cast<NodeDelegateModelType*>(it->second.get());

            return model;
        }

    Q_SIGNALS:
        void inPortDataWasSet(NodeId const, PortType const, PortIndex const);

    private:
        NodeId newNodeId() override { return _nextNodeId++; }

        void sendConnectionCreation(ConnectionId const connectionId);

        void sendConnectionDeletion(ConnectionId const connectionId);

    private Q_SLOTS:
        /**
       * Fuction is called in three cases:
       *
       * - By underlying NodeDelegateModel when a node has new data to propagate.
       *   @see DataFlowGraphModel::addNode
       * - When a new connection is created.
       *   @see DataFlowGraphModel::addConnection
       * - When a node restored from JSON an needs to send data downstream.
       *   @see DataFlowGraphModel::loadNode
       */
        void onOutPortDataUpdated(NodeId const nodeId, PortIndex const portIndex);

        /// Function is called after detaching a connection.
        void propagateEmptyDataTo(NodeId const nodeId, PortIndex const portIndex);

    private:
        std::shared_ptr<NodeDelegateModelRegistry> _registry;

        NodeId _nextNodeId;

        std::unordered_map<NodeId, std::unique_ptr<NodeDelegateModel>> _models;

        std::unordered_set<ConnectionId> _connectivity;

        mutable std::unordered_map<NodeId, NodeGeometryData> _nodeGeometryData;
    };

} // namespace QtNodes


