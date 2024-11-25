#pragma once

class Memory
{
public:
    char* base{};
    size_t size{};
    size_t guestBase{};

    Memory(void* address, size_t size);

    void* Alloc(size_t offset, size_t size, uint32_t type);

    void* Commit(size_t offset, size_t size);
    void* Reserve(size_t offset, size_t size);

    void* Translate(size_t offset) const noexcept
    {
        if (offset)
            assert(offset < 0x100000000ull);

        return base + offset;
    }

    uint32_t MapVirtual(void* host) const noexcept
    {
        if (host)
            assert(host >= base && host < (base + size));

        return static_cast<uint32_t>(static_cast<char*>(host) - base);
    }
};

extern "C" void* MmGetHostAddress(uint32_t ptr);
extern Memory g_memory;
