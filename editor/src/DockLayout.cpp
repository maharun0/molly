#include "DockLayout.h"

#include <imgui_internal.h>  // ImGuiDockBuilder family — plan/EDITOR_UI_PLAN.md §21 risk note

namespace molly {

bool dockspaceNeedsDefaultLayout(ImGuiID dockspaceId) {
    ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspaceId);
    return node == nullptr || (!node->IsSplitNode() && node->Windows.Size == 0);
}

void buildDefaultDockLayout(ImGuiID dockspaceId, const ImVec2& size) {
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, size);

    // 1) Root -> left 18% (Hierarchy) + remainder
    ImGuiID dockMainArea = dockspaceId;
    ImGuiID dockHierarchy =
        ImGui::DockBuilderSplitNode(dockMainArea, ImGuiDir_Left, 0.18f, nullptr, &dockMainArea);

    // 2) Remainder -> right 22% (Inspector) + remainder
    ImGuiID dockInspector =
        ImGui::DockBuilderSplitNode(dockMainArea, ImGuiDir_Right, 0.22f, nullptr, &dockMainArea);

    // 3) Remainder -> bottom 30% (Content Browser + Console) + remainder (Viewport)
    ImGuiID dockBottom =
        ImGui::DockBuilderSplitNode(dockMainArea, ImGuiDir_Down, 0.30f, nullptr, &dockMainArea);
    ImGuiID dockViewport = dockMainArea;

    // Assign each panel's window name (must match its ImGui::Begin(...) call exactly).
    ImGui::DockBuilderDockWindow("Hierarchy", dockHierarchy);
    ImGui::DockBuilderDockWindow("Viewport", dockViewport);
    ImGui::DockBuilderDockWindow("Inspector", dockInspector);
    ImGui::DockBuilderDockWindow("Content Browser", dockBottom);
    ImGui::DockBuilderDockWindow("Console", dockBottom);

    ImGui::DockBuilderFinish(dockspaceId);
}

}  // namespace molly
