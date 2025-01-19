#pragma once

#include <source_location>

#define LOG_IMPL(type, func, str)       os::logger::Log(str, os::logger::ELogType::type, func)
#define LOGF_IMPL(type, func, str, ...) os::logger::Log(fmt::format(str, __VA_ARGS__), os::logger::ELogType::type, func)

// Function-specific logging.

#define LOG(str)               LOG_IMPL(None, __func__, str)
#define LOG_WARNING(str)       LOG_IMPL(Warning, __func__, str)
#define LOG_ERROR(str)         LOG_IMPL(Error, __func__, str)

#if _DEBUG
#define LOG_UTILITY(str)       LOG_IMPL(Utility, __func__, str)
#else
#define LOG_UTILITY(str)
#endif

#define LOGF(str, ...)         LOGF_IMPL(None, __func__, str, __VA_ARGS__)
#define LOGF_WARNING(str, ...) LOGF_IMPL(Warning, __func__, str, __VA_ARGS__)
#define LOGF_ERROR(str, ...)   LOGF_IMPL(Error, __func__, str, __VA_ARGS__)

#if _DEBUG
#define LOGF_UTILITY(str, ...) LOGF_IMPL(Utility, __func__, str, __VA_ARGS__)
#else
#define LOGF_UTILITY(str, ...)
#endif

// Non-function-specific logging.

#define LOGN(str)               LOG_IMPL(None, "*", str)
#define LOGN_WARNING(str)       LOG_IMPL(Warning, "*", str)
#define LOGN_ERROR(str)         LOG_IMPL(Error, "*", str)

#if _DEBUG
#define LOGN_UTILITY(str)       LOG_IMPL(Utility, "*", str)
#else
#define LOGN_UTILITY(str)
#endif

#define LOGFN(str, ...)         LOGF_IMPL(None, "*", str, __VA_ARGS__)
#define LOGFN_WARNING(str, ...) LOGF_IMPL(Warning, "*", str, __VA_ARGS__)
#define LOGFN_ERROR(str, ...)   LOGF_IMPL(Error, "*", str, __VA_ARGS__)

#if _DEBUG
#define LOGFN_UTILITY(str, ...) LOGF_IMPL(Utility, "*", str, __VA_ARGS__)
#else
#define LOGFN_UTILITY(str, ...)
#endif

namespace os::logger
{
    enum class ELogType
    {
        None,
        Utility,
        Warning,
        Error
    };

    void Init();
    void Log(const std::string_view str, ELogType type = ELogType::None, const char* func = nullptr);
}
