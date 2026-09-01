#include "molly/core/Log.h"

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace molly {

std::shared_ptr<spdlog::logger> Log::s_engineLogger;
std::shared_ptr<spdlog::logger> Log::s_appLogger;

void Log::init() {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/molly.log", 1024 * 1024 * 5, 3));

    sinks[0]->set_pattern("%^[%T] %n: %v%$");
    sinks[1]->set_pattern("[%T] [%l] %n: %v");

    s_engineLogger = std::make_shared<spdlog::logger>("MOLLY", sinks.begin(), sinks.end());
    spdlog::register_logger(s_engineLogger);
    s_engineLogger->set_level(spdlog::level::trace);
    s_engineLogger->flush_on(spdlog::level::trace);

    s_appLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
    spdlog::register_logger(s_appLogger);
    s_appLogger->set_level(spdlog::level::trace);
    s_appLogger->flush_on(spdlog::level::trace);
}

}  // namespace molly
