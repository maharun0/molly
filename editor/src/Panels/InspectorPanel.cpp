#include "InspectorPanel.h"

#include <cstring>

#include <imgui.h>

namespace molly {

template <typename T, typename Fn>
static void drawComponent(const char* name, Entity entity, Fn drawFunc) {
    if (!entity.hasComponent<T>()) return;
    if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushID(name);
        drawFunc(entity.getComponent<T>());
        ImGui::PopID();
    }
}

static void editString(const char* label, std::string& value) {
    char buffer[256];
    std::strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    if (ImGui::InputText(label, buffer, sizeof(buffer))) {
        value = buffer;
    }
}

void InspectorPanel::onImGuiRender(Scene* scene, Entity selected) {
    ImGui::Begin("Inspector");

    if (!scene || !selected) {
        ImGui::TextDisabled("No entity selected");
        ImGui::End();
        return;
    }

    auto& tag = selected.getComponent<TagComponent>();
    editString("##Name", tag.name);

    ImGui::SameLine();
    drawAddComponentMenu(selected);

    ImGui::Separator();

    drawComponent<TransformComponent>("Transform", selected, [](TransformComponent& t) {
        ImGui::DragFloat3("Position", &t.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &t.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
    });

    drawComponent<MeshRendererComponent>("Mesh Renderer", selected, [](MeshRendererComponent& mr) {
        ImGui::Text("Mesh: %s", mr.mesh ? "loaded" : "(none)");
        if (mr.material) {
            ImGui::ColorEdit3("Albedo Color", &mr.material->albedoColor.x);
            ImGui::Text("Texture: %s", mr.material->albedoTexture ? "assigned" : "(white default)");
        }
    });

    drawComponent<CameraComponent>("Camera", selected, [](CameraComponent& cam) {
        ImGui::Checkbox("Primary", &cam.primary);
        float fov = cam.camera.getFov();
        if (ImGui::DragFloat("FOV", &fov, 0.5f, 10.0f, 120.0f)) {
            cam.camera.setFov(fov);
        }
    });

    drawComponent<LightComponent>("Light", selected, [](LightComponent& light) {
        int typeIndex = light.type == LightType::Directional ? 0 : 1;
        const char* typeNames[] = {"Directional", "Point"};
        if (ImGui::Combo("Type", &typeIndex, typeNames, 2)) {
            light.type = typeIndex == 0 ? LightType::Directional : LightType::Point;
        }
        ImGui::ColorEdit3("Color", &light.color.x);
        ImGui::DragFloat("Intensity", &light.intensity, 0.05f, 0.0f, 10.0f);
        if (light.type == LightType::Point) {
            ImGui::DragFloat("Radius", &light.radius, 0.1f, 0.1f, 100.0f);
        }
        if (light.type == LightType::Directional) {
            ImGui::TextDisabled("Direction is derived from this entity's Rotation.");
        }
    });

    drawComponent<RigidBodyComponent>("Rigid Body", selected, [](RigidBodyComponent& rb) {
        int typeIndex = static_cast<int>(rb.type);
        const char* typeNames[] = {"Static", "Kinematic", "Dynamic"};
        if (ImGui::Combo("Body Type", &typeIndex, typeNames, 3)) {
            rb.type = static_cast<BodyType>(typeIndex);
        }
        if (rb.type == BodyType::Dynamic) {
            ImGui::DragFloat("Mass", &rb.mass, 0.1f, 0.01f, 1000.0f);
        }
        if (rb.created) {
            ImGui::TextDisabled("Body already created — type/mass changes apply next play session.");
        }
    });

    drawComponent<ColliderComponent>("Collider", selected, [](ColliderComponent& col) {
        int shapeIndex = static_cast<int>(col.shape);
        const char* shapeNames[] = {"Box", "Sphere", "Capsule"};
        if (ImGui::Combo("Shape", &shapeIndex, shapeNames, 3)) {
            col.shape = static_cast<ColliderShape>(shapeIndex);
        }
        if (col.shape == ColliderShape::Box) {
            ImGui::DragFloat3("Half Extents", &col.halfExtents.x, 0.05f, 0.01f, 50.0f);
        } else {
            ImGui::DragFloat("Radius", &col.radius, 0.05f, 0.01f, 50.0f);
            if (col.shape == ColliderShape::Capsule) {
                ImGui::DragFloat("Height", &col.height, 0.05f, 0.01f, 50.0f);
            }
        }
        ImGui::DragFloat("Friction", &col.friction, 0.02f, 0.0f, 2.0f);
        ImGui::DragFloat("Restitution", &col.restitution, 0.02f, 0.0f, 1.0f);
        ImGui::Checkbox("Is Trigger", &col.isTrigger);
    });

    drawComponent<CharacterControllerComponent>(
        "Character Controller", selected, [](CharacterControllerComponent& cc) {
            ImGui::DragFloat("Speed", &cc.speed, 0.1f, 0.0f, 50.0f);
            ImGui::DragFloat("Radius", &cc.radius, 0.02f, 0.05f, 5.0f);
            ImGui::DragFloat("Height", &cc.height, 0.02f, 0.1f, 10.0f);
        });

    drawComponent<ScriptComponent>("Script", selected, [](ScriptComponent& script) {
        editString("Script Path", script.scriptPath);
        ImGui::TextDisabled(script.started ? "Running" : "Not started yet");
    });

    drawComponent<AudioSourceComponent>("Audio Source", selected, [](AudioSourceComponent& src) {
        editString("Clip Path", src.clipPath);
        ImGui::DragFloat("Volume", &src.volume, 0.02f, 0.0f, 2.0f);
        ImGui::Checkbox("Loop", &src.loop);
        ImGui::Checkbox("3D (Positional)", &src.is3D);
        ImGui::Checkbox("Auto Play", &src.autoPlay);
        ImGui::TextDisabled(src.sound ? "Loaded" : "Not loaded yet");
    });

    drawComponent<AudioListenerComponent>("Audio Listener", selected,
                                           [](AudioListenerComponent&) {
                                               ImGui::TextDisabled("No fields — marker component.");
                                           });

    ImGui::End();
}

void InspectorPanel::drawAddComponentMenu(Entity entity) {
    if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponentPopup");

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (!entity.hasComponent<MeshRendererComponent>() && ImGui::MenuItem("Mesh Renderer")) {
            entity.addComponent<MeshRendererComponent>();
        }
        if (!entity.hasComponent<CameraComponent>() && ImGui::MenuItem("Camera")) {
            entity.addComponent<CameraComponent>();
        }
        if (!entity.hasComponent<LightComponent>() && ImGui::MenuItem("Light")) {
            entity.addComponent<LightComponent>();
        }
        if (!entity.hasComponent<RigidBodyComponent>() && ImGui::MenuItem("Rigid Body")) {
            entity.addComponent<RigidBodyComponent>();
        }
        if (!entity.hasComponent<ColliderComponent>() && ImGui::MenuItem("Collider")) {
            entity.addComponent<ColliderComponent>();
        }
        if (!entity.hasComponent<CharacterControllerComponent>() &&
            ImGui::MenuItem("Character Controller")) {
            entity.addComponent<CharacterControllerComponent>();
        }
        if (!entity.hasComponent<ScriptComponent>() && ImGui::MenuItem("Script")) {
            entity.addComponent<ScriptComponent>();
        }
        if (!entity.hasComponent<AudioSourceComponent>() && ImGui::MenuItem("Audio Source")) {
            entity.addComponent<AudioSourceComponent>();
        }
        if (!entity.hasComponent<AudioListenerComponent>() && ImGui::MenuItem("Audio Listener")) {
            entity.addComponent<AudioListenerComponent>();
        }
        ImGui::EndPopup();
    }
}

}  // namespace molly
