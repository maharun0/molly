# --- GLFW -------------------------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# --- GLAD (generated, checked in) -------------------------------------
add_library(glad STATIC glad/src/gl.c)
target_include_directories(glad PUBLIC glad/include)

# --- GLM ----------------------------------------------------------------
add_subdirectory(glm)

# --- entt (header-only) --------------------------------------------------
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE entt/single_include)

# --- Dear ImGui (docking branch, hand-rolled target) ----------------------
add_library(molly-imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(molly-imgui PUBLIC imgui imgui/backends)
target_link_libraries(molly-imgui PUBLIC glfw glad)

# --- stb_image (header-only) --------------------------------------------
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# --- Jolt Physics ---------------------------------------------------------
# Building as a subdirectory (not the top-level project) already makes
# Jolt's own CMakeLists.txt skip its unit tests/samples/viewer targets.
# USE_STATIC_MSVC_RUNTIME_LIBRARY defaults ON, which links Jolt against the
# static CRT (/MT) while every other target here uses the dynamic CRT
# (/MD, CMake's default) — that mismatch caused duplicate CRT symbol
# errors (LNK2005) at final link time. Force it off so Jolt matches.
set(DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(JoltPhysics/Build)

# --- Lua (hand-rolled C library target; official source has no CMake) ---
file(GLOB LUA_SOURCES lua/*.c)
list(REMOVE_ITEM LUA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/lua.c      # standalone interpreter's main(), not needed as a library
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/onelua.c   # alternate amalgamated single-TU build; would duplicate symbols
    ${CMAKE_CURRENT_SOURCE_DIR}/lua/ltests.c   # internal test harness, inert unless LUA_USER_H defines it on
)
add_library(molly-lua STATIC ${LUA_SOURCES})
target_include_directories(molly-lua PUBLIC lua)
set_source_files_properties(${LUA_SOURCES} PROPERTIES LANGUAGE C)

# --- sol2 (header-only; wired directly rather than via its own CMake, ----
# which tries to find/link a system Lua we don't want it searching for)
add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE sol2/include)
target_link_libraries(sol2 INTERFACE molly-lua)

# --- nlohmann/json (header-only) -----------------------------------------
add_library(json INTERFACE)
target_include_directories(json INTERFACE json/single_include)

# --- Assimp (importer list trimmed to what the demo game needs) ---------
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
add_subdirectory(assimp)

# --- Catch2 (test framework) -----------------------------------------------
if (MOLLY_BUILD_TESTS)
    add_subdirectory(catch2)
    # Catch2's own CMakeLists exports its extras/ dir (Catch.cmake, needed by
    # tests/CMakeLists.txt's `include(Catch)`) to CMAKE_MODULE_PATH via
    # PARENT_SCOPE, which only reaches this file's scope (vendor/) — forward
    # it one more level so the sibling tests/ subdirectory can see it too.
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endif()

# --- spdlog -----------------------------------------------------------------
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(spdlog)
