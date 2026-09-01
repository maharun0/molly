#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "molly/renderer/Shader.h"
#include "molly/renderer/Texture.h"

namespace molly {

// Simple value-struct material for v1: one shader + a fixed set of named
// parameters. No shader-graph/node system — see PLAN.md §10.2.
struct Material {
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture2D> albedoTexture;
    glm::vec3 albedoColor{1.0f};
    float shininess = 32.0f;

    void bind() const {
        shader->bind();

        static std::shared_ptr<Texture2D> s_whiteTexture = [] {
            auto tex = std::make_shared<Texture2D>(1u, 1u);
            uint32_t whitePixel = 0xffffffff;
            tex->setData(&whitePixel, sizeof(whitePixel));
            return tex;
        }();

        (albedoTexture ? albedoTexture : s_whiteTexture)->bind(0);
        shader->setInt("u_AlbedoTexture", 0);
        shader->setFloat3("u_AlbedoColor", albedoColor);
    }
};

}  // namespace molly
