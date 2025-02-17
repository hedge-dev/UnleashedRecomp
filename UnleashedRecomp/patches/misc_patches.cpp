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

#include <gpu/video.h>
#include <gpu/imgui/imgui_snapshot.h>
#include <res/images/common/raw/kb_key_0.png.h>
#include <decompressor.h>
#include <ui/imgui_utils.h>
#include <exports.h>
#include <words.h>
#include <random>

static std::string g_qteText;
static std::vector<double> g_qteTimes;
static bool g_shouldDrawKeyboardQTE;
static std::unique_ptr<GuestTexture> g_keyboardTexture;
static uint8_t g_prevScanCodes[SDL_NUM_SCANCODES];
static std::default_random_engine g_engine{ std::random_device {}() };
static std::uniform_int_distribution g_intDistribution;
static std::uniform_real_distribution g_floatDistribution(0.0, 1.0);

void InitKeyboardQTE()
{
    g_keyboardTexture = LOAD_ZSTD_TEXTURE(g_kb_key_0);
}

void DrawKeyboardQTE()
{
    memcpy(g_prevScanCodes, SDL_GetKeyboardState(nullptr), sizeof(g_prevScanCodes));

    if (!g_shouldDrawKeyboardQTE)
        return;

    g_shouldDrawKeyboardQTE = false;

    auto drawList = ImGui::GetBackgroundDrawList();
    ImFont* font = ImFontAtlasSnapshot::GetFont("FOT-NewRodinPro-DB.otf");

    // y: 307
    // h: 110

    // font: x 36 y 30 30pt

    auto& res = ImGui::GetIO().DisplaySize;

    constexpr float padding = -40.0f;
    float width = Scale(g_qteText.size() * 110 + (g_qteText.size() - 1) * padding);
    for (size_t i = 0; i < g_qteText.size(); i++)
    {
        float x = res.x / 2.0f - width / 2.0f + Scale((110 + padding)) * i;
        double motion = ComputeLinearMotion(g_qteTimes[i], 0, 5);

        if (g_qteText[i] != ' ')
        {
            std::stringstream text;
            text << std::toupper(g_qteText[i], std::locale::classic());

            float x = res.x / 2.0f - width / 2.0f + Scale((110 + padding)) * i;
            drawList->AddImage(g_keyboardTexture.get(), { x, Scale(307) }, { x + Scale(110), Scale(307 + 110) }, {0.0f, 0.0f}, {1.0f, 1.0f}, IM_COL32(255, 255, 255, 255 * motion));
            drawList->AddText(font, Scale(24.0f), { x + Scale(36), Scale(307 + 28) }, IM_COL32(0, 0, 0, 255 * motion), text.str().c_str());
        }
        else
        {
            drawList->AddImage(g_keyboardTexture.get(), { x, Scale(307) }, { x + Scale(110), Scale(307 + 110) }, { 0.0f, 0.0f }, { 1.0f, 1.0f }, IM_COL32(255, 255, 255, 255 * (1.0 - motion)));
        }
    }
}

// + 100 is success bool
// return true to indicate either succeeded or failed
PPC_FUNC_IMPL(__imp__sub_823329F8);
PPC_FUNC(sub_823329F8)
{
    struct DelayCall
    {
        PPCRegister r3;
        uint8_t* base;
        PPCRegister time;

        DelayCall(PPCRegister& r3, uint8_t* base) : r3(r3), base(base)
        {
            time.u32 = PPC_LOAD_U32(r3.u32 + 104);

            PPCRegister newTime = time;
            newTime.f32 *= 5.0f;
            PPC_STORE_U32(r3.u32 + 104, newTime.u32);
        }

        ~DelayCall()
        {
            PPC_STORE_U32(r3.u32 + 104, time.u32);
        }
    } delayCall(ctx.r3, base);

    g_shouldDrawKeyboardQTE = true;

    bool foundAny = false;

    for (size_t i = 0; i < g_qteText.size(); i++)
    {
        if (g_qteText[i] != ' ')
        {
            int lower = std::tolower(g_qteText[i], std::locale::classic());
            SDL_Scancode scancode = SDL_GetScancodeFromKey(lower);

            for (size_t j = SDL_SCANCODE_A; j <= SDL_SCANCODE_Z; j++)
            {
                if (g_prevScanCodes[j] == 0 && SDL_GetKeyboardState(nullptr)[j] != 0)
                {
                    if (j == scancode) // pressed the right one
                    {
                        g_qteText[i] = ' ';
                        g_qteTimes[i] = ImGui::GetTime();

                        ctx.r3.u32 = 0;
                        Game_PlaySound("objsn_trickjump_button");

                        return;
                    }
                    else // wrong one!
                    {
                        PPC_STORE_U8(ctx.r3.u32 + 100, 0);
                        ctx.r3.u32 = 1;
                        return;
                    }
                }
            }

            foundAny = true;
            break;
        }
    }

    // ran out of time
    if (PPC_LOAD_U32(ctx.r3.u32 + 132) >= PPC_LOAD_U32(ctx.r3.u32 + 104))
    {
        PPC_STORE_U8(ctx.r3.u32 + 100, 0);
        ctx.r3.u32 = 1;
        return;
    }

    if (!foundAny)
    {
        // pressed all of them correctly
        PPC_STORE_U8(ctx.r3.u32 + 100, 1);
        ctx.r3.u32 = 1;
        return;
    }

    __imp__sub_823329F8(ctx, base);
    ctx.r3.u32 = 0;

    return;
}

PPC_FUNC_IMPL(__imp__sub_826117E0);
PPC_FUNC(sub_826117E0)
{
    auto counts = reinterpret_cast<uint32_t*>(base + PPC_LOAD_U32(ctx.r3.u32 + 252));
    auto times = reinterpret_cast<uint32_t*>(base + PPC_LOAD_U32(ctx.r3.u32 + 236));

    for (size_t i = 1; i < 3; i++)
    {
        if (counts[i] == 0)
        {
            counts[i] = counts[0];
            times[i] = times[0];
        }
    }

    __imp__sub_826117E0(ctx, base);
}

void QteButtonPromptInitMidAsmHook() 
{
    g_qteText = g_words[g_intDistribution(g_engine) % std::size(g_words)];
    g_qteTimes.clear();
    g_qteTimes.resize(g_qteText.size(), ImGui::GetTime());
}

void QteButtonPromptUpdateMidAsmHook()
{
}

void TrickJumperCompareMidAsmHook(PPCRegister& r28)
{
    if (r28.u32 == 12)
    {
        if (g_floatDistribution(g_engine) < 0.3)
            r28.u32 = 0;
    }
}
