cmake_minimum_required(VERSION 3.28)
project(Molly LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug)
endif()

option(MOLLY_BUILD_TESTS "Build the Catch2 test suite" OFF)

include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)

add_subdirectory(vendor)
add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(game)

if (MOLLY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
