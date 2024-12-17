#include <stdafx.h>
#include "code_cache.h"
#include "ppc_context.h"

CodeCache::CodeCache()
{
#ifdef _WIN32
    bucket = (char*)VirtualAlloc(nullptr, 0x200000000, MEM_RESERVE, PAGE_READWRITE);
    assert(bucket != nullptr);
#else
    bucket = (char*)mmap(NULL, 0x200000000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    assert(bucket != (char*)MAP_FAILED);
#endif
}

CodeCache::~CodeCache()
{
#ifdef _WIN32
    VirtualFree(bucket, 0, MEM_RELEASE);
#else
    munmap(bucket, 0x200000000);
#endif
}

void CodeCache::Init()
{
    for (size_t i = 0; PPCFuncMappings[i].guest != 0; i++)
    {
        if (PPCFuncMappings[i].host != nullptr)
        {
#ifdef _WIN32
            VirtualAlloc(bucket + PPCFuncMappings[i].guest * 2, sizeof(void*), MEM_COMMIT, PAGE_READWRITE);
#endif
            *(void**)(bucket + PPCFuncMappings[i].guest * 2) = (void*)PPCFuncMappings[i].host;
        }
    }
}

void CodeCache::Insert(uint32_t guest, PPCFunc* host)
{
#ifdef _WIN32
    VirtualAlloc(bucket + static_cast<uint64_t>(guest) * 2, sizeof(void*), MEM_COMMIT, PAGE_READWRITE);
#endif
    *reinterpret_cast<PPCFunc**>(bucket + static_cast<uint64_t>(guest) * 2) = host;
}

void* CodeCache::Find(uint32_t guest) const
{
    return *reinterpret_cast<void**>(bucket + static_cast<uint64_t>(guest) * 2);
}

SWA_API PPCFunc* KeFindHostFunction(uint32_t guest)
{
    return reinterpret_cast<PPCFunc*>(g_codeCache.Find(guest));
}

SWA_API void KeInsertHostFunction(uint32_t guest, PPCFunc* function)
{
    g_codeCache.Insert(guest, function);
}
