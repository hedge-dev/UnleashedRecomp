#pragma once

struct PreloadContext
{
#ifdef _WIN32
    HANDLE handle{};
    HANDLE mappingHandle{};
    SIZE_T size{};
    PVOID view{};
    bool preloaded{};
#endif

    ~PreloadContext();
    void PreloadExecutable();
};
