#include <os/host.h>

#if defined(__x86_64__) || defined(_M_X64)
#include <cpuid/cpuinfo.hpp>
#endif

bool os::host::IsCapableCPU()
{
#if defined(__x86_64__) || defined(_M_X64)
    cpuid::cpuinfo cpuid;
    return cpuid.has_avx();
#else
    return true;
#endif
}
