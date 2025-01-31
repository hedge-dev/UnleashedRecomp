#include "CTitleStateIntro_patches.h"
#include <api/SWA.h>
#include <install/update_checker.h>
#include <locale/locale.h>
#include <ui/fader.h>
#include <ui/message_window.h>
#include <user/achievement_manager.h>
#include <user/paths.h>
#include <app.h>

static bool g_faderBegun = false;

bool g_quitMessageOpen = false;
static int g_quitMessageResult = -1;

static std::atomic<bool> g_corruptSaveMessageOpen = false;
static int g_corruptSaveMessageResult = -1;

static bool g_corruptAchievementsMessageOpen = false;
static int g_corruptAchievementsMessageResult = -1;

static bool g_updateAvailableMessageOpen = false;
static int g_updateAvailableMessageResult = -1;

static bool ProcessQuitMessage()
{
    if (g_corruptSaveMessageOpen)
        return false;

    if (!g_quitMessageOpen)
        return false;

    if (g_faderBegun)
        return true;

    std::array<std::string, 2> options = { Localise("Common_Yes"), Localise("Common_No") };

    if (MessageWindow::Open(Localise("Title_Message_Quit"), &g_quitMessageResult, options, 1) == MSG_CLOSED)
    {
        switch (g_quitMessageResult)
        {
            case 0:
                Fader::FadeOut(1, []() { App::Exit(); });
                g_faderBegun = true;
                break;

            case 1:
                g_quitMessageOpen = false;
                g_quitMessageResult = -1;
                break;
        }
    }

    return true;
}

static bool ProcessCorruptSaveMessage()
{
    if (!g_corruptSaveMessageOpen)
        return false;

    if (MessageWindow::Open(Localise("Title_Message_SaveDataCorrupt"), &g_corruptSaveMessageResult) == MSG_CLOSED)
    {
        g_corruptSaveMessageOpen = false;
        g_corruptSaveMessageOpen.notify_one();
        g_corruptSaveMessageResult = -1;
    }

    return true;
}

static bool ProcessCorruptAchievementsMessage()
{
    if (!g_corruptAchievementsMessageOpen)
        return false;

    auto message = AchievementManager::Status == EAchStatus::IOError
        ? Localise("Title_Message_AchievementDataIOError")
        : Localise("Title_Message_AchievementDataCorrupt");

    if (MessageWindow::Open(message, &g_corruptAchievementsMessageResult) == MSG_CLOSED)
    {
        // Allow user to proceed if the achievement data couldn't be loaded.
        // Restarting may fix this error, so it isn't worth clearing the data for.
        if (AchievementManager::Status != EAchStatus::IOError)
            AchievementManager::Save(true);

        g_corruptAchievementsMessageOpen = false;
        g_corruptAchievementsMessageResult = -1;
    }

    return true;
}

static bool ProcessUpdateAvailableMessage()
{
    if (!g_updateAvailableMessageOpen)
        return false;

    if (g_faderBegun)
        return true;

    std::array<std::string, 2> options = { Localise("Common_Yes"), Localise("Common_No") };

    if (MessageWindow::Open(Localise("Title_Message_UpdateAvailable"), &g_updateAvailableMessageResult, options) == MSG_CLOSED)
    {
        if (!g_updateAvailableMessageResult)
        {
            Fader::FadeOut(1,
            //
                []()
                {
                    UpdateChecker::visitWebsite();
                    App::Exit();
                }
            );

            g_faderBegun = true;
        }

        g_updateAvailableMessageOpen = false;
        g_updateAvailableMessageResult = -1;
    }

    return true;
}

void StorageDevicePromptMidAsmHook() {}

// Save data validation hook.
PPC_FUNC_IMPL(__imp__sub_822C55B0);
PPC_FUNC(sub_822C55B0)
{
    App::s_isSaveDataCorrupt = true;
    g_corruptSaveMessageOpen = true;
    g_corruptSaveMessageOpen.wait(true);
    ctx.r3.u32 = 0;
}

// SWA::CTitleStateIntro::Update
PPC_FUNC_IMPL(__imp__sub_82587E50);
PPC_FUNC(sub_82587E50)
{
    auto isAutoSaveWarningShown = *(bool*)g_memory.Translate(0x83367BC1);

    if (isAutoSaveWarningShown)
    {
        __imp__sub_82587E50(ctx, base);
    }
    else if (!ProcessUpdateAvailableMessage() && !ProcessCorruptSaveMessage() && !ProcessCorruptAchievementsMessage() && !g_faderBegun)
    {
        if (auto pInputState = SWA::CInputState::GetInstance())
        {
            auto& rPadState = pInputState->GetPadState();
            auto isAccepted = rPadState.IsTapped(SWA::eKeyState_A) || rPadState.IsTapped(SWA::eKeyState_Start);
            auto isDeclined = rPadState.IsTapped(SWA::eKeyState_B);

            if (isAccepted)
            {
                g_updateAvailableMessageOpen = UpdateChecker::check() == UpdateChecker::Result::UpdateAvailable;

                AchievementManager::Load();

                if (AchievementManager::Status != EAchStatus::Success)
                    g_corruptAchievementsMessageOpen = true;
            }

            if (isDeclined)
                g_quitMessageOpen = true;
        }

        if (!ProcessQuitMessage())
            __imp__sub_82587E50(ctx, base);
    }
}
