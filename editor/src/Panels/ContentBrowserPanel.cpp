#include "ContentBrowserPanel.h"

#include <algorithm>
#include <vector>

#include <imgui.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")

namespace molly {

// Opens `path` in whatever the OS considers the default handler for its
// file type (e.g. a text editor for .lua) — Windows-only, matching the
// engine's Windows-only scope (PLAN.md §1.2).
static void openInSystemEditor(const std::filesystem::path& path) {
    std::wstring wpath = path.wstring();
    ShellExecuteW(nullptr, L"open", wpath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

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
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                openInSystemEditor(entry.path());
            }
        }
    }
}

void ContentBrowserPanel::onImGuiRender() {
    if (!isOpen) return;
    ImGui::Begin("Content Browser", &isOpen);
    drawDirectory(m_rootPath);
    ImGui::End();
}

}  // namespace molly
