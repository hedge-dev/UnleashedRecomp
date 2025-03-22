#include "preload_executable.h"
#include <os/logger.h>

// Code from Zelda 64: Recompiled
// https://github.com/Zelda64Recomp/Zelda64Recomp/blob/91db87632c2bfb6995ef1554ec71b11977c621f8/src/main/main.cpp#L440-L514

PreloadContext::~PreloadContext()
{
#ifdef _WIN32
    if (preloaded)
    {
        VirtualUnlock(view, size);
        CloseHandle(mappingHandle);
        CloseHandle(handle);
    }
#endif
}

void PreloadContext::PreloadExecutable()
{
#ifdef _WIN32
    wchar_t moduleName[MAX_PATH];
    GetModuleFileNameW(NULL, moduleName, MAX_PATH);

    handle = CreateFileW(moduleName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) 
    {
        LOG_ERROR("Failed to load executable into memory!");
        *this = {};
        return;
    }

    LARGE_INTEGER moduleSize;
    if (!GetFileSizeEx(handle, &moduleSize))
    {
        LOG_ERROR("Failed to get size of executable!");
        CloseHandle(handle);
        *this = {};
        return;
    }

    size = moduleSize.QuadPart;

    mappingHandle = CreateFileMappingW(handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (mappingHandle == nullptr) 
    {
        LOG_ERROR("Failed to create file mapping of executable!");
        CloseHandle(handle);
        *this = {};
        return;
    }

    view = MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0);
    if (view == nullptr)
    {
        LOG_ERROR("Failed to map view of of executable!");
        CloseHandle(mappingHandle);
        CloseHandle(handle);
        *this = {};
        return;
    }

    DWORD pid = GetCurrentProcessId();
    HANDLE processHandle = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (processHandle == nullptr) 
    {
        LOG_ERROR("Failed to open own process!");
        CloseHandle(mappingHandle);
        CloseHandle(handle);
        *this = {};
        return;
    }

    SIZE_T minimumSetSize, maximumSetSize;
    if (!GetProcessWorkingSetSize(processHandle, &minimumSetSize, &maximumSetSize))
    {
        LOG_ERROR("Failed to get working set size!");
        CloseHandle(mappingHandle);
        CloseHandle(handle);
        *this = {};
        return;
    }

    if (!SetProcessWorkingSetSize(processHandle, minimumSetSize + size, maximumSetSize + size)) 
    {
        LOG_ERROR("Failed to set working set size!");
        CloseHandle(mappingHandle);
        CloseHandle(handle);
        *this = {};
        return;
    }

    if (VirtualLock(view, size) == 0) 
    {
        LOGF_ERROR("Failed to lock view of executable! (Error: 0x{:X})\n", GetLastError());
        CloseHandle(mappingHandle);
        CloseHandle(handle);
        *this = {};
        return;
    }

    preloaded = true;
#endif
}
