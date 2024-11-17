#include <cpu/guest_code.h>
#include <kernel/function.h>
#include <api/SWA.h>
#include <ui/options_menu.h>

bool m_isOptionsFromPause = false;

void CHudPauseAddOptionsItemMidAsmHook(PPCRegister& pThis)
{
    auto pStrMemory = __HH_ALLOC(8);

    auto menu = Hedgehog::Base::CSharedString("TopMenu");
    auto name = Hedgehog::Base::CSharedString("option");

    // TODO: replace with wrapper to put these into guest memory.
    memcpy(pStrMemory, &menu, 4);
    memcpy((void*)((size_t)pStrMemory + 4), &name, 4);

    GuestToHostFunction<int>(0x824AE690, pThis.u32, pStrMemory, (void*)((size_t)pStrMemory + 4));

    __HH_FREE(pStrMemory);
}

bool InjectOptionsBehaviour(uint32_t pThis, uint32_t count, uint32_t exitType = 2, uint32_t transitionType = 2)
{
    auto status = *(be<uint32_t>*)g_memory.Translate(pThis + 0x190);
    auto pauseType = *(be<uint32_t>*)g_memory.Translate(pThis + 0x18C);
    auto cursorIndex = *(be<uint32_t>*)g_memory.Translate(4 * (*(be<uint32_t>*)g_memory.Translate(pThis + 0x19C) + 0x68) + pThis);

    /*
        0 ---- Undefined
        1 ---- Status
        2 ---- Return to Previous Area
        3 ---- Inventory
        4 ---- Skills
        5 ---- Go to the Lab
        6 ---- Return to World Map
        7 ---- Undefined
        8 ---- Restart Stage
        9 ---- Continue Stage
        <=10 - Undefined
    */
    auto pExitType = (be<uint32_t>*)g_memory.Translate(pThis + 0x188);

    /*
        0 --- Undefined
        1 --- Unknown menu
        2 --- Quit menu
        3 --- Pause menu?
        4 --- Undefined
        5 --- Make cursor small?
        6 --- Hide UI and ignore face buttons
        7 --- Stop updating pause menu
        8 --- Hide UI (apart from pause header) and ignore face buttons
        <=9 - Stop updating pause menu
    */
    auto pTransitionType = (be<uint32_t>*)g_memory.Translate(pThis + 0x194);

    if (status == 1)
    {
        if (cursorIndex == count - 2)
        {
            OptionsMenu::Open(pauseType);
            m_isOptionsFromPause = true;

            *pExitType = 0;
            *pTransitionType = 6;

            return true;
        }
        else if (cursorIndex == count - 1)
        {
            *pExitType = exitType;
            *pTransitionType = transitionType;

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

bool CHudPauseHubItemCountMidAsmHook(PPCRegister& pThis, PPCRegister& count)
{
    count.u32 += 1;

    return InjectOptionsBehaviour(pThis.u32, count.u32, 2, 6);
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
    if (!OptionsMenu::s_isVisible || !m_isOptionsFromPause)
    {
        __imp__sub_824B0930(ctx, base);
        return;
    }

    auto pauseType = *(be<uint32_t>*)g_memory.Translate(ctx.r3.u32 + 0x18C);

    if (auto pInputState = SWA::CInputState::GetInstance())
    {
        // TODO: disable Start button closing menu.
        if (pInputState->GetPadState().IsTapped(SWA::eKeyState_B))
        {
            OptionsMenu::Close(pauseType);
            m_isOptionsFromPause = false;

            GuestToHostFunction<int>(0x824AFD28, ctx.r3.u32, 0, 0, 0, 1);
        }
    }
}
