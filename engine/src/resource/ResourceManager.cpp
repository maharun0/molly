#include "molly/resource/ResourceManager.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <thread>
#include <vector>

#include "molly/core/Log.h"
#include "molly/resource/ModelLoader.h"

namespace molly {

namespace fs = std::filesystem;

enum class ResourceKind { Texture, Shader };

struct WatchEntry {
    std::string filePath;
    ResourceKind kind;
    std::string cacheKey;
    fs::file_time_type lastWriteTime;
};

struct ResourceManager::WatcherState {
    std::thread thread;
    std::atomic<bool> running{false};

    std::mutex entriesMutex;
    std::vector<WatchEntry> entries;

    std::mutex changedMutex;
    std::vector<std::pair<ResourceKind, std::string>> changedKeys;  // drained by update()
};

// Out-of-line so std::thread's non-trivial destructor (via
// unique_ptr<WatcherState>) doesn't require WatcherState to be a complete
// type wherever ResourceManager.h is included (same reasoning as
// PhysicsWorld's contact listener, PLAN.md Appendix D).
ResourceManager::ResourceManager() = default;

ResourceManager::~ResourceManager() { setHotReloadEnabled(false); }

std::shared_ptr<Texture2D> ResourceManager::loadTexture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) return it->second;

    auto texture = std::make_shared<Texture2D>(path);
    m_textures[path] = texture;

    if (m_watcher) {
        std::lock_guard<std::mutex> lock(m_watcher->entriesMutex);
        std::error_code ec;
        m_watcher->entries.push_back(
            {path, ResourceKind::Texture, path, fs::last_write_time(path, ec)});
    }

    return texture;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& vertexPath,
                                                     const std::string& fragmentPath) {
    std::string key = vertexPath + "|" + fragmentPath;
    auto it = m_shaders.find(key);
    if (it != m_shaders.end()) return it->second;

    auto shader = Shader::createFromFiles(vertexPath, fragmentPath);
    m_shaders[key] = shader;

    if (m_watcher) {
        std::lock_guard<std::mutex> lock(m_watcher->entriesMutex);
        std::error_code ec;
        m_watcher->entries.push_back(
            {vertexPath, ResourceKind::Shader, key, fs::last_write_time(vertexPath, ec)});
        m_watcher->entries.push_back(
            {fragmentPath, ResourceKind::Shader, key, fs::last_write_time(fragmentPath, ec)});
    }

    return shader;
}

std::vector<std::shared_ptr<Mesh>> ResourceManager::loadModel(const std::string& path) {
    auto it = m_models.find(path);
    if (it != m_models.end()) return it->second;

    auto meshes = ModelLoader::load(path);
    m_models[path] = meshes;
    return meshes;
}

void ResourceManager::setHotReloadEnabled(bool enabled) {
    if (enabled && !m_watcher) {
        m_watcher = std::make_unique<WatcherState>();
        m_watcher->running = true;
        m_watcher->thread = std::thread([this]() {
            while (m_watcher->running) {
                std::this_thread::sleep_for(std::chrono::seconds(1));

                std::lock_guard<std::mutex> entriesLock(m_watcher->entriesMutex);
                for (auto& entry : m_watcher->entries) {
                    std::error_code ec;
                    auto currentTime = fs::last_write_time(entry.filePath, ec);
                    if (ec || currentTime == entry.lastWriteTime) continue;

                    entry.lastWriteTime = currentTime;
                    std::lock_guard<std::mutex> changedLock(m_watcher->changedMutex);
                    m_watcher->changedKeys.emplace_back(entry.kind, entry.cacheKey);
                }
            }
        });
    } else if (!enabled && m_watcher) {
        m_watcher->running = false;
        if (m_watcher->thread.joinable()) m_watcher->thread.join();
        m_watcher.reset();
    }
}

void ResourceManager::update() {
    if (!m_watcher) return;

    std::vector<std::pair<ResourceKind, std::string>> changed;
    {
        std::lock_guard<std::mutex> lock(m_watcher->changedMutex);
        changed.swap(m_watcher->changedKeys);
    }

    for (auto& [kind, key] : changed) {
        if (kind == ResourceKind::Texture) {
            auto it = m_textures.find(key);
            if (it != m_textures.end() && it->second->reload()) {
                MOLLY_INFO("Hot-reloaded texture: {0}", key);
            }
        } else {
            auto it = m_shaders.find(key);
            if (it != m_shaders.end() && it->second->reload()) {
                MOLLY_INFO("Hot-reloaded shader: {0}", key);
            }
        }
    }
}

}  // namespace molly
