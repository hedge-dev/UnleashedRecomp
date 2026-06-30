#include <os/logger.h>

void os::logger::Init()
{
}

void os::logger::Log(const std::string_view str, ELogType type, const char* func)
{
    (void)type;

    if (func)
    {
        fmt::println("[{}] {}", func, str);
    }
    else
    {
        fmt::println("{}", str);
    }
}
