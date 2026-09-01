#pragma once

#include <string>

#include <molly/Molly.h>

namespace molly {

// Lists every entity in the active scene; owns the current selection (read
// by InspectorPanel each frame) and the create/delete entity actions
// (PLAN.md §18.3).
class SceneHierarchyPanel {
public:
    void setContext(Scene* scene) { m_context = scene; }

    void onImGuiRender();

    Entity getSelected() const { return m_selected; }
    void setSelected(Entity entity) { m_selected = entity; }

private:
    Scene* m_context = nullptr;
    Entity m_selected;
    int m_spawnCounter = 0;
};

}  // namespace molly
