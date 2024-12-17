#pragma once

struct CodeCache
{
    char* bucket{};

    CodeCache();
    ~CodeCache();

    void Init();
    void Insert(uint32_t guest, PPCFunc* host);

    void* Find(uint32_t guest) const;
};

SWA_API PPCFunc* KeFindHostFunction(uint32_t guest);
SWA_API void KeInsertHostFunction(uint32_t guest, PPCFunc* function);

extern CodeCache g_codeCache;
