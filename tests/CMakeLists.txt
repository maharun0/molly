file(GLOB_RECURSE MOLLY_TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(molly-tests ${MOLLY_TEST_SOURCES})

# Catch2 (not Catch2WithMain) — main.cpp provides its own main() so it can
# call molly::Log::init() before any test runs; every MOLLY_INFO/ERROR call
# dereferences a null logger otherwise (Log::init() is normally called by
# each app's own main(), which a test binary doesn't have).
target_link_libraries(molly-tests PRIVATE molly-engine Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(molly-tests)
