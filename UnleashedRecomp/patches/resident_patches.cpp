#include <cpu/guest_code.h>
#include "config.h"

const char* m_pStageID;

void GetStageIDMidAsmHook(PPCRegister& r5)
{
    m_pStageID = *(xpointer<const char>*)g_memory.Translate(r5.u32);
}

// Loading screen hook.
PPC_FUNC_IMPL(__imp__sub_824DCF38);
PPC_FUNC(sub_824DCF38)
{
    /* Force the Werehog transition ID
       to use a different transition. */
    if (!Config::WerehogHubTransformVideo)
    {
        /*
            0 - Tails Electric NOW LOADING
            1 - No Transition
            2 - Werehog Transition
            3 - Tails Electric NOW LOADING w/ Info (requires context)
            4 - Arrows In/Out
            5 - NOW LOADING
            6 - Event Gallery
            7 - NOW LOADING
            8 - Black Screen
        */
        if (ctx.r4.u32 == 2)
            ctx.r4.u32 = 4;
    }

    if (m_pStageID)
    {
        /* Fix restarting Eggmanland as the Werehog
           erroneously using the Event Gallery transition. */
        if (ctx.r4.u32 == 6 && !strcmp(m_pStageID, "Act_EggmanLand"))
            ctx.r4.u32 = 5;
    }

    __imp__sub_824DCF38(ctx, base);
}

// CApplicationDocument::LoadArchiveDatabases
PPC_FUNC_IMPL(__imp__sub_824EFD28);
PPC_FUNC(sub_824EFD28)
{
    auto r3 = ctx.r3;

    // CSigninXenon::InitializeDLC
    ctx.r3.u64 = PPC_LOAD_U32(r3.u32 + 4) + 200;
    ctx.r4.u64 = 0;
    sub_822C57D8(ctx, base);

    ctx.r3 = r3;
    __imp__sub_824EFD28(ctx, base);
}

// CFileReaderXenon_DLC::InitializeParallel
PPC_FUNC(sub_822C3778)
{
    if (!PPC_LOAD_U8(0x83361F10)) // ms_DLCInitialized
    {
        // CFileReaderXenon_DLC::InitializeSerial
        sub_822C2CF0(ctx, base);
    }
}

// HasAnyDLC
// NOTE: This is not used outside title screen outro, so it's safe to hook.
PPC_FUNC(sub_82BD06C8)
{
    ctx.r3.u64 = 0;
}
