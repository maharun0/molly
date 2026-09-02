file(GLOB_RECURSE MOLLY_EDITOR_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(molly-editor ${MOLLY_EDITOR_SOURCES})

target_link_libraries(molly-editor PRIVATE molly-engine molly_warnings)

set_target_properties(molly-editor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/editor"
)

# The exe chdirs to its own directory at startup (PlatformUtils.h) and
# resolves every asset/log path relative to that — so assets/ needs to
# actually exist next to it, regardless of where it's launched from.
add_custom_command(TARGET molly-editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:molly-editor>/assets"
)
