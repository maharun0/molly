#pragma once

#include <functional>
#include <string>
#include <unordered_set>

#include <molly/Molly.h>

namespace molly {

// Lists every entity in the active scene as a real parent/child tree
// (matching HierarchyComponent); owns the current selection (read by
// InspectorPanel each frame) and the create/delete entity actions
// (PLAN.md §18.3; plan/EDITOR_UI_PLAN.md §9.4).
class SceneHierarchyPanel {
public:
    void setContext(Scene* scene) { m_context = scene; }

    void onImGuiRender();

    Entity getSelected() const { return m_selected; }
    void setSelected(Entity entity) { m_selected = entity; }

    // Set by EditorLayer; invoked from the per-entity right-click context
    // menu's "Create Prefab From Selection..." item (plan/EDITOR_UI_PLAN.md
    // §9.4.2, §15.4) — EditorLayer owns the actual modal/path input, this
    // panel only needs to report which entity was right-clicked.
    std::function<void(Entity)> onCreatePrefabRequested;

    bool isOpen = true;

private:
    void drawEntityNode(entt::entity entity, std::unordered_set<entt::entity>& visited);

    Scene* m_context = nullptr;
    Entity m_selected;
    int m_spawnCounter = 0;

    // Deleting an entity while iterating a registry view over it is unsafe
    // (invalidates entt's iterator) — the context menu only records the
    // request here; onImGuiRender() processes it once the tree traversal
    // for this frame has fully finished.
    entt::entity m_pendingDelete = entt::null;
};

}  // namespace molly
