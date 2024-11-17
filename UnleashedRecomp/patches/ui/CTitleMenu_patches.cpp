#include <cpu/guest_code.h>
#include <api/SWA.h>
#include <ui/options_menu.h>

// SWA::CTitleStateMenu::Update
PPC_FUNC_IMPL(__imp__sub_825882B8);
PPC_FUNC(sub_825882B8)
{
    auto pTitleState = (SWA::CTitleStateBase*)g_memory.Translate(ctx.r3.u32);
    auto pInputState = SWA::CInputState::GetInstance();
    auto isOptionsIndex = pTitleState->m_pMember->m_pTitleMenu->m_CursorIndex == 2;

    if (pInputState && isOptionsIndex)
    {
        // TODO: play sys_worldmap_decide.
        if (pInputState->GetPadState().IsTapped(SWA::eKeyState_A))
            OptionsMenu::Open();
    }

    if (!OptionsMenu::s_isVisible)
        __imp__sub_825882B8(ctx, base);

    if (pInputState && isOptionsIndex)
    {
        // TODO: play sys_worldmap_cancel (could be "cansel" instead).
        if (pInputState->GetPadState().IsTapped(SWA::eKeyState_B))
            OptionsMenu::Close();
    }
}
