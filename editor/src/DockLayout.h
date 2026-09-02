#pragma once

#include <imgui.h>

namespace molly {

// Splits `dockspaceId`'s region into the standard Hierarchy / Viewport /
// Inspector / (Content Browser + Console) quadrant layout and docks each
// panel's window by name. Safe to call repeatedly — always tears down and
// rebuilds from scratch, so it doubles as the "Reset Layout" action.
// See plan/EDITOR_UI_PLAN.md §8, Appendix A.
void buildDefaultDockLayout(ImGuiID dockspaceId, const ImVec2& size);

// Returns true if `dockspaceId` has no existing dock structure — i.e.
// nothing was restored from imgui.ini, so the caller should build the
// default layout via buildDefaultDockLayout().
bool dockspaceNeedsDefaultLayout(ImGuiID dockspaceId);

}  // namespace molly
