#include "stdafx.h"
#include <kernel/function.h>

BOOL QueryPerformanceCounterImpl(LARGE_INTEGER* lpPerformanceCount)
{
    lpPerformanceCount->QuadPart = ByteSwap(std::chrono::steady_clock::now().time_since_epoch().count());
    return TRUE;
}

BOOL QueryPerformanceFrequencyImpl(LARGE_INTEGER* lpFrequency)
{
    constexpr auto Frequency = std::chrono::steady_clock::period::den / std::chrono::steady_clock::period::num;
    lpFrequency->QuadPart = ByteSwap(Frequency);
    return TRUE;
}

DWORD GetTickCountImpl()
{
    return DWORD(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
}

void GlobalMemoryStatusImpl(XLPMEMORYSTATUS lpMemoryStatus)
{
    lpMemoryStatus->dwLength = sizeof(XMEMORYSTATUS);
    lpMemoryStatus->dwMemoryLoad = 0;
    lpMemoryStatus->dwTotalPhys = 0x20000000;
    lpMemoryStatus->dwAvailPhys = 0x20000000;
    lpMemoryStatus->dwTotalPageFile = 0x20000000;
    lpMemoryStatus->dwAvailPageFile = 0x20000000;
    lpMemoryStatus->dwTotalVirtual = 0x20000000;
    lpMemoryStatus->dwAvailVirtual = 0x20000000;
}

GUEST_FUNCTION_HOOK(sub_831B0ED0, memcpy);
GUEST_FUNCTION_HOOK(sub_831CCB98, memcpy);
GUEST_FUNCTION_HOOK(sub_831CEAE0, memcpy);
GUEST_FUNCTION_HOOK(sub_831CEE04, memcpy);
GUEST_FUNCTION_HOOK(sub_831CF2D0, memcpy);
GUEST_FUNCTION_HOOK(sub_831CF660, memcpy);
GUEST_FUNCTION_HOOK(sub_831B1358, memcpy);
GUEST_FUNCTION_HOOK(sub_831B5E00, memmove);
GUEST_FUNCTION_HOOK(sub_831B0BA0, memset);
GUEST_FUNCTION_HOOK(sub_831CCAA0, memset);

GUEST_FUNCTION_HOOK(sub_82BD4CA8, OutputDebugStringA);

GUEST_FUNCTION_HOOK(sub_82BD4AC8, QueryPerformanceCounterImpl);
GUEST_FUNCTION_HOOK(sub_831CD040, QueryPerformanceFrequencyImpl);
GUEST_FUNCTION_HOOK(sub_831CDAD0, GetTickCountImpl);

GUEST_FUNCTION_HOOK(sub_82BD4BC0, GlobalMemoryStatusImpl);

// sprintf
PPC_FUNC(sub_82BD4AE8)
{
    sub_831B1630(ctx, base);
}
