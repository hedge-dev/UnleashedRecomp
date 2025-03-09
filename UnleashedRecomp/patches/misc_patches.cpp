#include <api/SWA.h>
#include <ui/game_window.h>
#include <user/achievement_manager.h>
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

// Remove boost filter
void DisableBoostFilterMidAsmHook(PPCRegister& r11)
{
    if (Config::DisableBoostFilter)
    {
        if (r11.u32 == 1)
            r11.u32 = 0;
    }
}
