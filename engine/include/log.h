//
// Created by koerriva on 2021/12/20.
//

#ifndef WX_LOG_H
#define WX_LOG_H
#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace wx {
    class Log {
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    public:
        static void Init();
        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
    };
}

#define WX_CORE_TRACE(...)		::wx::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define WX_CORE_INFO(...)		::wx::Log::GetCoreLogger()->info(__VA_ARGS__)
#define WX_CORE_WARN(...)		::wx::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define WX_CORE_ERROR(...)		::wx::Log::GetCoreLogger()->error(__VA_ARGS__)
#define WX_CORE_CRITICAL(...)	::wx::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define WX_TRACE(...)		::wx::Log::GetClientLogger()->trace(__VA_ARGS__)
#define WX_INFO(...)		::wx::Log::GetClientLogger()->info(__VA_ARGS__)
#define WX_WARN(...)		::wx::Log::GetClientLogger()->warn(__VA_ARGS__)
#define WX_ERROR(...)		::wx::Log::GetClientLogger()->error(__VA_ARGS__)
#define WX_CRITICAL(...)	::wx::Log::GetClientLogger()->critical(__VA_ARGS__)

#endif //WX_LOG_H
