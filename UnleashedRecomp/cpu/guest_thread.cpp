#include <stdafx.h>
#include "guest_thread.h"
#include <kernel/memory.h>
#include <kernel/heap.h>
#include <kernel/function.h>
#include "code_cache.h"
#include "guest_code.h"
#include "ppc_context.h"

constexpr size_t PCR_SIZE = 0xAB0;
constexpr size_t TLS_SIZE = 0x100;
constexpr size_t TEB_SIZE = 0x2E0;
constexpr size_t STACK_SIZE = 0x40000;
constexpr size_t TOTAL_SIZE = PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE;

constexpr size_t TEB_OFFSET = PCR_SIZE + TLS_SIZE;

GuestThreadContext::GuestThreadContext(uint32_t cpuNumber)
{
    assert(thread == nullptr);

    thread = (uint8_t*)g_userHeap.Alloc(TOTAL_SIZE);
    memset(thread, 0, TOTAL_SIZE);

    *(uint32_t*)thread = std::byteswap(g_memory.MapVirtual(thread + PCR_SIZE)); // tls pointer
    *(uint32_t*)(thread + 0x100) = std::byteswap(g_memory.MapVirtual(thread + PCR_SIZE + TLS_SIZE)); // teb pointer
    *(thread + 0x10C) = cpuNumber;

    *(uint32_t*)(thread + PCR_SIZE + 0x10) = 0xFFFFFFFF; // that one TLS entry that felt quirky
    *(uint32_t*)(thread + PCR_SIZE + TLS_SIZE + 0x14C) = std::byteswap(GetCurrentThreadId()); // thread id

    ppcContext.fn = (uint8_t*)g_codeCache.bucket;
    ppcContext.r1.u64 = g_memory.MapVirtual(thread + PCR_SIZE + TLS_SIZE + TEB_SIZE + STACK_SIZE); // stack pointer
    ppcContext.r13.u64 = g_memory.MapVirtual(thread);

    assert(GetPPCContext() == nullptr);
    SetPPCContext(ppcContext);
}

GuestThreadContext::~GuestThreadContext()
{
    g_userHeap.Free(thread);
}

DWORD GuestThread::Start(uint32_t function)
{
    const GuestThreadParameter parameter{ function };
    return Start(parameter);
}

DWORD GuestThread::Start(const GuestThreadParameter& parameter)
{
    const auto procMask = (uint8_t)(parameter.flags >> 24);
    const auto cpuNumber = procMask == 0 ? 0 : 7 - std::countl_zero(procMask);

    GuestThreadContext ctx(cpuNumber);
    ctx.ppcContext.r3.u64 = parameter.value;

    GuestCode::Run(g_codeCache.Find(parameter.function), &ctx.ppcContext, g_memory.Translate(0));

    return (DWORD)ctx.ppcContext.r3.u64;
}

DWORD HostThreadStart(void* pParameter)
{
    auto* parameter = static_cast<GuestThreadParameter*>(pParameter);
    const auto result = GuestThread::Start(*parameter);

    delete parameter;
    return result;
}

HANDLE GuestThread::Start(uint32_t function, uint32_t parameter, uint32_t flags, LPDWORD threadId)
{
    const auto hostCreationFlags = (flags & 1) != 0 ? CREATE_SUSPENDED : 0;
    //return CreateThread(nullptr, 0, Start, (void*)((uint64_t(parameter) << 32) | function), suspended ? CREATE_SUSPENDED : 0, threadId);
    return CreateThread(nullptr, 0, HostThreadStart, new GuestThreadParameter{ function, parameter, flags }, hostCreationFlags, threadId);
}

void GuestThread::SetThreadName(uint32_t id, const char* name)
{
#pragma pack(push,8)
    const DWORD MS_VC_EXCEPTION = 0x406D1388;

    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = id;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

void GuestThread::SetLastError(DWORD error)
{
    auto* thread = (char*)g_memory.Translate(GetPPCContext()->r13.u32);
    if (*(DWORD*)(thread + 0x150))
    {
        // Program doesn't want errors
        return;
    }

    // TEB + 0x160 : Win32LastError
    *(DWORD*)(thread + TEB_OFFSET + 0x160) = std::byteswap(error);
}

PPCContext* GuestThread::Invoke(uint32_t address)
{
    auto* ctx = GetPPCContext();
    GuestCode::Run(g_codeCache.Find(address), ctx);

    return ctx;
}

void SetThreadNameImpl(uint32_t a1, uint32_t threadId, uint32_t* name)
{
    GuestThread::SetThreadName(threadId, (const char*)g_memory.Translate(std::byteswap(*name)));
}

int GetThreadPriorityImpl(uint32_t hThread)
{
    return GetThreadPriority((HANDLE)hThread);
}

DWORD SetThreadIdealProcessorImpl(uint32_t hThread, DWORD dwIdealProcessor)
{
    return SetThreadIdealProcessor((HANDLE)hThread, dwIdealProcessor);
}

GUEST_FUNCTION_HOOK(sub_82DFA2E8, SetThreadNameImpl);
GUEST_FUNCTION_HOOK(sub_82BD57A8, GetThreadPriorityImpl);
GUEST_FUNCTION_HOOK(sub_82BD5910, SetThreadIdealProcessorImpl);

GUEST_FUNCTION_STUB(sub_82BD58F8); // Some function that updates the TEB, don't really care since the field is not set
