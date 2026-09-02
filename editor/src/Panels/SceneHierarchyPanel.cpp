#include "SceneHierarchyPanel.h"

#include <imgui.h>

namespace molly {

void SceneHierarchyPanel::drawEntityNode(entt::entity entity,
                                          std::unordered_set<entt::entity>& visited) {
    if (!visited.insert(entity).second) {
        // Already visited this frame — malformed/cyclic parent-child data.
        // Bail rather than trust it (plan/EDITOR_UI_PLAN.md §21 risk note).
        return;
    }

    auto& registry = m_context->getRegistry();
    auto& tag = registry.get<TagComponent>(entity);
    auto* hierarchy = registry.try_get<HierarchyComponent>(entity);
    bool hasChildren = hierarchy && !hierarchy->children.empty();
    bool isSelected = m_selected == Entity(entity, m_context);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth |
                                ImGuiTreeNodeFlags_DefaultOpen;
    if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;
    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
    bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<uintptr_t>(entity)), flags,
                                     "%s", tag.name.c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        m_selected = Entity(entity, m_context);
    }

    if (ImGui::BeginPopupContextItem()) {
        m_selected = Entity(entity, m_context);
        if (ImGui::MenuItem("Create Prefab From Selection...")) {
            if (onCreatePrefabRequested) onCreatePrefabRequested(Entity(entity, m_context));
        }
        if (ImGui::MenuItem("Delete")) {
            m_pendingDelete = entity;
            if (m_selected == Entity(entity, m_context)) m_selected = {};
        }
        ImGui::EndPopup();
    }

    if (hasChildren && opened) {
        for (auto child : hierarchy->children) {
            drawEntityNode(child, visited);
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
}

void SceneHierarchyPanel::onImGuiRender() {
    if (!isOpen) return;
    ImGui::Begin("Hierarchy", &isOpen);

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

    {
        std::unordered_set<entt::entity> visited;
        auto& registry = m_context->getRegistry();
        for (auto entity : registry.view<TagComponent, HierarchyComponent>()) {
            auto& hierarchy = registry.get<HierarchyComponent>(entity);
            if (hierarchy.parent == entt::null) {
                drawEntityNode(entity, visited);
            }
        }
    }

    // Clicking empty space below the list deselects, matching the usual
    // hierarchy-panel convention in DCC/editor tools.
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsAnyItemHovered()) {
        m_selected = {};
    }

    ImGui::End();

    // Safe to destroy now — the view iteration above has fully completed.
    if (m_pendingDelete != entt::null) {
        m_context->destroyEntity(Entity(m_pendingDelete, m_context));
        m_pendingDelete = entt::null;
    }
}

}  // namespace molly
