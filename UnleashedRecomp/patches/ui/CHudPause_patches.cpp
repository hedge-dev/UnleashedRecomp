#include <cpu/guest_code.h>
#include <kernel/function.h>
#include <api/SWA.h>
#include <ui/options_menu.h>

void CHudPauseAddOptionsItemMidAsmHook(PPCRegister& pThis)
{
    guest_stack_var<Hedgehog::Base::CSharedString> menu("TopMenu");
    guest_stack_var<Hedgehog::Base::CSharedString> name("option");

    GuestToHostFunction<int>(0x824AE690, pThis.u32, menu.get(), name.get());
}

bool InjectOptionsBehaviour(uint32_t pThis, uint32_t count)
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

    return InjectOptionsBehaviour(pThis.u32, count.u32);
}

void CHudPauseVillageItemCountMidAsmHook(PPCRegister& pThis, PPCRegister& count)
{
    count.u32 += 1;

    InjectOptionsBehaviour(pThis.u32, count.u32);
}

bool CHudPauseMiscItemCountMidAsmHook(PPCRegister& count)
{
    if (count.u32 < 3)
        return true;

    return false;
}

bool CHudPauseMiscInjectOptionsMidAsmHook(PPCRegister& pThis)
{
    return InjectOptionsBehaviour(pThis.u32, 3);
}

// SWA::CHudPause::Update
PPC_FUNC_IMPL(__imp__sub_824B0930);
PPC_FUNC(sub_824B0930)
{
    if (!OptionsMenu::s_isVisible || !OptionsMenu::s_isPause)
    {
        __imp__sub_824B0930(ctx, base);
        return;
    }

    if (auto pInputState = SWA::CInputState::GetInstance())
    {
        // TODO: disable Start button closing menu.
        if (OptionsMenu::CanClose() && pInputState->GetPadState().IsTapped(SWA::eKeyState_B))
        {
            OptionsMenu::Close();

            // Re-open pause menu.
            GuestToHostFunction<int>(0x824AFD28, ctx.r3.u32, 0, 0, 0, 1);
        }
    }
}
