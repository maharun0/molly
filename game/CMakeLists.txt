file(GLOB_RECURSE MOLLY_GAME_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(molly-demo ${MOLLY_GAME_SOURCES})

target_link_libraries(molly-demo PRIVATE molly-engine molly_warnings)

set_target_properties(molly-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/game"
)

# See editor/CMakeLists.txt — the exe chdirs to its own directory at
# startup and resolves asset/log paths relative to that.
add_custom_command(TARGET molly-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:molly-demo>/assets"
)
