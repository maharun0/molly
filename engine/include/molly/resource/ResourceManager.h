#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "molly/renderer/Mesh.h"
#include "molly/renderer/Shader.h"
#include "molly/renderer/Texture.h"

namespace molly {

// Centralized, path-keyed caches for textures/shaders/models (PLAN.md §14).
// Loading in v1 is synchronous, on the main thread, at first use — see
// §14.3 for why async loading isn't warranted at this project's scale.
//
// One instance lives for the whole application (owned by Application),
// since assets are shared across scenes rather than scene-scoped.
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    std::shared_ptr<Texture2D> loadTexture(const std::string& path);
    std::shared_ptr<Shader> loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    std::vector<std::shared_ptr<Mesh>> loadModel(const std::string& path);

    // Polls watched files for changes (once per second, PLAN.md §14.4) and,
    // on change, reloads the affected texture/shader in place. Must be
    // called from the main/GL thread once per frame — the actual GL calls
    // never happen off it, only the file-timestamp check runs in the
    // background (see ResourceManager.cpp for the watcher thread).
    void update();

    void setHotReloadEnabled(bool enabled);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Mesh>>> m_models;

    struct WatcherState;
    std::unique_ptr<WatcherState> m_watcher;
};

}  // namespace molly
