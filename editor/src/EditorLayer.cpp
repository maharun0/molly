#include "EditorLayer.h"

#include <glad/gl.h>
#include <imgui.h>

namespace molly {

static const char* s_vertexSrc = R"(
    #version 460 core
    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec3 a_Color;

    out vec3 v_Color;

    void main() {
        v_Color = a_Color;
        gl_Position = vec4(a_Position, 1.0);
    }
)";

static const char* s_fragmentSrc = R"(
    #version 460 core
    in vec3 v_Color;
    out vec4 o_Color;

    void main() {
        o_Color = vec4(v_Color, 1.0);
    }
)";

EditorLayer::EditorLayer() : Layer("EditorLayer") {}

void EditorLayer::onAttach() {
    m_shader = std::make_shared<Shader>(s_vertexSrc, s_fragmentSrc);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.2f, 0.2f,
         0.5f, -0.5f, 0.0f, 0.2f, 1.0f, 0.2f,
         0.0f,  0.5f, 0.0f, 0.2f, 0.2f, 1.0f,
    };
    uint32_t indices[] = {0, 1, 2};

    auto vb = std::make_shared<VertexBuffer>(vertices, static_cast<uint32_t>(sizeof(vertices)));
    vb->setLayout({
        {ShaderDataType::Float3, "a_Position"},
        {ShaderDataType::Float3, "a_Color"},
    });

    auto ib = std::make_shared<IndexBuffer>(indices, 3);

    m_vertexArray = std::make_shared<VertexArray>();
    m_vertexArray->addVertexBuffer(vb);
    m_vertexArray->setIndexBuffer(ib);
}

void EditorLayer::onUpdate(float /*dt*/) {
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->bind();
    m_vertexArray->bind();
    glDrawElements(GL_TRIANGLES, static_cast<int>(m_vertexArray->getIndexBuffer()->getCount()),
                   GL_UNSIGNED_INT, nullptr);
}

void EditorLayer::onImGuiRender() {
    ImGui::Begin("Molly Editor - Milestone 1");
    ImGui::Text("Bootstrap: window + OpenGL + ImGui, all working.");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Checkbox("Show ImGui Demo Window", &m_showDemoWindow);
    ImGui::End();

    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
}

}  // namespace molly
