#pragma once

#include <memory>
#include <string>

#include "molly/core/Application.h"
#include "molly/resource/ResourceManager.h"

namespace molly {

// A path-based handle that resolves lazily through the ResourceManager on
// first access, rather than eagerly holding a raw pointer (PLAN.md §14.2).
// Currently specialized for Texture2D — the engine's other cached resource
// types (Shader, per-model Mesh lists) are looked up via
// ResourceManager::loadShader/loadModel directly wherever they're needed
// (a Material already holds a shared_ptr<Shader>/Texture2D once resolved,
// so there's been no pressure yet to route those through a handle too;
// extend this template if that changes).
template <typename T>
class AssetHandle {
public:
    AssetHandle() = default;
    explicit AssetHandle(std::string path) : m_path(std::move(path)) {}

    const std::string& getPath() const { return m_path; }
    bool hasPath() const { return !m_path.empty(); }

    std::shared_ptr<T> get() const;

    std::shared_ptr<T> operator->() const { return get(); }
    explicit operator bool() const { return hasPath(); }

private:
    std::string m_path;
};

template <>
inline std::shared_ptr<Texture2D> AssetHandle<Texture2D>::get() const {
    if (m_path.empty()) return nullptr;
    return Application::get().getResourceManager().loadTexture(m_path);
}

}  // namespace molly
