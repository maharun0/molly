#pragma once

#include <memory>

#include <molly/Molly.h>

#include "Panels/ConsolePanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ViewportPanel.h"

namespace molly {

enum class EditorPlayState { Edit, Playing, Paused };

class EditorLayer : public Layer {
public:
    EditorLayer();

    void onAttach() override;
    void onUpdate(float dt) override;
    void onImGuiRender() override;

private:
    void buildDefaultScene();
    void drawMenuBar();
    void drawToolbar();
    Scene* activeScene();

    void onPlay();
    void onStop();
    void onPause();

    void newScene();
    void saveScene();
    void openScene();
    void createPrefabFromSelection();

    std::unique_ptr<Scene> m_editScene;
    std::unique_ptr<Scene> m_runtimeScene;
    EditorPlayState m_playState = EditorPlayState::Edit;

    std::shared_ptr<Material> m_material;
    char m_scenePathBuffer[256] = "assets/scenes/main.scene.json";
    char m_prefabPathBuffer[256] = "assets/prefabs/new_prefab.prefab.json";

    SceneHierarchyPanel m_hierarchyPanel;
    InspectorPanel m_inspectorPanel;
    ViewportPanel m_viewportPanel;
    ContentBrowserPanel m_contentBrowserPanel;
    ConsolePanel m_consolePanel;
};

}  // namespace molly
