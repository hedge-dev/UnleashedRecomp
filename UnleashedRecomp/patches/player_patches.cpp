#include "player_patches.h"
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

// RT's rising edge, recomputed once per frame in PlayerPatches::Update.
static bool g_rtBoostPrev = false;
static bool g_rtBoostRisingEdge = false;

// RT boost - the surgical hook (Config::RightTriggerAction == Boost)
static thread_local bool g_inBoostEntry = false;  // sub_823787B0 (tapped -> start)
static thread_local bool g_inBoostUpdate = false; // sub_8239E3B8  (held  -> sustain)
static thread_local bool g_inDiveDecision = false; // sub_82391898 / sub_82391408 (held -> dive)
static thread_local bool g_inSuperBoostEntry = false;  // sub_82451F10 (Super Sonic tapped -> start)
static thread_local bool g_inSuperBoostUpdate = false; // sub_82452518 (Super Sonic held -> sustain)

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

// "Native" Right Trigger Boost Support

static bool RTBoostActive()
{
    bool userSelectedBoost = Config::RightTriggerAction == ERightTriggerAction::Boost;
    return userSelectedBoost && App::s_rtBoost.load(std::memory_order_relaxed);
}

PPC_FUNC_IMPL(__imp__sub_823787B0);
PPC_FUNC(sub_823787B0)
{
    g_inBoostEntry = true;
    __imp__sub_823787B0(ctx, base);
    g_inBoostEntry = false;
}

// Air action decision (tapped-gated, then airboost/homing).
PPC_FUNC_IMPL(__imp__sub_82375748);
PPC_FUNC(sub_82375748)
{
    g_inBoostEntry = RTBoostActive();
    __imp__sub_82375748(ctx, base);
    g_inBoostEntry = false;
}

// Like the XButtonHoming flag, pressing right trigger in the air will always air boost 
// (leaving homing attack button preference in tact).
PPC_FUNC_IMPL(__imp__sub_8245DB18);
PPC_FUNC(sub_8245DB18)
{
    uint32_t id = ctx.r4.u32;
    __imp__sub_8245DB18(ctx, base);
    if (id == 294 && g_inBoostEntry && g_rtBoostRisingEdge)
        ctx.r3.u64 = 0; // 0 -> air boost, 1 -> homing attack.
}

PPC_FUNC_IMPL(__imp__sub_8239E3B8);
PPC_FUNC(sub_8239E3B8)
{
    g_inBoostUpdate = true;
    __imp__sub_8239E3B8(ctx, base);
    g_inBoostUpdate = false;
}

// Tapped: drive boost START from RT. Scoped to the boost entry decisions -
// day Sonic's sub_823787B0 (any id) and Super Sonic's flight update sub_82451F10 (which
// reads isTapped(action 4) to transition into the boost).
PPC_FUNC_IMPL(__imp__sub_82308360);
PPC_FUNC(sub_82308360)
{
    uint32_t id = ctx.r4.u32;
    __imp__sub_82308360(ctx, base);

    bool userConfigIsBoost = Config::RightTriggerAction == ERightTriggerAction::Boost;

    if (g_rtBoostRisingEdge && userConfigIsBoost && (g_inBoostEntry || (g_inSuperBoostEntry && id == 4)))
    {
        ctx.r3.u64 = 1;
    }
}

// Held: keep boost alive from RT (boost state update), and drive the
// falling-section DIVE from RT. The X button path is unchanged
PPC_FUNC_IMPL(__imp__sub_82308378);
PPC_FUNC(sub_82308378)
{
    uint32_t id = ctx.r4.u32;
    __imp__sub_82308378(ctx, base);
    if (RTBoostActive() && (g_inBoostUpdate
        || (g_inDiveDecision && id == 0x0B)
        || (g_inSuperBoostUpdate && id == 4)))
    {
        ctx.r3.u64 = 1;
    }
}

// Falling-section dive: glide state DivingFloat::Update Scope RT into both 
// so holding RT dives; the game shows the boost=RT prompt in these sections,
// so RT diving matches the prompt
PPC_FUNC_IMPL(__imp__sub_82391898);
PPC_FUNC(sub_82391898)
{
    g_inDiveDecision = true;
    __imp__sub_82391898(ctx, base);
    g_inDiveDecision = false;
}

PPC_FUNC_IMPL(__imp__sub_82391408);
PPC_FUNC(sub_82391408)
{
    g_inDiveDecision = true;
    __imp__sub_82391408(ctx, base);
    g_inDiveDecision = false;
}

/* --- super Sonic (final boss) boost ---------------------------------------------------
   the final boss runs Super Sonic through two flight modes, each its own state machine
   with a "tapped -> start" flight decision and a "held -> sustain" boost update, both
   reading boost-button action 4, so RT starts and sustains Super Sonic's
   boost in both phases, exactly like day Sonic */
PPC_FUNC_IMPL(__imp__sub_82451F10);   // Space Harrier: flight -> boost (tapped)
PPC_FUNC(sub_82451F10)
{
    g_inSuperBoostEntry = true;
    __imp__sub_82451F10(ctx, base);
    g_inSuperBoostEntry = false;
}

PPC_FUNC_IMPL(__imp__sub_82452518);   // Space Harrier: boost sustain (held)
PPC_FUNC(sub_82452518)
{
    g_inSuperBoostUpdate = true;
    __imp__sub_82452518(ctx, base);
    g_inSuperBoostUpdate = false;
}

PPC_FUNC_IMPL(__imp__sub_82451038);   // NiGHTS: flight -> boost (tapped)
PPC_FUNC(sub_82451038)
{
    g_inSuperBoostEntry = true;
    __imp__sub_82451038(ctx, base);
    g_inSuperBoostEntry = false;
}

PPC_FUNC_IMPL(__imp__sub_824514D8);   // NiGHTS: boost sustain (held)
PPC_FUNC(sub_824514D8)
{
    g_inSuperBoostUpdate = true;
    __imp__sub_824514D8(ctx, base);
    g_inSuperBoostUpdate = false;
}

void PlayerPatches::Update()
{
    // RT rising edge for the boost-entry override (see the sub_82308360 hook): true
    // only on the frame RT crosses into "held", so boost fires once per press.
    {
        bool rt = RTBoostActive();
        g_rtBoostRisingEdge = rt && !g_rtBoostPrev;
        g_rtBoostPrev = rt;
    }
}
