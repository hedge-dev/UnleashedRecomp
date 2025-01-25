#include <api/SWA.h>
#include <patches/CTitleStateIntro_patches.h>

// SWA::CGameModeStageTitle::Update
PPC_FUNC_IMPL(__imp__sub_825518B8);
PPC_FUNC(sub_825518B8)
{
    auto pGameModeStageTitle = (SWA::CGameModeStageTitle*)g_memory.Translate(ctx.r3.u32);

    __imp__sub_825518B8(ctx, base);

    if (g_quitMessageOpen)
        pGameModeStageTitle->m_AdvertiseMovieWaitTime = 0;
}
