//
// Created by koerriva on 2021/12/20.
//

#include "log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace wx {
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init(bool async) {
        spdlog::set_pattern("%^[%T.%e] %n-%t: %v%$");
//        spdlog::set_pattern("[%H:%M:%S.%m %z] [%n] [%^---%L---%$] [thread %t] %v");
//        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%s:%#:%t] %v");

        if(async){
            spdlog::init_thread_pool(10000,1);
            s_CoreLogger = spdlog::stderr_color_mt<spdlog::async_factory>("ENGINE");
            s_ClientLogger = spdlog::stderr_color_mt<spdlog::async_factory>("APP");
        }else{
            s_CoreLogger = spdlog::stdout_color_mt("ENGINE");
            s_ClientLogger = spdlog::stderr_color_mt("APP");
        }

        s_CoreLogger->set_level(spdlog::level::trace);
        s_ClientLogger->set_level(spdlog::level::trace);
    }
}