#pragma once

#include <memory>

#include <spdlog/spdlog.h>

namespace molly {

class Log {
public:
    static void init();

    static std::shared_ptr<spdlog::logger>& getEngineLogger() { return s_engineLogger; }
    static std::shared_ptr<spdlog::logger>& getAppLogger() { return s_appLogger; }

private:
    static std::shared_ptr<spdlog::logger> s_engineLogger;
    static std::shared_ptr<spdlog::logger> s_appLogger;
};

}  // namespace molly

#define MOLLY_TRACE(...)    ::molly::Log::getEngineLogger()->trace(__VA_ARGS__)
#define MOLLY_INFO(...)     ::molly::Log::getEngineLogger()->info(__VA_ARGS__)
#define MOLLY_WARN(...)     ::molly::Log::getEngineLogger()->warn(__VA_ARGS__)
#define MOLLY_ERROR(...)    ::molly::Log::getEngineLogger()->error(__VA_ARGS__)
#define MOLLY_CRITICAL(...) ::molly::Log::getEngineLogger()->critical(__VA_ARGS__)

#define APP_TRACE(...)    ::molly::Log::getAppLogger()->trace(__VA_ARGS__)
#define APP_INFO(...)     ::molly::Log::getAppLogger()->info(__VA_ARGS__)
#define APP_WARN(...)     ::molly::Log::getAppLogger()->warn(__VA_ARGS__)
#define APP_ERROR(...)    ::molly::Log::getAppLogger()->error(__VA_ARGS__)
#define APP_CRITICAL(...) ::molly::Log::getAppLogger()->critical(__VA_ARGS__)
