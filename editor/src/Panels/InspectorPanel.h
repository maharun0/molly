#pragma once

#include <molly/Molly.h>

namespace molly {

// Renders every component on the given entity with an editable widget block,
// plus an "Add Component" dropdown for component types the entity doesn't
// have yet (PLAN.md §18.3).
class InspectorPanel {
public:
    void onImGuiRender(Scene* scene, Entity selected);

private:
    void drawAddComponentMenu(Entity entity);
};

}  // namespace molly
