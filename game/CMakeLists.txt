file(GLOB_RECURSE MOLLY_GAME_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(molly-demo ${MOLLY_GAME_SOURCES})

target_link_libraries(molly-demo PRIVATE molly-engine molly_warnings)

set_target_properties(molly-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/game"
)
