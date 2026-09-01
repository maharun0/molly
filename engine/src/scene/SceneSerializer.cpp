#include "molly/scene/SceneSerializer.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "molly/core/Application.h"
#include "molly/core/Log.h"
#include "molly/ecs/Components.h"
#include "molly/ecs/Scene.h"
#include "molly/physics/PhysicsComponents.h"
#include "molly/resource/ResourceManager.h"
#include "molly/scripting/ScriptComponent.h"

using json = nlohmann::json;

namespace molly {

namespace {

json vec3ToJson(const glm::vec3& v) { return json::array({v.x, v.y, v.z}); }

glm::vec3 jsonToVec3(const json& j, const glm::vec3& fallback) {
    if (!j.is_array() || j.size() != 3) return fallback;
    return {j[0].get<float>(), j[1].get<float>(), j[2].get<float>()};
}

const char* meshSourceToString(MeshSource s) {
    switch (s) {
        case MeshSource::Model: return "Model";
        case MeshSource::Cube: return "Cube";
        case MeshSource::Plane: return "Plane";
        default: return "None";
    }
}

MeshSource meshSourceFromString(const std::string& s) {
    if (s == "Model") return MeshSource::Model;
    if (s == "Cube") return MeshSource::Cube;
    if (s == "Plane") return MeshSource::Plane;
    return MeshSource::None;
}

const char* bodyTypeToString(BodyType t) {
    switch (t) {
        case BodyType::Kinematic: return "Kinematic";
        case BodyType::Dynamic: return "Dynamic";
        default: return "Static";
    }
}

BodyType bodyTypeFromString(const std::string& s) {
    if (s == "Kinematic") return BodyType::Kinematic;
    if (s == "Dynamic") return BodyType::Dynamic;
    return BodyType::Static;
}

const char* colliderShapeToString(ColliderShape s) {
    switch (s) {
        case ColliderShape::Sphere: return "Sphere";
        case ColliderShape::Capsule: return "Capsule";
        default: return "Box";
    }
}

ColliderShape colliderShapeFromString(const std::string& s) {
    if (s == "Sphere") return ColliderShape::Sphere;
    if (s == "Capsule") return ColliderShape::Capsule;
    return ColliderShape::Box;
}

using SerializeFn = std::function<void(json&, Entity)>;
using DeserializeFn = std::function<void(const json&, Entity)>;

struct ComponentEntry {
    const char* key;
    SerializeFn serialize;
    DeserializeFn deserialize;
};

// Each entry writes/reads its own `key` under an entity's "components"
// object if (and only if) the entity has that component — adding a new
// component type to the engine means adding one entry here, not touching a
// central switch statement (PLAN.md §8.2, §15.2).
const std::vector<ComponentEntry>& componentRegistry() {
    static const std::vector<ComponentEntry> registry = [] {
        std::vector<ComponentEntry> r;

        // Tag/Transform are always present (Scene::createEntity adds them),
        // so deserialize mutates in place rather than calling addComponent.
        r.push_back({"Tag",
                     [](json& out, Entity e) {
                         out["Tag"] = {{"name", e.getComponent<TagComponent>().name}};
                     },
                     [](const json& in, Entity e) {
                         if (auto it = in.find("Tag"); it != in.end()) {
                             e.getComponent<TagComponent>().name = it->value("name", "Entity");
                         }
                     }});

        r.push_back({"Transform",
                     [](json& out, Entity e) {
                         auto& t = e.getComponent<TransformComponent>();
                         out["Transform"] = {{"position", vec3ToJson(t.position)},
                                              {"rotation", vec3ToJson(t.rotation)},
                                              {"scale", vec3ToJson(t.scale)}};
                     },
                     [](const json& in, Entity e) {
                         auto it = in.find("Transform");
                         if (it == in.end()) return;
                         auto& t = e.getComponent<TransformComponent>();
                         t.position = jsonToVec3((*it)["position"], t.position);
                         t.rotation = jsonToVec3((*it)["rotation"], t.rotation);
                         t.scale = jsonToVec3((*it)["scale"], t.scale);
                     }});

        r.push_back(
            {"MeshRenderer",
             [](json& out, Entity e) {
                 if (!e.hasComponent<MeshRendererComponent>()) return;
                 auto& mr = e.getComponent<MeshRendererComponent>();
                 if (!mr.mesh || !mr.material) return;

                 const auto& desc = mr.mesh->getDescriptor();
                 json meshJson = {{"source", meshSourceToString(desc.source)}};
                 if (desc.source == MeshSource::Model) {
                     meshJson["modelPath"] = desc.modelPath;
                     meshJson["meshIndex"] = desc.meshIndex;
                 } else if (desc.source != MeshSource::None) {
                     meshJson["size"] = desc.size;
                 }

                 json materialJson = {{"albedoColor", vec3ToJson(mr.material->albedoColor)}};
                 if (mr.material->shader) {
                     materialJson["vertexShader"] = mr.material->shader->getVertexPath();
                     materialJson["fragmentShader"] = mr.material->shader->getFragmentPath();
                 }
                 if (mr.material->albedoTexture) {
                     materialJson["albedoTexture"] = mr.material->albedoTexture->getPath();
                 }

                 out["MeshRenderer"] = {{"mesh", meshJson}, {"material", materialJson}};
             },
             [](const json& in, Entity e) {
                 auto it = in.find("MeshRenderer");
                 if (it == in.end()) return;

                 auto& resources = Application::get().getResourceManager();
                 auto& mr = e.addComponent<MeshRendererComponent>();

                 const json& meshJson = (*it)["mesh"];
                 MeshSource source = meshSourceFromString(meshJson.value("source", "None"));
                 if (source == MeshSource::Model) {
                     auto meshes = resources.loadModel(meshJson.value("modelPath", ""));
                     int index = meshJson.value("meshIndex", 0);
                     if (index >= 0 && static_cast<size_t>(index) < meshes.size()) {
                         mr.mesh = meshes[static_cast<size_t>(index)];
                     }
                 } else if (source == MeshSource::Cube) {
                     mr.mesh = Mesh::createCube(meshJson.value("size", 1.0f));
                 } else if (source == MeshSource::Plane) {
                     mr.mesh = Mesh::createPlane(meshJson.value("size", 1.0f));
                 }

                 const json& materialJson = (*it)["material"];
                 mr.material = std::make_shared<Material>();
                 mr.material->albedoColor =
                     jsonToVec3(materialJson.value("albedoColor", json::array({1, 1, 1})), {1, 1, 1});
                 std::string vs = materialJson.value("vertexShader", "");
                 std::string fs = materialJson.value("fragmentShader", "");
                 if (!vs.empty() && !fs.empty()) {
                     mr.material->shader = resources.loadShader(vs, fs);
                 }
                 std::string texPath = materialJson.value("albedoTexture", "");
                 if (!texPath.empty()) {
                     mr.material->albedoTexture = resources.loadTexture(texPath);
                 }
             }});

        r.push_back({"Camera",
                     [](json& out, Entity e) {
                         if (!e.hasComponent<CameraComponent>()) return;
                         auto& cam = e.getComponent<CameraComponent>();
                         out["Camera"] = {{"primary", cam.primary},
                                          {"position", vec3ToJson(cam.camera.getPosition())},
                                          {"pitch", cam.camera.getPitch()},
                                          {"yaw", cam.camera.getYaw()},
                                          {"fov", cam.camera.getFov()},
                                          {"aspect", cam.camera.getAspectRatio()},
                                          {"nearClip", cam.camera.getNearClip()},
                                          {"farClip", cam.camera.getFarClip()}};
                     },
                     [](const json& in, Entity e) {
                         auto it = in.find("Camera");
                         if (it == in.end()) return;
                         auto& cam = e.addComponent<CameraComponent>();
                         cam.primary = it->value("primary", true);
                         cam.camera = PerspectiveCamera(it->value("fov", 60.0f), it->value("aspect", 16.0f / 9.0f),
                                                         it->value("nearClip", 0.1f), it->value("farClip", 1000.0f));
                         cam.camera.setPosition(jsonToVec3((*it)["position"], {0, 0, 0}));
                         cam.camera.setPitchYaw(it->value("pitch", 0.0f), it->value("yaw", -90.0f));
                     }});

        r.push_back({"Light",
                     [](json& out, Entity e) {
                         if (!e.hasComponent<LightComponent>()) return;
                         auto& light = e.getComponent<LightComponent>();
                         out["Light"] = {
                             {"type", light.type == LightType::Directional ? "Directional" : "Point"},
                             {"color", vec3ToJson(light.color)},
                             {"intensity", light.intensity},
                             {"radius", light.radius}};
                     },
                     [](const json& in, Entity e) {
                         auto it = in.find("Light");
                         if (it == in.end()) return;
                         auto& light = e.addComponent<LightComponent>();
                         light.type = it->value("type", "Point") == "Directional" ? LightType::Directional
                                                                                    : LightType::Point;
                         light.color = jsonToVec3((*it)["color"], {1, 1, 1});
                         light.intensity = it->value("intensity", 1.0f);
                         light.radius = it->value("radius", 10.0f);
                     }});

        r.push_back({"RigidBody",
                     [](json& out, Entity e) {
                         if (!e.hasComponent<RigidBodyComponent>()) return;
                         auto& rb = e.getComponent<RigidBodyComponent>();
                         out["RigidBody"] = {{"type", bodyTypeToString(rb.type)}, {"mass", rb.mass}};
                     },
                     [](const json& in, Entity e) {
                         auto it = in.find("RigidBody");
                         if (it == in.end()) return;
                         auto& rb = e.addComponent<RigidBodyComponent>();
                         rb.type = bodyTypeFromString(it->value("type", "Static"));
                         rb.mass = it->value("mass", 1.0f);
                     }});

        r.push_back({"Collider",
                     [](json& out, Entity e) {
                         if (!e.hasComponent<ColliderComponent>()) return;
                         auto& col = e.getComponent<ColliderComponent>();
                         out["Collider"] = {{"shape", colliderShapeToString(col.shape)},
                                            {"halfExtents", vec3ToJson(col.halfExtents)},
                                            {"radius", col.radius},
                                            {"height", col.height},
                                            {"friction", col.friction},
                                            {"restitution", col.restitution},
                                            {"isTrigger", col.isTrigger}};
                     },
                     [](const json& in, Entity e) {
                         auto it = in.find("Collider");
                         if (it == in.end()) return;
                         auto& col = e.addComponent<ColliderComponent>();
                         col.shape = colliderShapeFromString(it->value("shape", "Box"));
                         col.halfExtents = jsonToVec3((*it)["halfExtents"], {0.5f, 0.5f, 0.5f});
                         col.radius = it->value("radius", 0.5f);
                         col.height = it->value("height", 1.0f);
                         col.friction = it->value("friction", 0.6f);
                         col.restitution = it->value("restitution", 0.0f);
                         col.isTrigger = it->value("isTrigger", false);
                     }});

        r.push_back({"CharacterController",
                     [](json& out, Entity e) {
                         if (!e.hasComponent<CharacterControllerComponent>()) return;
                         auto& cc = e.getComponent<CharacterControllerComponent>();
                         out["CharacterController"] = {
                             {"speed", cc.speed}, {"radius", cc.radius}, {"height", cc.height}};
                     },
                     [](const json& in, Entity e) {
                         auto it = in.find("CharacterController");
                         if (it == in.end()) return;
                         auto& cc = e.addComponent<CharacterControllerComponent>();
                         cc.speed = it->value("speed", 3.0f);
                         cc.radius = it->value("radius", 0.4f);
                         cc.height = it->value("height", 1.8f);
                     }});

        r.push_back({"Script",
                     [](json& out, Entity e) {
                         if (!e.hasComponent<ScriptComponent>()) return;
                         out["Script"] = {{"scriptPath", e.getComponent<ScriptComponent>().scriptPath}};
                     },
                     [](const json& in, Entity e) {
                         auto it = in.find("Script");
                         if (it == in.end()) return;
                         e.addComponent<ScriptComponent>().scriptPath = it->value("scriptPath", "");
                     }});

        return r;
    }();
    return registry;
}

}  // namespace

bool SceneSerializer::serialize(Scene& scene, const std::string& path) {
    auto& registry = scene.getRegistry();

    std::unordered_map<entt::entity, int> indexOf;
    int nextIndex = 0;
    for (auto entity : registry.view<TagComponent>()) indexOf[entity] = nextIndex++;

    json root;
    root["molly_version"] = "0.8.0";
    root["scene_name"] = std::filesystem::path(path).stem().string();
    root["entities"] = json::array();

    for (auto entity : registry.view<TagComponent>()) {
        Entity e(entity, &scene);
        json components;
        for (auto& entry : componentRegistry()) entry.serialize(components, e);

        if (auto* h = registry.try_get<HierarchyComponent>(entity)) {
            json hierarchyJson;
            hierarchyJson["parent"] = (h->parent != entt::null) ? json(indexOf.at(h->parent)) : json();
            json children = json::array();
            for (auto child : h->children) children.push_back(indexOf.at(child));
            hierarchyJson["children"] = children;
            components["Hierarchy"] = hierarchyJson;
        }

        root["entities"].push_back({{"id", indexOf.at(entity)}, {"components", components}});
    }

    std::ofstream file(path);
    if (!file) {
        MOLLY_ERROR("Failed to open scene file for writing: {0}", path);
        return false;
    }
    file << root.dump(2);
    MOLLY_INFO("Saved scene: {0} ({1} entities)", path, indexOf.size());
    return true;
}

bool SceneSerializer::deserialize(const std::string& path, Scene& scene) {
    std::ifstream file(path);
    if (!file) {
        MOLLY_ERROR("Failed to open scene file: {0}", path);
        return false;
    }

    json root;
    try {
        file >> root;
    } catch (const std::exception& ex) {
        MOLLY_ERROR("Malformed scene JSON \"{0}\": {1}", path, ex.what());
        return false;
    }

    if (!root.contains("entities")) {
        MOLLY_ERROR("Scene file \"{0}\" has no \"entities\" array", path);
        return false;
    }

    std::unordered_map<int, entt::entity> indexToEntity;
    for (auto& entityJson : root["entities"]) {
        int id = entityJson.value("id", 0);
        indexToEntity[id] = scene.createEntity();
    }

    for (auto& entityJson : root["entities"]) {
        int id = entityJson.value("id", 0);
        entt::entity handle = indexToEntity.at(id);
        Entity e(handle, &scene);
        const json& components = entityJson.value("components", json::object());

        for (auto& entry : componentRegistry()) entry.deserialize(components, e);

        if (auto it = components.find("Hierarchy"); it != components.end()) {
            auto& h = scene.getRegistry().get<HierarchyComponent>(handle);
            if (!(*it)["parent"].is_null()) {
                h.parent = indexToEntity.at((*it)["parent"].get<int>());
            }
            for (auto& child : (*it)["children"]) {
                h.children.push_back(indexToEntity.at(child.get<int>()));
            }
        }
    }

    MOLLY_INFO("Loaded scene: {0} ({1} entities)", path, indexToEntity.size());
    return true;
}

bool SceneSerializer::serializePrefab(Entity entity, const std::string& path) {
    if (!entity) return false;

    // A prefab file is just a scene fragment containing the entity and its
    // subtree (PLAN.md §15.3) — reuse the exact same component registry,
    // walking children via HierarchyComponent instead of the whole registry.
    // (Implemented as a minimal single/no-children path for now: full
    // recursive subtree export is a straightforward extension once a real
    // multi-entity prefab is needed.)
    json root;
    root["molly_version"] = "0.8.0";
    root["entities"] = json::array();

    json components;
    for (auto& entry : componentRegistry()) entry.serialize(components, entity);
    root["entities"].push_back({{"id", 0}, {"components", components}});

    std::ofstream file(path);
    if (!file) {
        MOLLY_ERROR("Failed to open prefab file for writing: {0}", path);
        return false;
    }
    file << root.dump(2);
    MOLLY_INFO("Saved prefab: {0}", path);
    return true;
}

Entity SceneSerializer::instantiatePrefab(Scene& scene, const std::string& path,
                                           const glm::vec3& position) {
    std::ifstream file(path);
    if (!file) {
        MOLLY_ERROR("Failed to open prefab file: {0}", path);
        return {};
    }

    json root;
    try {
        file >> root;
    } catch (const std::exception& ex) {
        MOLLY_ERROR("Malformed prefab JSON \"{0}\": {1}", path, ex.what());
        return {};
    }

    if (!root.contains("entities") || root["entities"].empty()) return {};

    const json& components = root["entities"][0].value("components", json::object());
    Entity entity = scene.createEntity();
    for (auto& entry : componentRegistry()) entry.deserialize(components, entity);

    entity.getComponent<TransformComponent>().position = position;
    return entity;
}

}  // namespace molly
