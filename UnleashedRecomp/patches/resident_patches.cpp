#include <user/achievement_data.h>
#include <user/config.h>
#include <api/SWA.h>
#include <os/logger.h>
#include <hid/hid.h>
#include <app.h>

bool m_isSavedAchievementData = false;

// SWA::Message::MsgRequestStartLoading::Impl
PPC_FUNC_IMPL(__imp__sub_824DCF38);
PPC_FUNC(sub_824DCF38)
{
    App::s_isLoading = true;

    // TODO: use the actual PS3 loading screen ("n_2_d").
    if (Config::TimeOfDayTransition == ETimeOfDayTransition::PlayStation)
    {
        if (ctx.r4.u32 == SWA::eLoadingDisplayType_WerehogMovie)
            ctx.r4.u32 = SWA::eLoadingDisplayType_Arrows;
    }

    if (auto pGameDocument = SWA::CGameDocument::GetInstance())
    {
        auto stageName = pGameDocument->m_pMember->m_StageName.c_str();

        if (stageName && strlen(stageName))
        {
            /* Fix restarting Eggmanland as the Werehog
               erroneously using the Event Gallery transition. */
            if (ctx.r4.u32 == SWA::eLoadingDisplayType_EventGallery && !strcmp(stageName, "Act_EggmanLand"))
                ctx.r4.u32 = SWA::eLoadingDisplayType_NowLoading;
        }
    }

    __imp__sub_824DCF38(ctx, base);
}

// SWA::CLoading::Update
PPC_FUNC_IMPL(__imp__sub_824DAB60);
PPC_FUNC(sub_824DAB60)
{
    auto pLoading = (SWA::CLoading*)g_memory.Translate(ctx.r3.u32);

    __imp__sub_824DAB60(ctx, base);

    if (!pLoading->m_LoadingDisplayType)
        App::s_isLoading = false;
}

// Load voice language files.
PPC_FUNC_IMPL(__imp__sub_824EB9B0);
PPC_FUNC(sub_824EB9B0)
{
    auto pApplicationDocument = (SWA::CApplicationDocument*)g_memory.Translate(ctx.r4.u32);

    pApplicationDocument->m_VoiceLanguage = (SWA::EVoiceLanguage)Config::VoiceLanguage.Value;

    __imp__sub_824EB9B0(ctx, base);
}

// SWA::CSaveIcon::Update
PPC_FUNC_IMPL(__imp__sub_824E5170);
PPC_FUNC(sub_824E5170)
{
    auto pSaveIcon = (SWA::CSaveIcon*)g_memory.Translate(ctx.r3.u32);

    __imp__sub_824E5170(ctx, base);

    if (pSaveIcon->m_IsVisible)
    {
        App::s_isSaveDataCorrupt = false;

        if (!m_isSavedAchievementData)
        {
            LOGN("Saving achievements...");

            AchievementData::Save();

            m_isSavedAchievementData = true;
        }
    }
    else
    {
        m_isSavedAchievementData = false;
    }
}

// SWA::CApplicationDocument::LoadArchiveDatabases
PPC_FUNC_IMPL(__imp__sub_824EFD28);
PPC_FUNC(sub_824EFD28)
{
    auto r3 = ctx.r3;

    // SWA::CSigninXenon::InitializeDLC
    ctx.r3.u64 = PPC_LOAD_U32(r3.u32 + 4) + 200;
    ctx.r4.u64 = 0;
    sub_822C57D8(ctx, base);

    ctx.r3 = r3;
    __imp__sub_824EFD28(ctx, base);
}

// SWA::CFileReaderXenon_DLC::InitializeParallel
PPC_FUNC(sub_822C3778)
{
    if (!PPC_LOAD_U8(0x83361F10)) // ms_DLCInitialized
    {
        // CFileReaderXenon_DLC::InitializeSerial
        sub_822C2CF0(ctx, base);
    }
}

// HasAnyDLC
// NOTE: This is not used outside title screen outro, so it's safe to hook.
PPC_FUNC(sub_82BD06C8)
{
    ctx.r3.u64 = 0;
}

void LoadingScreenControllerMidAsmHook()
{
    static constexpr size_t STR_ADDRESSES[] =
    {
        0x820301AC, // 360_sonic1
        0x820301B8, // 360_sonic2
        0x820301C4, // 360_sonic3
        0x820301D0, // 360_evil
        0x820301DC, // 360_robo
        0x820301E8  // 360_super
    };

    auto isPlayStation = Config::ControllerIcons == EControllerIcons::PlayStation;

    if (Config::ControllerIcons == EControllerIcons::Auto)
        isPlayStation = hid::g_inputDeviceController == hid::EInputDevice::PlayStation;

    const char* prefix = isPlayStation ? "ps3" : "360";

    for (auto address : STR_ADDRESSES)
        memcpy(g_memory.Translate(address), prefix, 3);
}
