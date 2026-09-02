#include "molly/ecs/Systems/RenderSystem.h"

#include <string>
#include <vector>

#include <glad/gl.h>
#include <glm/gtc/quaternion.hpp>

#include "molly/ecs/Components.h"

namespace molly {

glm::mat4 getWorldTransform(entt::registry& registry, entt::entity entity) {
    glm::mat4 local = registry.get<TransformComponent>(entity).getTransform();

    if (auto* hierarchy = registry.try_get<HierarchyComponent>(entity);
        hierarchy && hierarchy->parent != entt::null && registry.valid(hierarchy->parent)) {
        return getWorldTransform(registry, hierarchy->parent) * local;
    }
    return local;
}

void renderScene(entt::registry& registry, const glm::mat4& viewProjection,
                  const glm::vec3& cameraPosition) {
    // Without this, fragments are painted in whatever order entt iterates
    // entities (reverse creation order), not by distance to the camera —
    // e.g. a large floor plane created before a prop draws over it even
    // though the prop is closer to the camera.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Gather lights: first directional light found, plus up to 8 point lights.
    Light dirLight;
    bool hasDirLight = false;
    std::vector<Light> pointLights;

    for (auto entity : registry.view<TransformComponent, LightComponent>()) {
        const auto& transform = registry.get<TransformComponent>(entity);
        const auto& light = registry.get<LightComponent>(entity);

        if (light.type == LightType::Directional && !hasDirLight) {
            // The entity's rotation orients the light; -Z is the default "forward" a
            // directional light points along before rotation is applied.
            glm::quat orientation(glm::radians(transform.rotation));
            dirLight.direction = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, -1.0f));
            dirLight.color = light.color;
            dirLight.intensity = light.intensity;
            hasDirLight = true;
        } else if (light.type == LightType::Point && pointLights.size() < 8) {
            Light pl;
            pl.position = transform.position;
            pl.color = light.color;
            pl.intensity = light.intensity;
            pl.radius = light.radius;
            pointLights.push_back(pl);
        }
    }

    for (auto entity : registry.view<TransformComponent, MeshRendererComponent>()) {
        const auto& meshRenderer = registry.get<MeshRendererComponent>(entity);
        if (!meshRenderer.mesh || !meshRenderer.material) continue;

        glm::mat4 model = getWorldTransform(registry, entity);

        meshRenderer.material->bind();
        auto& shader = *meshRenderer.material->shader;
        shader.setMat4("u_ViewProjection", viewProjection);
        shader.setMat4("u_Model", model);
        shader.setMat3("u_NormalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
        shader.setFloat3("u_ViewPos", cameraPosition);

        shader.setFloat3("u_DirLight.direction", hasDirLight ? dirLight.direction
                                                              : glm::vec3(0.0f, -1.0f, 0.0f));
        shader.setFloat3("u_DirLight.color", dirLight.color);
        shader.setFloat("u_DirLight.intensity", hasDirLight ? dirLight.intensity : 0.0f);

        shader.setInt("u_PointLightCount", static_cast<int>(pointLights.size()));
        for (size_t i = 0; i < pointLights.size(); ++i) {
            std::string prefix = "u_PointLights[" + std::to_string(i) + "].";
            shader.setFloat3(prefix + "position", pointLights[i].position);
            shader.setFloat3(prefix + "color", pointLights[i].color);
            shader.setFloat(prefix + "intensity", pointLights[i].intensity);
            shader.setFloat(prefix + "radius", pointLights[i].radius);
        }

        meshRenderer.mesh->bind();
        glDrawElements(GL_TRIANGLES, static_cast<int>(meshRenderer.mesh->getIndexCount()),
                       GL_UNSIGNED_INT, nullptr);
    }
}

}  // namespace molly
