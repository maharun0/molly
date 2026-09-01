file(GLOB_RECURSE MOLLY_EDITOR_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(molly-editor ${MOLLY_EDITOR_SOURCES})

target_link_libraries(molly-editor PRIVATE molly-engine molly_warnings)

set_target_properties(molly-editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/editor"
)
