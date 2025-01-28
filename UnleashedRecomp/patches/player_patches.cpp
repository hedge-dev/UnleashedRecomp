#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/config.h>
#include <os/logger.h>
#include <app.h>
#include <sdl_events.h>

static uint32_t g_lastEnemyScore;
static uint32_t g_lastTrickScore;
static float g_lastDarkGaiaEnergy;
static bool g_isUnleashCancelled;

/* Hook function for when checkpoints are activated
   to preserve the current checkpoint score. */
PPC_FUNC_IMPL(__imp__sub_82624308);
PPC_FUNC(sub_82624308)
{
    __imp__sub_82624308(ctx, base);

    if (!Config::SaveScoreAtCheckpoints)
        return;

    if (auto pGameDocument = SWA::CGameDocument::GetInstance())
    {
        g_lastEnemyScore = pGameDocument->m_pMember->m_ScoreInfo.EnemyScore;
        g_lastTrickScore = pGameDocument->m_pMember->m_ScoreInfo.TrickScore;

        LOGFN("Score: {}", g_lastEnemyScore + g_lastTrickScore);
    }
}

/* Hook function that resets the score
   and restore the last checkpoint score. */
PPC_FUNC_IMPL(__imp__sub_8245F048);
PPC_FUNC(sub_8245F048)
{
    __imp__sub_8245F048(ctx, base);

    if (!Config::SaveScoreAtCheckpoints)
        return;

    if (auto pGameDocument = SWA::CGameDocument::GetInstance())
    {
        LOGFN("Score: {}", g_lastEnemyScore + g_lastTrickScore);

        pGameDocument->m_pMember->m_ScoreInfo.EnemyScore = g_lastEnemyScore;
        pGameDocument->m_pMember->m_ScoreInfo.TrickScore = g_lastTrickScore;
    }
}

void ResetScoreOnRestartMidAsmHook()
{
    g_lastEnemyScore = 0;
    g_lastTrickScore = 0;
}

// Dark Gaia energy change hook.
PPC_FUNC_IMPL(__imp__sub_823AF7A8);
PPC_FUNC(sub_823AF7A8)
{
    auto pEvilSonicContext = (SWA::Player::CEvilSonicContext*)g_memory.Translate(ctx.r3.u32);

    g_lastDarkGaiaEnergy = pEvilSonicContext->m_DarkGaiaEnergy;

    // Don't drain energy if out of control.
    if (Config::FixUnleashOutOfControlDrain && pEvilSonicContext->m_OutOfControlCount && ctx.f1.f64 < 0.0)
        return;

    __imp__sub_823AF7A8(ctx, base);

    if (!Config::AllowCancellingUnleash)
        return;

    auto pInputState = SWA::CInputState::GetInstance();

    // Don't allow cancelling Unleash if the intro anim is still playing.
    if (!pInputState || pEvilSonicContext->m_AnimationID == 39)
        return;

    if (pInputState->GetPadState().IsTapped(SWA::eKeyState_RightBumper))
    {
        pEvilSonicContext->m_DarkGaiaEnergy = 0.0f;
        g_isUnleashCancelled = true;
    }
}

void PostUnleashMidAsmHook(PPCRegister& r30)
{
    if (!g_isUnleashCancelled)
        return;

    if (auto pEvilSonicContext = (SWA::Player::CEvilSonicContext*)g_memory.Translate(r30.u32))
        pEvilSonicContext->m_DarkGaiaEnergy = std::max(0.0f, g_lastDarkGaiaEnergy - 35.0f);

    g_isUnleashCancelled = false;
}

void SetXButtonHomingMidAsmHook(PPCRegister& r30)
{
    r30.u32 = Config::HomingAttackOnBoost;
}

// SWA::Player::CEvilSonicContext
PPC_FUNC_IMPL(__imp__sub_823B49D8);
PPC_FUNC(sub_823B49D8)
{
    __imp__sub_823B49D8(ctx, base);

    App::s_isWerehog = true;

    SDL_User_EvilSonic(true);
}

// ~SWA::Player::CEvilSonicContext
PPC_FUNC_IMPL(__imp__sub_823B4590);
PPC_FUNC(sub_823B4590)
{
    __imp__sub_823B4590(ctx, base);

    App::s_isWerehog = false;

    SDL_User_EvilSonic(false);
}
