#include <cpu/guest_code.h>
#include <api/SWA.h>
#include <ui/options_menu.h>
#include <exports.h>

// SWA::CTitleStateMenu::Update
PPC_FUNC_IMPL(__imp__sub_825882B8);
PPC_FUNC(sub_825882B8)
{
    auto pTitleState = (SWA::CTitleStateBase*)g_memory.Translate(ctx.r3.u32);
    auto pInputState = SWA::CInputState::GetInstance();
    auto& pPadState = pInputState->GetPadState();
    auto isOptionsIndex = pTitleState->m_pMember->m_pTitleMenu->m_CursorIndex == 2;

    if (!OptionsMenu::s_isVisible && pInputState && isOptionsIndex)
    {
        if (pPadState.IsTapped(SWA::eKeyState_A) || pPadState.IsTapped(SWA::eKeyState_Start))
        {
            Game_PlaySound("sys_worldmap_window");
            Game_PlaySound("sys_worldmap_decide");

            OptionsMenu::Open();
        }
    }

    if (!OptionsMenu::s_isVisible)
        __imp__sub_825882B8(ctx, base);

    if (pInputState && isOptionsIndex)
    {
        if (OptionsMenu::CanClose() && pPadState.IsTapped(SWA::eKeyState_B))
        {
            Game_PlaySound("sys_worldmap_cansel");

            OptionsMenu::Close();
        }
    }
}

void TitleMenuRemoveStorageDeviceOptionMidAsmHook(PPCRegister& r11)
{
    r11.u32 = 0;
}

void TitleMenuAddInstallOptionMidAsmHook(PPCRegister& r3)
{
    r3.u32 = 1;
}