//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QPointF>
#include <QtCore/QSize>

#include <QtNodes/AbstractGraphModel>
#include <QtNodes/ConnectionIdUtils>
#include <QtNodes/StyleCollection>

namespace vox {

using ConnectionId = QtNodes::ConnectionId;
using ConnectionPolicy = QtNodes::ConnectionPolicy;
using NodeFlag = QtNodes::NodeFlag;
using NodeId = QtNodes::NodeId;
using NodeRole = QtNodes::NodeRole;
using PortIndex = QtNodes::PortIndex;
using PortRole = QtNodes::PortRole;
using PortType = QtNodes::PortType;
using StyleCollection = QtNodes::StyleCollection;
using QtNodes::InvalidNodeId;

/**
 * The class implements a bare minimum required to demonstrate a model-based
 * graph.
 */
class SimpleGraphModel : public QtNodes::AbstractGraphModel {
    Q_OBJECT
public:
    struct NodeGeometryData {
        QSize size;
        QPointF pos;
    };

public:
    SimpleGraphModel();

    ~SimpleGraphModel() override;

    std::unordered_set<NodeId> allNodeIds() const override;

    std::unordered_set<ConnectionId> allConnectionIds(NodeId nodeId) const override;

    std::unordered_set<ConnectionId> connections(NodeId nodeId,
                                                 PortType portType,
                                                 PortIndex portIndex) const override;

    bool connectionExists(ConnectionId connectionId) const override;

    NodeId addNode(QString nodeType = QString()) override;

    /**
   * Connection is possible when graph contains no connectivity data
   * in both directions `Out -> In` and `In -> Out`.
   */
    bool connectionPossible(ConnectionId connectionId) const override;

    void addConnection(ConnectionId connectionId) override;

    bool nodeExists(NodeId nodeId) const override;

    QVariant nodeData(NodeId nodeId, NodeRole role) const override;

    bool setNodeData(NodeId nodeId, NodeRole role, QVariant value) override;

    QVariant portData(NodeId nodeId,
                      PortType portType,
                      PortIndex portIndex,
                      PortRole role) const override;

    bool setPortData(NodeId nodeId,
                     PortType portType,
                     PortIndex portIndex,
                     QVariant const &value,
                     PortRole role = PortRole::Data) override;

    bool deleteConnection(ConnectionId connectionId) override;

    bool deleteNode(NodeId nodeId) override;

    QJsonObject saveNode(NodeId) const override;

    /// @brief Creates a new node based on the information in `nodeJson`.
    /**
   * @param nodeJson contains a `NodeId`, node's position, internal node
   * information.
   */
    void loadNode(QJsonObject const &nodeJson) override;

    NodeId newNodeId() override { return _nextNodeId++; }

private:
    std::unordered_set<NodeId> _nodeIds;

    /// [Important] This is a user defined data structure backing your model.
    /// In your case it could be anything else representing a graph, for example, a
    /// table. Or a collection of structs with pointers to each other. Or an
    /// abstract syntax tree, you name it.
    ///
    /// This data structure contains the graph connectivity information in both
    /// directions, i.e. from Node1 to Node2 and from Node2 to Node1.
    std::unordered_set<ConnectionId> _connectivity;

    mutable std::unordered_map<NodeId, NodeGeometryData> _nodeGeometryData;

    /// A convenience variable needed for generating unique node ids.
    unsigned int _nextNodeId;
};
}// namespace vox