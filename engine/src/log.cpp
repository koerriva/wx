//
// Created by koerriva on 2021/12/20.
//

#include "log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace wx {
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init() {
        spdlog::set_pattern("%^[%T.%e] %n-%t: %v%$");
//        spdlog::set_pattern("[%H:%M:%S.%m %z] [%n] [%^---%L---%$] [thread %t] %v");
//        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#:%t] %v");

        s_CoreLogger = spdlog::stdout_color_mt("ENGINE");
        s_CoreLogger->set_level(spdlog::level::trace);

        s_ClientLogger = spdlog::stderr_color_mt("APP");
        s_ClientLogger->set_level(spdlog::level::trace);
    }
}