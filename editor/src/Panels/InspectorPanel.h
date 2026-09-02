#pragma once

#include <molly/Molly.h>

namespace molly {

// Renders every component on the given entity with an editable widget block,
// plus an "Add Component" dropdown for component types the entity doesn't
// have yet (PLAN.md §18.3).
class InspectorPanel {
public:
    void onImGuiRender(Scene* scene, Entity selected);

    // Backs the Window menu's show/hide checkbox (plan/EDITOR_UI_PLAN.md §9.1).
    bool isOpen = true;

private:
    void drawAddComponentMenu(Entity entity);
};

}  // namespace molly
