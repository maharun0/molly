#pragma once

#include <filesystem>

namespace molly {

// A read-only file tree rooted at assets/ (PLAN.md §18.3). Double-click/
// open-in-editor actions land in Milestone 8 alongside real asset saving;
// for now this just proves the project's asset layout is browsable from
// inside the editor.
class ContentBrowserPanel {
public:
    ContentBrowserPanel();

    void onImGuiRender();

private:
    void drawDirectory(const std::filesystem::path& path);

    std::filesystem::path m_rootPath;
};

}  // namespace molly
