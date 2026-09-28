#include <os/logger.h>
#if TARGET_OS_IPHONE
#include <os/log.h>
#endif
void os::logger::Init()
{
}

void os::logger::Log(const std::string_view str, ELogType type, const char* func)
{
    #if TARGET_OS_IPHONE
        if (func)
        {
            os_log(OS_LOG_DEFAULT, "%{public}s: %{public}s", func, str.data());
        }
        else
        {
            os_log(OS_LOG_DEFAULT, "%{public}s", str.data());
        }
    #else
        if (func)
        {
            fmt::println("[{}] {}", func, str);
        }
        else
        {
            fmt::println("{}", str);
        }
    #endif
}
