#include "ContentBrowserPanel.h"

#include <algorithm>
#include <vector>

#include <imgui.h>

namespace molly {

ContentBrowserPanel::ContentBrowserPanel() : m_rootPath("assets") {}

void ContentBrowserPanel::drawDirectory(const std::filesystem::path& path) {
    namespace fs = std::filesystem;

    std::error_code ec;
    if (!fs::exists(path, ec)) return;

    std::vector<fs::directory_entry> entries;
    for (auto& entry : fs::directory_iterator(path, ec)) entries.push_back(entry);
    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        if (a.is_directory() != b.is_directory()) return a.is_directory() > b.is_directory();
        return a.path().filename() < b.path().filename();
    });

    for (auto& entry : entries) {
        std::string name = entry.path().filename().string();
        if (entry.is_directory()) {
            if (ImGui::TreeNode(name.c_str())) {
                drawDirectory(entry.path());
                ImGui::TreePop();
            }
        } else {
            ImGui::TreeNodeEx(name.c_str(),
                               ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                   ImGuiTreeNodeFlags_Bullet);
        }
    }
}

void ContentBrowserPanel::onImGuiRender() {
    ImGui::Begin("Content Browser");
    drawDirectory(m_rootPath);
    ImGui::End();
}

}  // namespace molly
