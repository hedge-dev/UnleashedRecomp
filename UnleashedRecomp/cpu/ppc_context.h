#pragma once

inline thread_local PPCContext* g_ppcContext;

inline PPCContext* GetPPCContext()
{
    return g_ppcContext;
}

inline void SetPPCContext(PPCContext& ctx)
{
    g_ppcContext = &ctx;
}
