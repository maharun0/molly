#pragma once

#include <string>

#include <sol/sol.hpp>

namespace molly {

struct ScriptComponent {
    std::string scriptPath;

    // Runtime-only; not meant to be hand-authored or serialized.
    sol::table instance;
    bool started = false;
};

}  // namespace molly
