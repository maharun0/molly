#include "molly/renderer/Shader.h"

#include <vector>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "molly/core/Log.h"

namespace molly {

static unsigned int compile(unsigned int type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<size_t>(length));
        glGetShaderInfoLog(shader, length, &length, log.data());
        MOLLY_ERROR("Shader compile error: {0}", log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
    unsigned int vs = compile(GL_VERTEX_SHADER, vertexSrc);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, fragmentSrc);

    m_rendererId = glCreateProgram();
    glAttachShader(m_rendererId, vs);
    glAttachShader(m_rendererId, fs);
    glLinkProgram(m_rendererId);

    int success = 0;
    glGetProgramiv(m_rendererId, GL_LINK_STATUS, &success);
    if (!success) {
        int length = 0;
        glGetProgramiv(m_rendererId, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<size_t>(length));
        glGetProgramInfoLog(m_rendererId, length, &length, log.data());
        MOLLY_ERROR("Shader link error: {0}", log.data());
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() { glDeleteProgram(m_rendererId); }

void Shader::bind() const { glUseProgram(m_rendererId); }

void Shader::unbind() const { glUseProgram(0); }

int Shader::getUniformLocation(const std::string& name) {
    return glGetUniformLocation(m_rendererId, name.c_str());
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

}  // namespace molly
