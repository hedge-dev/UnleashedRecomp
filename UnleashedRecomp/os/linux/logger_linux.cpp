#include <os/logger_detail.h>

void os::logger::detail::Init()
{
}

void os::logger::detail::Log(const std::string_view str, detail::ELogType type, const char* func)
{
    if (func)
    {
        fmt::println("[{}] {}", func, str);
    }
    else
    {
        fmt::println("{}", str);
    }
}
