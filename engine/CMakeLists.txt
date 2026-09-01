file(GLOB_RECURSE MOLLY_ENGINE_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(molly-engine STATIC ${MOLLY_ENGINE_SOURCES})

target_include_directories(molly-engine PUBLIC include)

target_link_libraries(molly-engine
    PUBLIC
        glfw
        glad
        glm::glm
        entt
        molly-imgui
        spdlog::spdlog
        stb
        assimp::assimp
    PRIVATE
        molly_warnings
        molly_sanitizers
)

target_compile_definitions(molly-engine PUBLIC
    $<$<CONFIG:Debug>:MOLLY_DEBUG>
)
