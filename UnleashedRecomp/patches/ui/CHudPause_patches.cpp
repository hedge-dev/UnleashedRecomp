#include <cpu/guest_code.h>
#include <kernel/function.h>
#include <api/SWA.h>
#include <ui/achievement_menu.h>
#include <ui/options_menu.h>
#include <app.h>

float g_achievementMenuIntroTime = 0.0f;
constexpr float g_achievementMenuIntroThreshold = 3.0f;
float g_achievementMenuOutroTime = 0.0f;
constexpr float g_achievementMenuOutroThreshold = 0.32f;
bool g_isAchievementMenuOutro = false;

void CHudPauseAddOptionsItemMidAsmHook(PPCRegister& pThis)
{
    guest_stack_var<Hedgehog::Base::CSharedString> menu("TopMenu");
    guest_stack_var<Hedgehog::Base::CSharedString> name("option");

    GuestToHostFunction<int>(0x824AE690, pThis.u32, menu.get(), name.get());
}

bool InjectMenuBehaviour(uint32_t pThis, uint32_t count)
{
    auto pHudPause = (SWA::CHudPause*)g_memory.Translate(pThis);
    auto cursorIndex = *(be<uint32_t>*)g_memory.Translate(4 * (*(be<uint32_t>*)g_memory.Translate(pThis + 0x19C) + 0x68) + pThis);

    auto actionType = SWA::eActionType_Undefined;
    auto transitionType = SWA::eTransitionType_Undefined;

    switch (pHudPause->m_Menu)
    {
        case SWA::eMenuType_WorldMap:
        case SWA::eMenuType_Stage:
        case SWA::eMenuType_Misc:
            actionType = SWA::eActionType_Return;
            transitionType = SWA::eTransitionType_Quit;
            break;

        case SWA::eMenuType_Village:
        case SWA::eMenuType_Hub:
            actionType = SWA::eActionType_Return;
            transitionType = SWA::eTransitionType_Hide;
            break;
    }

    if (auto pInputState = SWA::CInputState::GetInstance())
    {
        if (pInputState->GetPadState().IsTapped(SWA::eKeyState_Select))
        {
            AchievementMenu::Open();
    
            pHudPause->m_Action = SWA::eActionType_Undefined;
            pHudPause->m_Transition = SWA::eTransitionType_SubMenu;
    
            return false;
        }
    }

    if (pHudPause->m_Status == SWA::eStatusType_Accept)
    {
        if (cursorIndex == count - 2)
        {
            OptionsMenu::Open(true, pHudPause->m_Menu);

            pHudPause->m_Action = SWA::eActionType_Undefined;
            pHudPause->m_Transition = SWA::eTransitionType_Hide;

            return true;
        }
        else if (cursorIndex == count - 1)
        {
            pHudPause->m_Action = actionType;
            pHudPause->m_Transition = transitionType;

            return true;
        }
    }

    return false;
}

bool CHudPauseItemCountMidAsmHook(PPCRegister& pThis, PPCRegister& count)
{
    count.u32 += 1;

    return InjectMenuBehaviour(pThis.u32, count.u32);
}

void CHudPauseVillageItemCountMidAsmHook(PPCRegister& pThis, PPCRegister& count)
{
    count.u32 += 1;

    InjectMenuBehaviour(pThis.u32, count.u32);
}

bool CHudPauseMiscItemCountMidAsmHook(PPCRegister& count)
{
    if (count.u32 < 3)
        return true;

    return false;
}

bool CHudPauseMiscInjectOptionsMidAsmHook(PPCRegister& pThis)
{
    return InjectMenuBehaviour(pThis.u32, 3);
}

// SWA::CHudPause::Update
PPC_FUNC_IMPL(__imp__sub_824B0930);
PPC_FUNC(sub_824B0930)
{
    auto pHudPause = (SWA::CHudPause*)g_memory.Translate(ctx.r3.u32);
    auto pInputState = SWA::CInputState::GetInstance();

    g_achievementMenuIntroTime += g_deltaTime;

    if (g_isAchievementMenuOutro)
    {
        g_achievementMenuOutroTime += g_deltaTime;

        // Re-open pause menu after achievement menu closes with delay.
        if (g_achievementMenuOutroTime >= g_achievementMenuOutroThreshold)
        {
            GuestToHostFunction<int>(0x824AFD28, pHudPause, 0, 1, 0, 0);

            g_achievementMenuOutroTime = 0;
            g_isAchievementMenuOutro = false;
        }
    }

    // TODO: disable Start button closing menu.
    if (AchievementMenu::s_isVisible)
    {
        // HACK: wait for transition to finish before restoring control.
        if (g_achievementMenuIntroThreshold >= g_achievementMenuIntroTime)
            __imp__sub_824B0930(ctx, base);

        if (pInputState->GetPadState().IsTapped(SWA::eKeyState_B) && !g_isAchievementMenuOutro)
        {
            AchievementMenu::Close();

            g_isAchievementMenuOutro = true;
        }
    }
    else if (OptionsMenu::s_isVisible && OptionsMenu::s_isPause)
    {
        if (OptionsMenu::CanClose() && pInputState->GetPadState().IsTapped(SWA::eKeyState_B))
        {
            OptionsMenu::Close();

            GuestToHostFunction<int>(0x824AFD28, pHudPause, 0, 0, 0, 1);
        }
    }
    else
    {
        g_achievementMenuIntroTime = 0;

        __imp__sub_824B0930(ctx, base);
    }
}
