#include "molly/core/PlatformUtils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdio>
#include <filesystem>

namespace molly {

// Must run before Log::init() (the log file itself is a cwd-relative path,
// "logs/molly.log"), so this can't use MOLLY_ERROR/spdlog — plain stderr is
// the only sink available this early.
void setWorkingDirectoryToExecutable() {
    wchar_t path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        std::fprintf(stderr, "Failed to resolve executable path for working-directory setup\n");
        return;
    }

    std::filesystem::path exeDir = std::filesystem::path(path).parent_path();
    std::error_code ec;
    std::filesystem::current_path(exeDir, ec);
    if (ec) {
        std::fprintf(stderr, "Failed to set working directory to %s: %s\n",
                      exeDir.string().c_str(), ec.message().c_str());
    }
}

}  // namespace molly
