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

# --- spdlog -----------------------------------------------------------------
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(spdlog)
