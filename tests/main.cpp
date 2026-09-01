// Custom Catch2 entry point (instead of Catch2WithMain) so engine logging is
// initialized before any test runs — every MOLLY_INFO/WARN/ERROR call
// dereferences a null spdlog logger otherwise, since Log::init() is
// normally called by each app's own main(), which this test binary doesn't
// have (see tests/CMakeLists.txt).

#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>

#include <molly/core/Log.h>

int main(int argc, char* argv[]) {
    molly::Log::init();
    return Catch::Session().run(argc, argv);
}
