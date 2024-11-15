#include <cpu/guest_code.h>
#include <api/SWA.h>
#include <ui/window.h>
#include <ui/window_events.h>
#include <cfg/config.h>

uint32_t m_lastCheckpointScore = 0;
float m_lastDarkGaiaEnergy = 0.0f;
bool m_isUnleashCancelled = false;

/* Hook function for when checkpoints are activated
   to preserve the current checkpoint score. */
PPC_FUNC_IMPL(__imp__sub_82624308);
PPC_FUNC(sub_82624308)
{
    __imp__sub_82624308(ctx, base);

    if (Config::ScoreBehaviour != EScoreBehaviour::CheckpointRetain)
        return;

    auto pGameDocument = SWA::CGameDocument::GetInstance();

    if (!pGameDocument)
        return;

    m_lastCheckpointScore = pGameDocument->m_pMember->m_Score;
}

/* Hook function that resets the score
   and restore the last checkpoint score. */
PPC_FUNC_IMPL(__imp__sub_8245F048);
PPC_FUNC(sub_8245F048)
{
    __imp__sub_8245F048(ctx, base);

    if (Config::ScoreBehaviour != EScoreBehaviour::CheckpointRetain)
        return;

    auto pGameDocument = SWA::CGameDocument::GetInstance();

    if (!pGameDocument)
        return;

    printf("[*] Resetting score to %d\n", m_lastCheckpointScore);

    pGameDocument->m_pMember->m_Score = m_lastCheckpointScore;
}

void ResetScoreOnRestartMidAsmHook()
{
    m_lastCheckpointScore = 0;
}

// Dark Gaia energy change hook.
PPC_FUNC_IMPL(__imp__sub_823AF7A8);
PPC_FUNC(sub_823AF7A8)
{
    auto pEvilSonicContext = (SWA::Player::CEvilSonicContext*)g_memory.Translate(ctx.r3.u32);

    m_lastDarkGaiaEnergy = pEvilSonicContext->m_DarkGaiaEnergy;

    // Don't drain energy if out of control.
    if (!Config::UnleashOutOfControlDrain && pEvilSonicContext->m_OutOfControlCount && ctx.f1.f64 < 0.0)
        return;

    __imp__sub_823AF7A8(ctx, base);

    if (!Config::UnleashCancel)
        return;

    auto pInputState = SWA::CInputState::GetInstance();

    // Don't allow cancelling Unleash if the intro anim is still playing.
    if (!pInputState || pEvilSonicContext->m_AnimationID == 39)
        return;

    if (pInputState->GetPadState().IsTapped(SWA::eKeyState_RightBumper))
    {
        pEvilSonicContext->m_DarkGaiaEnergy = 0.0f;
        m_isUnleashCancelled = true;
    }
}

void PostUnleashMidAsmHook(PPCRegister& r30)
{
    if (m_isUnleashCancelled)
    {
        if (auto pEvilSonicContext = (SWA::Player::CEvilSonicContext*)g_memory.Translate(r30.u32))
            pEvilSonicContext->m_DarkGaiaEnergy = m_lastDarkGaiaEnergy;

        m_isUnleashCancelled = false;
    }
}

void SetXButtonHomingMidAsmHook(PPCRegister& r30)
{
    r30.u32 = Config::XButtonHoming;
}

// SWA::Player::CEvilSonicContext::Ctor
PPC_FUNC_IMPL(__imp__sub_823B49D8);
PPC_FUNC(sub_823B49D8)
{
    __imp__sub_823B49D8(ctx, base);

    SDL_User_EvilSonic(Window::s_pWindow, true);
}

// SWA::Player::CEvilSonicContext::Dtor
PPC_FUNC_IMPL(__imp__sub_823B4590);
PPC_FUNC(sub_823B4590)
{
    __imp__sub_823B4590(ctx, base);

    SDL_User_EvilSonic(Window::s_pWindow, false);
}
