#include "SceneHierarchyPanel.h"

#include <imgui.h>

namespace molly {

void SceneHierarchyPanel::onImGuiRender() {
    ImGui::Begin("Hierarchy");

    if (!m_context) {
        ImGui::TextDisabled("No active scene");
        ImGui::End();
        return;
    }

    if (ImGui::Button("Create Entity")) {
        ++m_spawnCounter;
        m_selected = m_context->createEntity("Entity" + std::to_string(m_spawnCounter));
    }

    ImGui::Separator();

    auto& registry = m_context->getRegistry();
    for (auto entity : registry.view<TagComponent>()) {
        auto& tag = registry.get<TagComponent>(entity);
        bool isSelected = m_selected == Entity(entity, m_context);
        if (ImGui::Selectable(tag.name.c_str(), isSelected)) {
            m_selected = Entity(entity, m_context);
        }
    }

    // Clicking empty space below the list deselects, matching the usual
    // hierarchy-panel convention in DCC/editor tools.
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered()) {
        m_selected = {};
    }

    ImGui::End();
}

}  // namespace molly
