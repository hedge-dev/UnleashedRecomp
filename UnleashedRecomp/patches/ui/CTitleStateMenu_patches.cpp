#include <api/SWA.h>
#include <locale/locale.h>
#include <ui/button_guide.h>
#include <ui/fader.h>
#include <ui/message_window.h>
#include <ui/options_menu.h>
#include <user/paths.h>
#include <app.h>
#include <exports.h>

static bool g_installMessageOpen = false;
static bool g_installMessageFaderBegun = false;
static int g_installMessageResult = -1;

static bool ProcessInstallMessage()
{
    if (!g_installMessageOpen)
        return false;

    if (g_installMessageFaderBegun)
        return true;

    auto& str = App::s_isMissingDLC
        ? Localise("Installer_Message_TitleMissingDLC")
        : Localise("Installer_Message_Title");

    std::array<std::string, 2> options = { Localise("Common_Yes"), Localise("Common_No") };

    if (MessageWindow::Open(str, &g_installMessageResult, options, 1) == MSG_CLOSED)
    {
        switch (g_installMessageResult)
        {
            case 0:
                Fader::FadeOut(1, []() { App::Restart({ "--install-dlc" }); });
                g_installMessageFaderBegun = true;
                break;

            case 1:
                g_installMessageOpen = false;
                g_installMessageResult = -1;
                break;
        }
    }

    return true;
}

// SWA::CTitleStateMenu::Update
PPC_FUNC_IMPL(__imp__sub_825882B8);
PPC_FUNC(sub_825882B8)
{
    auto pTitleState = (SWA::CTitleStateBase*)g_memory.Translate(ctx.r3.u32);

    auto pInputState = SWA::CInputState::GetInstance();
    auto& pPadState = pInputState->GetPadState();
    auto isAccepted = pPadState.IsTapped(SWA::eKeyState_A) || pPadState.IsTapped(SWA::eKeyState_Start);

    auto pContext = pTitleState->GetContextBase<SWA::CTitleStateBase::CTitleStateContext>();
    auto isOptionsIndex = pContext->m_pTitleMenu->m_CursorIndex == 2;
    auto isInstallIndex = pContext->m_pTitleMenu->m_CursorIndex == 3;

    // Always default to New Game with corrupted save data.
    if (App::s_isSaveDataCorrupt && pContext->m_pTitleMenu->m_CursorIndex == 1)
        pContext->m_pTitleMenu->m_CursorIndex = 0;

    if (!OptionsMenu::s_isVisible && isOptionsIndex)
    {
        if (OptionsMenu::s_isRestartRequired)
        {
            static int result = -1;

            if (MessageWindow::Open(Localise("Options_Message_Restart"), &result) == MSG_CLOSED)
                Fader::FadeOut(1, []() { App::Restart(); });
        }
        else if (isAccepted)
        {
            Game_PlaySound("sys_worldmap_window");
            Game_PlaySound("sys_worldmap_decide");

            OptionsMenu::Open();
        }
    }
    else if (isInstallIndex && isAccepted)
    {
        g_installMessageOpen = true;
    }

    if (!OptionsMenu::s_isVisible && !OptionsMenu::s_isRestartRequired && !ProcessInstallMessage())
        __imp__sub_825882B8(ctx, base);

    if (isOptionsIndex)
    {
        if (OptionsMenu::CanClose() && pPadState.IsTapped(SWA::eKeyState_B))
        {
            Game_PlaySound("sys_worldmap_cansel");

            OptionsMenu::Close();
        }
    }
}

void TitleMenuRemoveContinueOnCorruptSaveMidAsmHook(PPCRegister& r3)
{
    if (!App::s_isSaveDataCorrupt)
        return;

    r3.u64 = 0;
}

void TitleMenuRemoveStorageDeviceOptionMidAsmHook(PPCRegister& r11)
{
    r11.u32 = 0;
}

void TitleMenuAddInstallOptionMidAsmHook(PPCRegister& r3)
{
    r3.u32 = 1;
}
