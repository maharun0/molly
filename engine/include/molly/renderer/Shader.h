#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace molly {

class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~Shader();

    // Reads vertex/fragment GLSL source from disk and compiles them.
    static std::shared_ptr<Shader> createFromFiles(const std::string& vertexPath,
                                                     const std::string& fragmentPath);

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void bind() const;
    void unbind() const;

    // Re-reads source from the paths given to createFromFiles and recompiles
    // in place (same GL program name, so existing shared_ptr<Shader>
    // references — e.g. held by Material — stay valid). On a compile/link
    // failure, logs the error and leaves the previously working program
    // bound, rather than replacing it with a broken one. Used by the
    // resource hot-reload watcher (PLAN.md §14.4); only meaningful for
    // shaders created via createFromFiles (no-op otherwise).
    bool reload();

    const std::string& getVertexPath() const { return m_vertexPath; }
    const std::string& getFragmentPath() const { return m_fragmentPath; }

    void setInt(const std::string& name, int value);
    void setIntArray(const std::string& name, const int* values, uint32_t count);
    void setFloat(const std::string& name, float value);
    void setFloat3(const std::string& name, const glm::vec3& value);
    void setFloat4(const std::string& name, const glm::vec4& value);
    void setMat3(const std::string& name, const glm::mat3& value);
    void setMat4(const std::string& name, const glm::mat4& value);

private:
    int getUniformLocation(const std::string& name);

    unsigned int m_rendererId = 0;
    std::unordered_map<std::string, int> m_uniformLocationCache;
    std::string m_vertexPath;
    std::string m_fragmentPath;
};

}  // namespace molly
