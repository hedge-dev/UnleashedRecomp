#pragma once

struct PPCContext;
struct GuestThreadParameter
{
    uint32_t function;
    uint32_t value;
    uint32_t flags;
};

struct GuestThreadContext
{
    PPCContext ppcContext{};
    uint8_t* thread = nullptr;

    GuestThreadContext(uint32_t cpuNumber);
    ~GuestThreadContext();
};

struct GuestThread
{
    static DWORD Start(uint32_t function);
    static DWORD Start(const GuestThreadParameter& parameter);
    static HANDLE Start(uint32_t function, uint32_t parameter, uint32_t flags, LPDWORD threadId);

    static void SetThreadName(uint32_t id, const char* name);
    static void SetLastError(DWORD error);
    static PPCContext* Invoke(uint32_t address);
};
