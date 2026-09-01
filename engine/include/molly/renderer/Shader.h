#pragma once

#include <string>

#include <glm/glm.hpp>

namespace molly {

class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void bind() const;
    void unbind() const;

    void setInt(const std::string& name, int value);
    void setFloat3(const std::string& name, const glm::vec3& value);
    void setMat4(const std::string& name, const glm::mat4& value);

private:
    int getUniformLocation(const std::string& name);

    unsigned int m_rendererId = 0;
};

}  // namespace molly
