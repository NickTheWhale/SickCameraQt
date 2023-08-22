#pragma once

#include "BasicGraphicsScene.hpp"
#include "FilterFlowGraphModel.h"
#include "Export.hpp"

namespace QtNodes {

    /// @brief An advanced scene working with data-propagating graphs.
    /**
     * The class represents a scene that existed in v2.x but built wit the
     * new model-view approach in mind.
     */
    class NODE_EDITOR_PUBLIC FilterFlowGraphicsScene : public BasicGraphicsScene
    {
        Q_OBJECT
    public:
        FilterFlowGraphicsScene(FilterFlowGraphModel& graphModel, QObject* parent = nullptr);

        ~FilterFlowGraphicsScene() = default;

    public:
        std::vector<NodeId> selectedNodes() const;

    public:
        QMenu* createSceneMenu(QPointF const scenePos) override;

    public Q_SLOTS:
        void save() const;

        void load();

    Q_SIGNALS:
        void sceneLoaded();

    private:
        FilterFlowGraphModel& _graphModel;
    };

} // namespace QtNodes
