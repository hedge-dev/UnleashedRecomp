#include <api/SWA.h>
#include <app.h>
#include <ui/game_window.h>
#include <user/achievement_manager.h>
#include <user/persistent_storage_manager.h>
#include <user/config.h>

void AchievementManagerUnlockMidAsmHook(PPCRegister& id)
{
    AchievementManager::Unlock(id.u32);
}

bool DisableHintsMidAsmHook()
{
    return !Config::Hints;
}

// Disable Perfect Dark Gaia hints.
PPC_FUNC_IMPL(__imp__sub_82AC36E0);
PPC_FUNC(sub_82AC36E0)
{
    auto pPerfectDarkGaiaChipHintName = (xpointer<char>*)g_memory.Translate(0x8338EF10);

    strcpy(pPerfectDarkGaiaChipHintName->get(), Config::Hints ? "V_CHP_067\0" : "end\0");

    __imp__sub_82AC36E0(ctx, base);
}

bool DisableControlTutorialMidAsmHook()
{
    return !Config::ControlTutorial;
}

bool DisableEvilControlTutorialMidAsmHook(PPCRegister& r4, PPCRegister& r5)
{
    if (Config::ControlTutorial)
        return true;

    // Only allow enemy QTE prompts to get through.
    return r4.u32 == 1 && r5.u32 == 1;
}

// The day-stage boost navigation sign (sub_8239F4A8) hardcodes cast "btn_1" with img
// pattern 2 (X button). When boost is remapped to the right trigger, redirect the
// prompt to the trigger cast so the HUD matches the actual control. The play-screen
// scene's button casts map as: btn_1 = face buttons (0=A 1=B 2=X 3=Y), btn_2 = bumpers
// (0=LB 1=RB), btn_3 = triggers (0=LT 1=RT).
void BoostPromptCastMidAsmHook(PPCRegister& r5)
{
    if (Config::RightTriggerAction == ERightTriggerAction::Boost)
        r5.u32 = 0x820116D8; // "btn_3"
}

void BoostPromptPatternMidAsmHook(PPCRegister& r4)
{
    if (Config::RightTriggerAction == ERightTriggerAction::Boost)
        r4.u32 = 1; // RT
}

// The Gaia Colossus (Chip) control overlay ("Boost / Guard / Attack") lives in the
// "footer" scene of ui_playscreen_su.yncp (a common archive, cached from boot), with
// the button art baked into the asset as sprite crops of the shared x360 button atlas
// (crop order A, B, X, Y, LB, RB, LT, RT; bumpers are double width).
void ChipOverlayCsdPatchMidAsmHook(PPCRegister& r8)
{
    auto base = (uint8_t*)g_memory.Translate(r8.u32);

    // Identify ui_playscreen_su.yncp first: container size in the FAPC header, then
    // the footer cast name table. Everything else returns before touching config.
    if (*(be<uint32_t>*)(base + 4) != 0x4970)
        return;

    static constexpr char s_footerCastNames[] = "btn_lb\0\0btn_lt\0\0btn_rt\0\0btn_x";
    if (memcmp(base + 0xC08, s_footerCastNames, sizeof(s_footerCastNames) - 1) != 0)
        return;

    if (Config::RightTriggerAction != ERightTriggerAction::Boost)
        return;

    // Layout constants, hand-tuned in-game (widths/heights are quad half-extents in
    // scene units; X/Y are cast translations). AttackL* is btn_lt's offset relative
    // to its parent btn_rt (the pair's spacing); the others are absolute.
    struct QuadLayout { float w, h, x, y; };
    constexpr QuadLayout kBoost  = { 0.0308f, 0.0556f,  -0.1289f, -0.103f  };
    constexpr QuadLayout kGuard  = { 0.031f,  0.0502f,  -0.1369f, -0.0545f };
    constexpr QuadLayout kAttackR = { 0.0625f, 0.05556f, -0.1064f, -0.011f };
    constexpr float kAttackLX = -0.055f; // btn_lt X relative to btn_rt
    constexpr float kAttackLY = 0.0f;    // btn_lt Y relative to btn_rt

    // The footer's button casts (records at 0x1C30 btn_x, 0x1CA4 btn_lb, 0x1D18
    // btn_lt, 0x1D8C btn_rt) each show their icon via a sprite-index array
    // ([0, 0, crop], live entry at +8) into the scene crop table (entry 0 at file
    // 0x734, 20-byte records): 13 = X, 15 = LB (double width), 16 = RB, 17 = LT,
    // 18 = RT. Redirect the indices instead of rewriting the shared crop table.
    *(be<uint32_t>*)(base + 0x3570 + 8) = 18; // Boost row:  btn_x  X  -> RT
    *(be<uint32_t>*)(base + 0x362C + 8) = 17; // Guard row:  btn_lb LB -> LT
    *(be<uint32_t>*)(base + 0x36E8 + 8) = 15; // Attack row: btn_lt LT -> LB
    *(be<uint32_t>*)(base + 0x37A4 + 8) = 16; //             btn_rt RT -> RB

    // Position each cast via its quad corners (8 floats at +0x14: TL BL TR BR as x,y
    // pairs, right-edge / vertical-centre anchored) and its info-block translation
    // (+0x0C x, +0x10 y). btn_lt is a CHILD of btn_rt, so its translation is relative.
    auto setQuad = [&](uint32_t rec, const QuadLayout& q)
    {
        *(be<float>*)(base + rec + 0x14) = -q.w;        // TL.x
        *(be<float>*)(base + rec + 0x1C) = -q.w;        // BL.x
        *(be<float>*)(base + rec + 0x18) = -q.h * 0.5f; // TL.y
        *(be<float>*)(base + rec + 0x28) = -q.h * 0.5f; // TR.y
        *(be<float>*)(base + rec + 0x20) = q.h * 0.5f;  // BL.y
        *(be<float>*)(base + rec + 0x30) = q.h * 0.5f;  // BR.y
    };

    QuadLayout attackL = { kAttackR.w, kAttackR.h, kAttackLX, kAttackLY };
    setQuad(0x1C30, kBoost);   // btn_x
    setQuad(0x1CA4, kGuard);   // btn_lb
    setQuad(0x1D18, attackL);  // btn_lt
    setQuad(0x1D8C, kAttackR); // btn_rt

    *(be<float>*)(base + 0x35F0 + 0x0C) = kBoost.x;
    *(be<float>*)(base + 0x35F0 + 0x10) = kBoost.y;
    *(be<float>*)(base + 0x36AC + 0x0C) = kGuard.x;
    *(be<float>*)(base + 0x36AC + 0x10) = kGuard.y;
    *(be<float>*)(base + 0x3768 + 0x0C) = attackL.x;
    *(be<float>*)(base + 0x3768 + 0x10) = attackL.y;
    *(be<float>*)(base + 0x3824 + 0x0C) = kAttackR.x;
    *(be<float>*)(base + 0x3824 + 0x10) = kAttackR.y;

    // The rendered shape comes from each cast's baked pixel box (u32 width at +0x58,
    // height at +0x5C) — the art is fitted into it, which is what stretched the
    // swapped icons. Give each cast the box of the art it now shows: the guard cast
    // drops from the 80x40 bumper box to the 40x40 trigger box, the attack casts
    // grow from 40x40 to 80x40.
    *(be<uint32_t>*)(base + 0x1CA4 + 0x58) = 40; // btn_lb: square trigger box
    *(be<uint32_t>*)(base + 0x1D18 + 0x58) = 80; // btn_lt: wide bumper box
    *(be<uint32_t>*)(base + 0x1D8C + 0x58) = 80; // btn_rt: wide bumper box
}

// Stamp the time whenever a prompt is on screen (this+101 = active, this+102 = done).
// The input driver keeps the boost trigger remap alive during a QTE only while the
// trigger is held continuously from before the prompt (so the boost aura persists).
// The QTE is edge-triggered, so a held trigger registers no press and can't answer it.
// Releasing the trigger suspends the remap so the real face button can.
void QTEPromptActiveMidAsmHook(PPCRegister& pThis)
{
    bool isActive = *(uint8_t*)g_memory.Translate(pThis.u32 + 101) != 0;
    bool isDone = *(uint8_t*)g_memory.Translate(pThis.u32 + 102) != 0;
    if (isActive && !isDone)
        App::s_lastQTEPromptMs = uint32_t(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

bool DisableDLCIconMidAsmHook()
{
    return Config::DisableDLCIcon;
}

void WerehogBattleMusicMidAsmHook(PPCRegister& r11)
{
    if (Config::BattleTheme)
        return;

    // Swap CStateBattle for CStateNormal.
    if (r11.u8 == 4)
        r11.u8 = 3;
}

bool UseAlternateTitleMidAsmHook()
{
    auto isSWA = Config::Language == ELanguage::Japanese;

    if (Config::UseAlternateTitle)
        isSWA = !isSWA;

    return isSWA;
}

/* Hook function that gets the game region
   and force result to zero for Japanese
   to display the correct logos. */
PPC_FUNC_IMPL(__imp__sub_825197C0);
PPC_FUNC(sub_825197C0)
{
    if (Config::Language == ELanguage::Japanese)
    {
        ctx.r3.u64 = 0;
        return;
    }

    __imp__sub_825197C0(ctx, base);
}

// Logo skip
PPC_FUNC_IMPL(__imp__sub_82547DF0);
PPC_FUNC(sub_82547DF0)
{
    if (Config::SkipIntroLogos)
    {
        ctx.r4.u64 = 0;
        ctx.r5.u64 = 0;
        ctx.r6.u64 = 1;
        ctx.r7.u64 = 0;
        sub_825517C8(ctx, base);
    }
    else
    {
        __imp__sub_82547DF0(ctx, base);
    }
}

/* Ignore xercesc::EmptyStackException to
   allow DLC stages with invalid XML to load. */
PPC_FUNC_IMPL(__imp__sub_8305D5B8);
PPC_FUNC(sub_8305D5B8)
{
    auto value = PPC_LOAD_U32(ctx.r3.u32 + 4);

    if (!value)
        return;

    __imp__sub_8305D5B8(ctx, base);
}

// Disable auto save warning.
PPC_FUNC_IMPL(__imp__sub_82586698);
PPC_FUNC(sub_82586698)
{
    if (Config::DisableAutoSaveWarning)
        *(bool*)g_memory.Translate(0x83367BC2) = true;

    __imp__sub_82586698(ctx, base);
}

// SWA::CObjHint::MsgNotifyObjectEvent::Impl
// Disable only certain hints from hint volumes.
// This hook should be used to allow hint volumes specifically to also prevent them from affecting the player.
PPC_FUNC_IMPL(__imp__sub_82736E80);
PPC_FUNC(sub_82736E80)
{
    // GroupID parameter text
    auto* groupId = (const char*)(base + PPC_LOAD_U32(ctx.r3.u32 + 0x100));
    
    if (!Config::Hints)
    {
        // WhiteIsland_ACT1_001: "Your friend went off that way, Sonic. Quick, let's go after him!"
        // s20n_mykETF_c_navi_2: "Huh? Weird! We can't get through here anymore. We were able to earlier!"
        if (strcmp(groupId, "WhiteIsland_ACT1_001") != 0 && strcmp(groupId, "s20n_mykETF_c_navi_2") != 0)
            return;
    }

    __imp__sub_82736E80(ctx, base);
}

// SWA::CHelpWindow::MsgRequestHelp::Impl
// Disable only certain hints from other sequences.
// This hook should be used to block hint messages from unknown sources.
PPC_FUNC_IMPL(__imp__sub_824C1E60);
PPC_FUNC(sub_824C1E60)
{
    auto pMsgRequestHelp = (SWA::Message::MsgRequestHelp*)(base + ctx.r4.u32);

    if (!Config::Hints)
    {
        // s10d_mykETF_c_navi: "Looks like we can get to a bunch of places in the village from here!"
        if (strcmp(pMsgRequestHelp->m_Name.c_str(), "s10d_mykETF_c_navi") == 0)
            return;
    }

    __imp__sub_824C1E60(ctx, base);
}

// This function is called in various places but primarily for the boost filter
// when the second argument (r4) is set to "boost". Whilst boosting the third argument (f1)
// will go up to 1.0f and then down to 0.0f as the player lets off of the boost button.
// To avoid the boost filter from kicking in at all if the function is called with "boost"
// we set the third argument to zero no matter what (if the code is on).
PPC_FUNC_IMPL(__imp__sub_82B4DB48);
PPC_FUNC(sub_82B4DB48)
{
    if (Config::DisableBoostFilter && strcmp((const char*)(base + ctx.r4.u32), "boost") == 0)
    {
        ctx.f1.f64 = 0.0;
    }

    __imp__sub_82B4DB48(ctx, base);
}

// DLC save data flag check.
// 
// The DLC checks are fundamentally broken in this game, resulting in this method always
// returning true and displaying the DLC info message when it shouldn't be.
// 
// The original intent here seems to have been to display the message every time new DLC
// content is installed, but the flags in the save data never get written to properly,
// causing this function to always pass in some way.
//
// We bypass the save data completely and write to external persistent storage to store
// whether we've seen the DLC info message instead. This way we can retain the original
// broken game behaviour, whilst also providing a fix for this issue that is safe.
PPC_FUNC_IMPL(__imp__sub_824EE620);
PPC_FUNC(sub_824EE620)
{
    __imp__sub_824EE620(ctx, base);

    ctx.r3.u32 = PersistentStorageManager::ShouldDisplayDLCMessage(true);
}
