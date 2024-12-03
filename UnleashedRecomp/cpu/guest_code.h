#pragma once
#include "ppc_context.h"
#include <kernel/memory.h>

struct GuestCode 
{
    inline static void Run(void* hostAddress, PPCContext* ctx, void* baseAddress)
    {
        ctx->fpscr.loadFromHost();
        reinterpret_cast<PPCFunc*>(hostAddress)(*ctx, reinterpret_cast<uint8_t*>(baseAddress));
    }

    inline static void Run(void* hostAddress, PPCContext* ctx)
    {
        ctx->fpscr.loadFromHost();
        reinterpret_cast<PPCFunc*>(hostAddress)(*ctx, reinterpret_cast<uint8_t*>(g_memory.base));
    }

    inline static void Run(void* hostAddress)
    {
        Run(hostAddress, GetPPCContext());
    }
};
