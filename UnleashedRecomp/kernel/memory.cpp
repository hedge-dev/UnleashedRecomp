#include <stdafx.h>
#include "memory.h"

Memory::Memory(void* address, size_t size) : size(size)
{
#ifdef _WIN32
    base = (char*)VirtualAlloc(address, size, MEM_RESERVE, PAGE_READWRITE);

    if (base == nullptr)
        base = (char*)VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_READWRITE);
#else
    base = (char*)mmap(address, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    if (base == (char*)MAP_FAILED)
        base = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    mprotect(base, 4096, PROT_NONE);
#endif
}

void* Memory::Alloc(size_t offset, size_t size, uint32_t type)
{
#ifdef _WIN32
    return VirtualAlloc(base + offset, size, type, PAGE_READWRITE);
#else
    return base + offset;
#endif
}

void* Memory::Commit(size_t offset, size_t size)
{
#ifdef _WIN32
    return Alloc(offset, size, MEM_COMMIT);
#else
    return base + offset;
#endif
}

void* Memory::Reserve(size_t offset, size_t size)
{
#ifdef _WIN32
    return Alloc(offset, size, MEM_RESERVE);
#else
    return base + offset;
#endif
}

void* MmGetHostAddress(uint32_t ptr)
{
    return g_memory.Translate(ptr);
}
