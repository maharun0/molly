#pragma once

#include <memory>
#include <string>
#include <vector>

#include "molly/renderer/Mesh.h"

namespace molly {

class ModelLoader {
public:
    // Loads every mesh in the file, flattened into one list (no scene-graph
    // preservation in v1 — sufficient for the demo game's simple props).
    static std::vector<std::shared_ptr<Mesh>> load(const std::string& path);
};

}  // namespace molly
