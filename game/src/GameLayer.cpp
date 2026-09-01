#include "GameLayer.h"

#include <glad/gl.h>
#include <imgui.h>

namespace molly {

GameLayer::GameLayer() : Layer("GameLayer") {}

void GameLayer::onAttach() {
    glEnable(GL_DEPTH_TEST);

    auto shader = Shader::createFromFiles("assets/shaders/lit.vert", "assets/shaders/lit.frag");
    m_material = std::make_shared<Material>();
    m_material->shader = shader;
    m_material->albedoColor = {0.85f, 0.65f, 0.35f};

    m_meshes = ModelLoader::load("assets/models/box/Box.gltf");
    if (m_meshes.empty()) {
        MOLLY_WARN("No meshes loaded for the Milestone 2 test model; falling back to a procedural cube");
        m_meshes.push_back(Mesh::createCube());
    }

    m_dirLight.type = LightType::Directional;
    m_dirLight.direction = glm::normalize(glm::vec3(-0.4f, -0.9f, -0.3f));
    m_dirLight.color = {1.0f, 0.96f, 0.9f};
    m_dirLight.intensity = 1.1f;

    auto& window = Application::get().getWindow();
    float aspect = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());
    m_camera = PerspectiveCamera(50.0f, aspect, 0.1f, 100.0f);
    m_camera.setPosition({0.0f, 2.2f, 3.0f});
    m_camera.setPitchYaw(-45.0f, -90.0f);
}

void GameLayer::onUpdate(float dt) {
    m_time += dt;

    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_material->bind();
    m_material->shader->setMat4("u_ViewProjection", m_camera.getViewProjectionMatrix());
    m_material->shader->setFloat3("u_ViewPos", m_camera.getPosition());

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), m_time * 0.6f, glm::vec3(0.0f, 1.0f, 0.0f));
    m_material->shader->setMat4("u_Model", model);
    m_material->shader->setMat3("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));

    m_material->shader->setFloat3("u_DirLight.direction", m_dirLight.direction);
    m_material->shader->setFloat3("u_DirLight.color", m_dirLight.color);
    m_material->shader->setFloat("u_DirLight.intensity", m_dirLight.intensity);
    m_material->shader->setInt("u_PointLightCount", 0);

    for (const auto& mesh : m_meshes) {
        mesh->bind();
        glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->getIndexCount()), GL_UNSIGNED_INT,
                       nullptr);
    }
}

void GameLayer::onImGuiRender() {
    ImGui::Begin("Molly Demo - Milestone 2");
    ImGui::Text("Renderer core: textured/lit model loaded via Assimp, perspective camera.");
    ImGui::Text("Frame time: %.3f ms (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::End();
}

}  // namespace molly
