#include "installer_wizard.h"

#include <nfd.h>

#include <apu/embedded_player.h>
#include <install/installer.h>
#include <gpu/video.h>
#include <gpu/imgui/imgui_snapshot.h>
#include <hid/hid.h>
#include <locale/locale.h>
#include <patches/aspect_ratio_patches.h>
#include <ui/imgui_utils.h>
#include <ui/button_guide.h>
#include <ui/message_window.h>
#include <ui/sdl_listener.h>
#include <ui/game_window.h>
#include <decompressor.h>
#include <exports.h>

#include <res/images/common/hedge-dev.dds.h>
#include <res/images/installer/install_001.dds.h>
#include <res/images/installer/install_002.dds.h>
#include <res/images/installer/install_003.dds.h>
#include <res/images/installer/install_004.dds.h>
#include <res/images/installer/install_005.dds.h>
#include <res/images/installer/install_006.dds.h>
#include <res/images/installer/install_007.dds.h>
#include <res/images/installer/install_008.dds.h>
#include <res/images/installer/miles_electric_icon.dds.h>
#include <res/images/installer/arrow_circle.dds.h>
#include <res/images/installer/pulse_install.dds.h>

// One Shot Animations Constants
static constexpr double SCANLINES_ANIMATION_TIME = 0.0;
static constexpr double SCANLINES_ANIMATION_DURATION = 15.0;

static constexpr double MILES_ICON_ANIMATION_TIME = SCANLINES_ANIMATION_TIME + 10.0;
static constexpr double MILES_ICON_ANIMATION_DURATION = 15.0;

static constexpr double IMAGE_ANIMATION_TIME = MILES_ICON_ANIMATION_TIME + MILES_ICON_ANIMATION_DURATION;
static constexpr double IMAGE_ANIMATION_DURATION = 15.0;

static constexpr double TITLE_ANIMATION_TIME = SCANLINES_ANIMATION_DURATION;
static constexpr double TITLE_ANIMATION_DURATION = 30.0;

static constexpr double CONTAINER_LINE_ANIMATION_TIME = SCANLINES_ANIMATION_DURATION;
static constexpr double CONTAINER_LINE_ANIMATION_DURATION = 23.0;

static constexpr double CONTAINER_OUTER_TIME = SCANLINES_ANIMATION_DURATION + CONTAINER_LINE_ANIMATION_DURATION;
static constexpr double CONTAINER_OUTER_DURATION = 23.0;

static constexpr double CONTAINER_INNER_TIME = SCANLINES_ANIMATION_DURATION + CONTAINER_LINE_ANIMATION_DURATION + 8.0;
static constexpr double CONTAINER_INNER_DURATION = 15.0;

static constexpr double ALL_ANIMATIONS_FULL_DURATION = CONTAINER_INNER_TIME + CONTAINER_INNER_DURATION;

static constexpr double INSTALL_ICONS_FADE_IN_ANIMATION_TIME = 0.0;
static constexpr double INSTALL_ICONS_FADE_IN_ANIMATION_DURATION = 15.0;

// Loop Animations Constants - their time range is [0.0, 1.0 + DELAY]
static constexpr double ARROW_CIRCLE_LOOP_SPEED = 1;

static constexpr double PULSE_ANIMATION_LOOP_SPEED = 1.5;
static constexpr double PULSE_ANIMATION_LOOP_DELAY = 0.5;
static constexpr double PULSE_ANIMATION_LOOP_FADE_HIGH_POINT = 0.5;

constexpr float IMAGE_X = 165.0f;
constexpr float IMAGE_Y = 106.0f;
constexpr float IMAGE_WIDTH = 512.0f;
constexpr float IMAGE_HEIGHT = 512.0f;

constexpr float CONTAINER_X = 510.0f;
constexpr float CONTAINER_Y = 225.0f;
constexpr float CONTAINER_WIDTH = 528.0f;
constexpr float CONTAINER_HEIGHT = 245.0f;
constexpr float SIDE_CONTAINER_WIDTH = CONTAINER_WIDTH / 2.0f;

constexpr float BOTTOM_X_GAP = 4.0f;
constexpr float BOTTOM_Y_GAP = 4.0f;
constexpr float CONTAINER_BUTTON_WIDTH = 250.0f;
constexpr float CONTAINER_BUTTON_GAP = 9.0f;
constexpr float BUTTON_HEIGHT = 22.0f;
constexpr float BUTTON_TEXT_GAP = 28.0f;

constexpr float BORDER_SIZE = 1.0f;
constexpr float BORDER_OVERSHOOT = 36.0f;
constexpr float FAKE_PROGRESS_RATIO = 0.25f;

static constexpr size_t GRID_SIZE = 9;

static ImFont *g_seuratFont;
static ImFont *g_dfsogeistdFont;
static ImFont *g_newRodinFont;

static double g_arrowCircleCurrentRotation = 0.0;

static double g_appearTime = 0.0;
static double g_disappearTime = DBL_MAX;
static bool g_isDisappearing = false;

static std::filesystem::path g_installPath;
static std::filesystem::path g_gameSourcePath;
static std::filesystem::path g_updateSourcePath;
static std::array<std::filesystem::path, int(DLC::Count)> g_dlcSourcePaths;
static std::array<bool, int(DLC::Count)> g_dlcInstalled = {};
static std::array<std::unique_ptr<GuestTexture>, 8> g_installTextures;
static std::unique_ptr<GuestTexture> g_milesElectricIcon;
static std::unique_ptr<GuestTexture> g_arrowCircle;
static std::unique_ptr<GuestTexture> g_pulseInstall;
static std::unique_ptr<GuestTexture> g_upHedgeDev;
static Journal g_installerJournal;
static Installer::Sources g_installerSources;
static uint64_t g_installerAvailableSize = 0;
static std::unique_ptr<std::thread> g_installerThread;
static double g_installerStartTime = 0.0;
static double g_installerEndTime = DBL_MAX;
static float g_installerProgressRatioCurrent = 0.0f;
static std::atomic<float> g_installerProgressRatioTarget = 0.0f;
static std::atomic<bool> g_installerFinished = false;
static bool g_installerFailed = false;
static std::string g_installerErrorMessage;

enum class WizardPage
{
    SelectLanguage,
    Introduction,
    SelectGameAndUpdate,
    SelectDLC,
    CheckSpace,
    Installing,
    InstallSucceeded,
    InstallFailed,
};

static WizardPage g_firstPage = WizardPage::SelectLanguage;
static WizardPage g_currentPage = g_firstPage;
static std::string g_currentMessagePrompt = "";
static bool g_currentMessagePromptConfirmation = false;
static std::list<std::filesystem::path> g_currentPickerResults;
static std::atomic<bool> g_currentPickerResultsReady = false;
static std::string g_currentPickerErrorMessage;
static std::unique_ptr<std::thread> g_currentPickerThread;
static bool g_pickerTutorialCleared[2] = {};
static bool g_pickerTutorialTriggered = false;
static bool g_pickerTutorialFolderMode = false;
static bool g_currentPickerVisible = false;
static bool g_currentPickerFolderMode = false;
static int g_currentMessageResult = -1;
static ImVec2 g_joypadAxis = {};
static int g_currentCursorIndex = -1;
static int g_currentCursorDefault = 0;
static bool g_currentCursorAccepted = false;
static std::vector<std::pair<ImVec2, ImVec2>> g_currentCursorRects;

class SDLEventListenerForInstaller : public SDLEventListener
{
public:
    void OnSDLEvent(SDL_Event *event) override
    {
        constexpr float AxisValueRange = 32767.0f;
        constexpr float AxisTapRange = 0.5f;
        if (!InstallerWizard::s_isVisible || !g_currentMessagePrompt.empty() || g_currentPickerVisible)
        {
            return;
        }

        int newCursorIndex = -1;
        ImVec2 tapDirection = {};
        switch (event->type)
        {
        case SDL_KEYDOWN:
            switch (event->key.keysym.scancode)
            {
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_RIGHT:
                tapDirection.x = (event->key.keysym.scancode == SDL_SCANCODE_RIGHT) ? 1.0f : -1.0f;
                break;
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_DOWN:
                tapDirection.y = (event->key.keysym.scancode == SDL_SCANCODE_DOWN) ? 1.0f : -1.0f;
                break;
            case SDL_SCANCODE_RETURN:
            case SDL_SCANCODE_KP_ENTER:
                g_currentCursorAccepted = (g_currentCursorIndex >= 0);
                break;
            }

            break;
        case SDL_CONTROLLERBUTTONDOWN:
            switch (event->cbutton.button)
            {
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                tapDirection = { -1.0f, 0.0f };
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                tapDirection = { 1.0f, 0.0f };
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                tapDirection = { 0.0f, -1.0f };
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                tapDirection = { 0.0f, 1.0f };
                break;
            case SDL_CONTROLLER_BUTTON_A:
                g_currentCursorAccepted = (g_currentCursorIndex >= 0);
                break;
            }

            break;
        case SDL_CONTROLLERAXISMOTION:
        {
            if (event->caxis.axis < 2)
            {
                float newAxisValue = event->caxis.value / AxisValueRange;
                bool sameDirection = (newAxisValue * g_joypadAxis[event->caxis.axis]) > 0.0f;
                bool wasInRange = abs(g_joypadAxis[event->caxis.axis]) > AxisTapRange;
                bool isInRange = abs(newAxisValue) > AxisTapRange;
                if (sameDirection && !wasInRange && isInRange)
                {
                    tapDirection[event->caxis.axis] = newAxisValue;
                }

                g_joypadAxis[event->caxis.axis] = newAxisValue;
            }

            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        {
            for (size_t i = 0; i < g_currentCursorRects.size(); i++)
            {
                auto &currentRect = g_currentCursorRects[i];
                if (ImGui::IsMouseHoveringRect(currentRect.first, currentRect.second, false))
                {
                    newCursorIndex = int(i);

                    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT)
                    {
                        g_currentCursorAccepted = true;
                    }

                    break;
                }
            }

            if (newCursorIndex < 0)
            {
                g_currentCursorIndex = -1;
            }

            break;
        }
        }

        if (tapDirection.x != 0.0f || tapDirection.y != 0.0f)
        {
            if (g_currentCursorIndex >= g_currentCursorRects.size() || g_currentCursorIndex < 0)
            {
                newCursorIndex = g_currentCursorDefault;
            }
            else
            {
                auto &currentRect = g_currentCursorRects[g_currentCursorIndex];
                ImVec2 currentPoint = ImVec2
                (
                    (currentRect.first.x + currentRect.second.x) / 2.0f + tapDirection.x * (currentRect.second.x - currentRect.first.x) / 2.0f,
                    (currentRect.first.y + currentRect.second.y) / 2.0f + tapDirection.y * (currentRect.second.y - currentRect.first.y) / 2.0f
                );

                float closestDistance = FLT_MAX;
                for (size_t i = 0; i < g_currentCursorRects.size(); i++)
                {
                    if (g_currentCursorIndex == i)
                    {
                        continue;
                    }

                    auto &targetRect = g_currentCursorRects[i];
                    ImVec2 targetPoint = ImVec2
                    (
                        (targetRect.first.x + targetRect.second.x) / 2.0f + tapDirection.x * (targetRect.first.x - targetRect.second.x) / 2.0f,
                        (targetRect.first.y + targetRect.second.y) / 2.0f + tapDirection.y * (targetRect.first.y - targetRect.second.y) / 2.0f
                    );

                    ImVec2 delta = ImVec2(targetPoint.x - currentPoint.x, targetPoint.y - currentPoint.y);
                    float projectedDistance = delta.x * tapDirection.x + delta.y * tapDirection.y;
                    float manhattanDistance = abs(delta.x) + abs(delta.y);
                    if (projectedDistance > 0.0f && manhattanDistance < closestDistance)
                    {
                        newCursorIndex = int(i);
                        closestDistance = manhattanDistance;
                    }
                }
            }
        }

        if (newCursorIndex >= 0)
        {
            if (g_currentCursorIndex != newCursorIndex)
            {
                Game_PlaySound("sys_worldmap_cursor");
            }

            g_currentCursorIndex = newCursorIndex;
        }
    }
};

static SDLEventListenerForInstaller g_eventListener;

const char CREDITS_TEXT[] = "Skyth, Hyper, Darío, Sajid, RadiantDerg, PTKay, DeaThProj, NextinHKRY, M&M, LadyLunanova";

static std::string& GetWizardText(WizardPage page)
{
    switch (page)
    {
        case WizardPage::SelectLanguage: return Localise("Installer_Page_SelectLanguage");
        case WizardPage::Introduction: return Localise("Installer_Page_Introduction");
        case WizardPage::SelectGameAndUpdate: return Localise("Installer_Page_SelectGameAndUpdate");
        case WizardPage::SelectDLC: return Localise("Installer_Page_SelectDLC");
        case WizardPage::CheckSpace: return Localise("Installer_Page_CheckSpace");
        case WizardPage::Installing: return Localise("Installer_Page_Installing");
        case WizardPage::InstallSucceeded: return Localise("Installer_Page_InstallSucceeded");
        case WizardPage::InstallFailed: return Localise("Installer_Page_InstallFailed");
    }

    return g_localeMissing;
}

static const int WIZARD_INSTALL_TEXTURE_INDEX[] =
{
    0,
    0,
    1,
    2,
    3,
    4,
    7, // Force Werehog on InstallSucceeded.
    5  // Force Eggman on InstallFailed.
};

// These are ordered from bottom to top in a 3x2 grid.
const char *LANGUAGE_TEXT[] =
{
    "FRANÇAIS", // French
    "DEUTSCH", // German
    "ENGLISH", // English
    "ESPAÑOL", // Spanish
    "ITALIANO", // Italian
    "日本語", // Japanese
};

const ELanguage LANGUAGE_ENUM[] =
{
    ELanguage::French,
    ELanguage::German,
    ELanguage::English,
    ELanguage::Spanish,
    ELanguage::Italian,
    ELanguage::Japanese,
};

const char *DLC_SOURCE_TEXT[] =
{
    "SPAGONIA",
    "CHUN-NAN",
    "MAZURI",
    "HOLOSKA",
    "APOTOS & SHAMAR",
    "EMPIRE CITY & ADABAT",
};

static int DLCIndex(DLC dlc)
{
    assert(dlc != DLC::Unknown);
    return (int)(dlc) - 1;
}

static double ComputeMotionInstaller(double timeAppear, double timeDisappear, double offset, double total)
{
    return ComputeMotion(timeAppear, offset, total) * (1.0 - ComputeMotion(timeDisappear, ALL_ANIMATIONS_FULL_DURATION - offset - total, total));
}

static double ComputeMotionInstallerLoop(double timeAppear, double speed, double offset) 
{
    return std::clamp(fmodf((ImGui::GetTime() - timeAppear) * speed, 1.0f + offset) - offset, 0.0, 1.0) / 1.0;
}

static double ComputeHermiteMotionInstallerLoop(double timeAppear, double speed, double offset) 
{
    return (cosf(M_PI * ComputeMotionInstallerLoop(timeAppear, speed, offset) + M_PI) + 1) / 2;
}

static bool PushCursorRect(ImVec2 min, ImVec2 max, bool &cursorPressed, bool makeDefault = false)
{
    int currentIndex = int(g_currentCursorRects.size());
    g_currentCursorRects.emplace_back(min, max);

    if (makeDefault)
    {
        g_currentCursorDefault = currentIndex;
    }

    if (g_currentCursorIndex == currentIndex)
    {
        if (g_currentCursorAccepted)
        {
            Game_PlaySound("sys_worldmap_finaldecide");
            cursorPressed = true;
            g_currentCursorAccepted = false;
        }

        return true;
    }
    else
    {
        return false;
    }
}

static void ResetCursorRects()
{
    g_currentCursorDefault = 0;
    g_currentCursorRects.clear();
}

static void DrawBackground()
{
    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    drawList->AddRectFilled({ 0.0, 0.0 }, res, IM_COL32_BLACK);
}

static void DrawLeftImage()
{
    int installTextureIndex = WIZARD_INSTALL_TEXTURE_INDEX[int(g_currentPage)];
    if (g_currentPage == WizardPage::Installing)
    {
        // Cycle through the available images while time passes during installation.
        constexpr double InstallationSpeed = 1.0 / 15.0;
        double installationTime = (ImGui::GetTime() - g_installerStartTime) * InstallationSpeed;
        installTextureIndex += int(installationTime);
    }

    double imageAlpha = ComputeMotionInstaller(g_appearTime, g_disappearTime, IMAGE_ANIMATION_TIME, IMAGE_ANIMATION_DURATION);
    int a = std::lround(255.0 * imageAlpha);
    GuestTexture *guestTexture = g_installTextures[installTextureIndex % g_installTextures.size()].get();
    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    ImVec2 min = { g_aspectRatioOffsetX + Scale(IMAGE_X), g_aspectRatioOffsetY + Scale(IMAGE_Y) };
    ImVec2 max = { min.x + Scale(IMAGE_WIDTH), min.y + Scale(IMAGE_HEIGHT) };
    drawList->AddImage(guestTexture, min, max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, a));
}

static void DrawHeaderIconsForInstallPhase(double iconsPosX, double iconsPosY, double iconsScale)
{
    auto drawList = ImGui::GetForegroundDrawList();

    // Arrow Circle Icon
    ImVec2 arrowCircleMin = { g_aspectRatioOffsetX + Scale(iconsPosX - iconsScale / 2), Scale(iconsPosY - iconsScale / 2) };
    ImVec2 arrowCircleMax = { g_aspectRatioOffsetX + Scale(iconsPosX + iconsScale / 2), Scale(iconsPosY + iconsScale / 2) };
    ImVec2 center = { g_aspectRatioOffsetX + Scale(iconsPosX) + 0.5f, Scale(iconsPosY) - 0.5f };

    float arrowCircleFadeMotion = ComputeMotionInstaller(g_installerStartTime, g_installerEndTime, INSTALL_ICONS_FADE_IN_ANIMATION_TIME, INSTALL_ICONS_FADE_IN_ANIMATION_DURATION);
    float rotationMotion = ComputeMotionInstallerLoop(g_installerStartTime, ARROW_CIRCLE_LOOP_SPEED, 0);
    float rotation = -2 * M_PI * rotationMotion;

    // Calculate rotated corners
    float cosCurrentAngle = cosf(rotation);
    float sinCurrentAngle = sinf(rotation);
    ImVec2 corners[4] = 
    {
        ImRotate(ImVec2(arrowCircleMin.x - center.x, arrowCircleMin.y - center.y), cosCurrentAngle, sinCurrentAngle),
        ImRotate(ImVec2(arrowCircleMax.x - center.x, arrowCircleMin.y - center.y), cosCurrentAngle, sinCurrentAngle),
        ImRotate(ImVec2(arrowCircleMax.x - center.x, arrowCircleMax.y - center.y), cosCurrentAngle, sinCurrentAngle),
        ImRotate(ImVec2(arrowCircleMin.x - center.x, arrowCircleMax.y - center.y), cosCurrentAngle, sinCurrentAngle),
    };

    for (int i = 0; i < IM_ARRAYSIZE(corners); ++i)
    {
        corners[i].x += center.x;
        corners[i].y += center.y;
    }

    drawList->AddImageQuad(g_arrowCircle.get(), corners[0], corners[1], corners[2], corners[3], ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32(255, 255, 255, 96 * arrowCircleFadeMotion));


    // Pulse
    float pulseFadeMotion = ComputeMotionInstaller(g_installerStartTime, g_installerEndTime, INSTALL_ICONS_FADE_IN_ANIMATION_TIME, INSTALL_ICONS_FADE_IN_ANIMATION_DURATION);
    float pulseMotion = ComputeMotionInstallerLoop(g_installerStartTime, PULSE_ANIMATION_LOOP_SPEED, PULSE_ANIMATION_LOOP_DELAY);
    float pulseHermiteMotion = ComputeHermiteMotionInstallerLoop(g_installerStartTime, PULSE_ANIMATION_LOOP_SPEED, PULSE_ANIMATION_LOOP_DELAY);

    float pulseFade = pulseMotion / PULSE_ANIMATION_LOOP_FADE_HIGH_POINT;

    if (pulseMotion >= PULSE_ANIMATION_LOOP_FADE_HIGH_POINT) {
        // Calculate linear fade-out from high point time - ({PULSE_ANIMATION_LOOP_FADE_HIGH_POINT}, 1) - to loop end - (1, 0) -.
        float m = -1 / (1 - PULSE_ANIMATION_LOOP_FADE_HIGH_POINT);
        float b = m * (-PULSE_ANIMATION_LOOP_FADE_HIGH_POINT) + 1;
        
        pulseFade = m * pulseMotion + b;
    }

    float pulseScale = iconsScale * pulseHermiteMotion * 1.5;

    ImVec2 pulseMin = { g_aspectRatioOffsetX + Scale(iconsPosX - pulseScale / 2), Scale(iconsPosY - pulseScale / 2) };
    ImVec2 pulseMax = { g_aspectRatioOffsetX + Scale(iconsPosX + pulseScale / 2), Scale(iconsPosY + pulseScale / 2) };
    drawList->AddImage(g_pulseInstall.get(), pulseMin, pulseMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255 * pulseFade * pulseFadeMotion));
}

static void DrawHeaderIcons()
{
    auto drawList = ImGui::GetForegroundDrawList();

    float iconsPosX = 253.0f;
    float iconsPosY = 79.0f;
    float iconsScale = 58;

    // Miles Electric Icon
    float milesIconMotion = ComputeMotionInstaller(g_appearTime, g_disappearTime, MILES_ICON_ANIMATION_TIME, MILES_ICON_ANIMATION_DURATION);
    float milesIconScale = iconsScale * (2 - milesIconMotion);

    ImVec2 milesElectricMin = { g_aspectRatioOffsetX + Scale(iconsPosX - milesIconScale / 2), Scale(iconsPosY - milesIconScale / 2) };
    ImVec2 milesElectricMax = { g_aspectRatioOffsetX + Scale(iconsPosX + milesIconScale / 2), Scale(iconsPosY + milesIconScale / 2) };
    drawList->AddImage(g_milesElectricIcon.get(), milesElectricMin, milesElectricMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255 * milesIconMotion));

    if (int(g_currentPage) >= int(WizardPage::Installing))
    {
        DrawHeaderIconsForInstallPhase(iconsPosX, iconsPosY, iconsScale);
    }
}

static void DrawScanlineBars()
{
    double scanlinesAlpha = ComputeMotionInstaller(g_appearTime, g_disappearTime, 0.0, SCANLINES_ANIMATION_DURATION);

    const uint32_t COLOR0 = IM_COL32(203, 255, 0, 0);
    const uint32_t COLOR1 = IM_COL32(203, 255, 0, 55 * scanlinesAlpha);
    const uint32_t FADE_COLOR0 = IM_COL32(0, 0, 0, 255 * scanlinesAlpha);
    const uint32_t FADE_COLOR1 = IM_COL32(0, 0, 0, 0);
    const uint32_t OUTLINE_COLOR = IM_COL32(115, 178, 104, 255 * scanlinesAlpha);

    float height = Scale(105.0f) * ComputeMotionInstaller(g_appearTime, g_disappearTime, 0.0, SCANLINES_ANIMATION_DURATION);
    if (height < 1e-6f)
    {
        return;
    }

    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE);

    // Top bar
    drawList->AddRectFilledMultiColor
    (
        { 0.0f, 0.0f },
        { res.x, height },
        COLOR0,
        COLOR0,
        COLOR1,
        COLOR1
    );

    // Bottom bar
    drawList->AddRectFilledMultiColor
    (
        { res.x, res.y },
        { 0.0f, res.y - height },
        COLOR0,
        COLOR0,
        COLOR1,
        COLOR1
    );

    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    // Installer text
    const std::string &headerText = Localise(g_currentPage == WizardPage::Installing ? "Installer_Header_Installing" : "Installer_Header_Installer");
    auto alphaMotion = ComputeMotionInstaller(g_appearTime, g_disappearTime, TITLE_ANIMATION_TIME, TITLE_ANIMATION_DURATION);
    DrawTextWithOutline(g_dfsogeistdFont, Scale(42.0f), { g_aspectRatioOffsetX + Scale(285.0f), Scale(57.0f) }, IM_COL32(255, 195, 0, 255 * alphaMotion), headerText.c_str(), 4, IM_COL32(0, 0, 0, 255 * alphaMotion), IMGUI_SHADER_MODIFIER_TITLE_BEVEL);

    // Top bar line
    drawList->AddLine
    (
        { 0.0f, height },
        { res.x, height },
        OUTLINE_COLOR,
        Scale(1)
    );

    // Bottom bar line
    drawList->AddLine
    (
        { 0.0f, res.y - height },
        { res.x, res.y - height },
        OUTLINE_COLOR,
        Scale(1)
    );

    DrawHeaderIcons();
    DrawVersionString(g_newRodinFont, IM_COL32(255, 255, 255, 70 * alphaMotion));
}

static void DrawContainer(ImVec2 min, ImVec2 max, bool isTextArea)
{   
    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();

    double gridAlpha = ComputeMotionInstaller(g_appearTime, g_disappearTime, 
        isTextArea ? CONTAINER_INNER_TIME : CONTAINER_OUTER_TIME,
        isTextArea ? CONTAINER_INNER_DURATION : CONTAINER_OUTER_DURATION
    );
    double gridOverlayAlpha = ComputeMotionInstaller(g_appearTime, g_disappearTime, CONTAINER_INNER_TIME, CONTAINER_INNER_DURATION);

    const uint32_t gridColor = IM_COL32(0, 33, 0, (isTextArea ? 223 : 255) * gridAlpha);
    const uint32_t gridOverlayColor = IM_COL32(0, 32, 0, 128 * gridOverlayAlpha);

    float gridSize = Scale(GRID_SIZE);

    SetShaderModifier(IMGUI_SHADER_MODIFIER_CHECKERBOARD);
    SetAdditive(true);
    drawList->AddRectFilled(min, max, gridColor);
    SetAdditive(false);
    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);

    if (isTextArea) 
    {
        drawList->AddRectFilled(min, max, gridOverlayColor);
    }

    // The draw area
    drawList->PushClipRect({ min.x + gridSize * 2.0f, min.y + gridSize * 2.0f }, { max.x - gridSize * 2.0f + 1.0f, max.y - gridSize * 2.0f + 1.0f });
}

static void DrawDescriptionContainer()
{
    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    auto fontSize = Scale(26.0f);

    ImVec2 descriptionMin = { g_aspectRatioOffsetX + Scale(CONTAINER_X), g_aspectRatioOffsetY + Scale(CONTAINER_Y) };
    ImVec2 descriptionMax = { g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH), g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT) };
    SetProceduralOrigin(descriptionMin);
    DrawContainer(descriptionMin, descriptionMax, true);

    char descriptionText[512];
    char requiredSpaceText[128];
    char availableSpaceText[128];
    strncpy(descriptionText, GetWizardText(g_currentPage).c_str(), sizeof(descriptionText) - 1);

    if (g_currentPage == WizardPage::CheckSpace)
    {
        constexpr double DivisorGiB = (1024.0 * 1024.0 * 1024.0);
        double requiredGiB = double(g_installerSources.totalSize) / DivisorGiB;
        double availableGiB = double(g_installerAvailableSize) / DivisorGiB;
        snprintf(requiredSpaceText, sizeof(requiredSpaceText), Localise("Installer_Step_RequiredSpace").c_str(), requiredGiB);
        snprintf(availableSpaceText, sizeof(availableSpaceText), (g_installerAvailableSize > 0) ? Localise("Installer_Step_AvailableSpace").c_str() : "", availableGiB);
        snprintf(descriptionText, sizeof(descriptionText), "%s%s\n%s", GetWizardText(g_currentPage).c_str(), requiredSpaceText, availableSpaceText);
    }
    else if (g_currentPage == WizardPage::InstallFailed)
    {
        strncat(descriptionText, g_installerErrorMessage.c_str(), sizeof(descriptionText) - 1);
    }

    double textAlpha = ComputeMotionInstaller(g_appearTime, g_disappearTime, CONTAINER_INNER_TIME, CONTAINER_INNER_DURATION);
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();

    drawList->AddText
    (
        g_seuratFont,
        fontSize,
        { clipRectMin.x, clipRectMin.y },
        IM_COL32(255, 255, 255, 255 * textAlpha),
        descriptionText,
        0,
        clipRectMax.x - clipRectMin.x
    );

    drawList->PopClipRect();

    if (g_currentPage == WizardPage::InstallSucceeded)
    {
        auto hedgeDevStr = "hedge-dev";
        auto hedgeDevTextSize = g_seuratFont->CalcTextSizeA(fontSize, FLT_MAX, 0, hedgeDevStr);
        auto hedgeDevTextMarginX = Scale(15);

        auto imageScale = hedgeDevTextSize.x / 3;
        auto imageMarginY = Scale(15);

        auto colWhite = IM_COL32(255, 255, 255, 255 * textAlpha);

        ImVec2 imageMin = 
        {
            /* X */ g_aspectRatioOffsetX + Scale(CONTAINER_X) + (Scale(CONTAINER_WIDTH) / 2) - (imageScale / 2) - (hedgeDevTextSize.x / 2) - hedgeDevTextMarginX,
            /* Y */ g_aspectRatioOffsetY + Scale(CONTAINER_Y) + (Scale(CONTAINER_HEIGHT) / 2) - (imageScale / 2) + imageMarginY
        };

        ImVec2 imageMax = { imageMin.x + imageScale, imageMin.y + imageScale };

        drawList->AddImage(g_upHedgeDev.get(), imageMin, imageMax, { 0, 0 }, { 1, 1 }, colWhite);

        drawList->AddText
        (
            g_seuratFont,
            fontSize,
            { /* X */ imageMax.x + hedgeDevTextMarginX, /* Y */ imageMin.y + (imageScale / 2) - (hedgeDevTextSize.y / 2) },
            colWhite,
            hedgeDevStr
        );

        auto marqueeTextSize = g_seuratFont->CalcTextSizeA(fontSize, FLT_MAX, 0, CREDITS_TEXT);
        auto marqueeTextMarginX = Scale(5);
        auto marqueeTextMarginY = Scale(15);

        ImVec2 textPos = { descriptionMax.x, g_aspectRatioOffsetY + Scale(CONTAINER_Y) + Scale(CONTAINER_HEIGHT) - marqueeTextSize.y - marqueeTextMarginY };
        ImVec2 textMin = { g_aspectRatioOffsetX + Scale(CONTAINER_X), textPos.y };
        ImVec2 textMax = { g_aspectRatioOffsetX + Scale(CONTAINER_X) + Scale(CONTAINER_WIDTH), g_aspectRatioOffsetY + Scale(CONTAINER_Y) + Scale(CONTAINER_HEIGHT) };

        SetMarqueeFade(textMin, textMax, Scale(32));
        DrawTextWithMarquee(g_seuratFont, fontSize, textPos, textMin, textMax, colWhite, CREDITS_TEXT, g_appearTime, 0.9, Scale(250));
        ResetMarqueeFade();
    }

    ImVec2 sideMin = { descriptionMax.x, descriptionMin.y };
    ImVec2 sideMax = { res.x, descriptionMax.y };
    DrawContainer(sideMin, sideMax, false);
    drawList->PopClipRect();

    if (g_currentPage != WizardPage::Installing && textAlpha >= 1.0)
    {
        auto icon = hid::IsInputDeviceController()
            ? EButtonIcon::A
            : hid::g_inputDevice == hid::EInputDevice::Keyboard
                ? EButtonIcon::Enter
                : EButtonIcon::LMB;

        ButtonGuide::Open(Button(Localise("Common_Select"), icon));
    }
    else
    {
        ButtonGuide::Close();
    }

    ResetProceduralOrigin();
}

static void DrawButtonContainer(ImVec2 min, ImVec2 max, int baser, int baseg, float alpha)
{
    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    SetShaderModifier(IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON);
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(baser, baseg + 130, 0, 223 * alpha), IM_COL32(baser, baseg + 130, 0, 178 * alpha), IM_COL32(baser, baseg + 130, 0, 223 * alpha), IM_COL32(baser, baseg + 130, 0, 178 * alpha));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(baser, baseg, 0, 13 * alpha), IM_COL32(baser, baseg, 0, 0), IM_COL32(baser, baseg, 0, 55 * alpha), IM_COL32(baser, baseg, 0, 6 * alpha));
    drawList->AddRectFilledMultiColor(min, max, IM_COL32(baser, baseg + 130, 0, 13 * alpha), IM_COL32(baser, baseg + 130, 0, 111 * alpha), IM_COL32(baser, baseg + 130, 0, 0), IM_COL32(baser, baseg + 130, 0, 55 * alpha));
    SetShaderModifier(IMGUI_SHADER_MODIFIER_NONE);
}

static ImVec2 ComputeTextSize(ImFont *font, const char *text, float size, float &squashRatio, float maxTextWidth = FLT_MAX)
{
    ImVec2 textSize = font->CalcTextSizeA(size, FLT_MAX, 0.0f, text);
    if (textSize.x > maxTextWidth)
    {
        squashRatio = maxTextWidth / textSize.x;
    }
    else
    {
        squashRatio = 1.0f;
    }

    return textSize;
}

static void DrawButton(ImVec2 min, ImVec2 max, const char *buttonText, bool sourceButton, bool buttonEnabled, bool &buttonPressed, float maxTextWidth = FLT_MAX, bool makeDefault = false)
{
    buttonPressed = false;

    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    float alpha = ComputeMotionInstaller(g_appearTime, g_disappearTime, CONTAINER_INNER_TIME, CONTAINER_INNER_DURATION);
    if (!buttonEnabled)
    {
        alpha *= 0.5f;
    }

    int baser = 0;
    int baseg = 0;
    if (g_currentMessagePrompt.empty() && !g_currentPickerVisible && !sourceButton && buttonEnabled && (alpha >= 1.0f))
    {
        bool cursorOnButton = PushCursorRect(min, max, buttonPressed, makeDefault);
        if (cursorOnButton)
        {
            baser = 48;
            baseg = 32;
        }
    }

    DrawButtonContainer(min, max, baser, baseg, alpha);

    ImFont *font = sourceButton ? g_newRodinFont : g_dfsogeistdFont;
    float size = Scale(sourceButton ? 15.0f : 20.0f);
    float squashRatio;
    ImVec2 textSize = ComputeTextSize(font, buttonText, size, squashRatio, Scale(maxTextWidth));
    min.x += ((max.x - min.x) - textSize.x) / 2.0f;
    min.y += ((max.y - min.y) - textSize.y) / 2.0f;

    if (!sourceButton)
    {
        // Fixes slight misalignment caused by this particular font.
        min.y -= Scale(1.0f);
    }

    SetOrigin({ min.x + textSize.x / 2.0f, min.y });
    SetScale({ squashRatio, 1.0f });
    SetGradient
    (
        min,
        { min.x + textSize.x, min.y + textSize.y },
        IM_COL32(baser + 192, 255, 0, 255),
        IM_COL32(baser + 128, baseg + 170, 0, 255)
    );

    DrawTextWithOutline
    (
        font,
        size,
        min,
        IM_COL32(255, 255, 255, 255 * alpha),
        buttonText,
        4,
        IM_COL32(baser, baseg, 0, 255 * alpha)
    );

    ResetGradient();
    SetScale({ 1.0f, 1.0f });
    SetOrigin({ 0.0f, 0.0f });
}

enum ButtonColumn
{
    ButtonColumnLeft,
    ButtonColumnMiddle,
    ButtonColumnRight
};

static void ComputeButtonColumnCoordinates(ButtonColumn buttonColumn, float &minX, float &maxX)
{
    switch (buttonColumn)
    {
    case ButtonColumnLeft:
        minX = g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_BUTTON_GAP);
        maxX = g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_BUTTON_GAP + CONTAINER_BUTTON_WIDTH);
        break;
    case ButtonColumnMiddle:
        minX = g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH / 2.0f - CONTAINER_BUTTON_WIDTH / 2.0f);
        maxX = g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH / 2.0f + CONTAINER_BUTTON_WIDTH / 2.0f);
        break;
    case ButtonColumnRight:
        minX = g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH - CONTAINER_BUTTON_GAP - CONTAINER_BUTTON_WIDTH);
        maxX = g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH - CONTAINER_BUTTON_GAP);
        break;
    }
}

static void DrawSourceButton(ButtonColumn buttonColumn, float yRatio, const char *sourceText, bool sourceSet)
{
    bool buttonPressed;
    float minX, maxX;
    ComputeButtonColumnCoordinates(buttonColumn, minX, maxX);

    float minusY = (CONTAINER_BUTTON_GAP + BUTTON_HEIGHT) * yRatio;
    ImVec2 min = { minX, g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT - CONTAINER_BUTTON_GAP - BUTTON_HEIGHT - minusY) };
    ImVec2 max = { maxX, g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT - CONTAINER_BUTTON_GAP - minusY) };
    DrawButton(min, max, sourceText, true, sourceSet, buttonPressed);
}

static void DrawProgressBar(float progressRatio)
{
    auto &res = ImGui::GetIO().DisplaySize;
    auto drawList = ImGui::GetForegroundDrawList();
    float alpha = 1.0;
    const uint32_t innerColor0 = IM_COL32(0, 65, 0, 255 * alpha);
    const uint32_t innerColor1 = IM_COL32(0, 32, 0, 255 * alpha);
    float xPadding = Scale(6.0f);
    float yPadding = Scale(3.0f);
    ImVec2 min = { g_aspectRatioOffsetX + Scale(CONTAINER_X) + BOTTOM_X_GAP, g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT + BOTTOM_Y_GAP) };
    ImVec2 max = { g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH - BOTTOM_X_GAP), g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT + BOTTOM_Y_GAP + BUTTON_HEIGHT) };

    DrawButtonContainer(min, max, 0, 0, alpha);

    drawList->AddRectFilledMultiColor
    (
        { min.x + xPadding, min.y + yPadding },
        { max.x - xPadding, max.y - yPadding },
        innerColor0,
        innerColor0,
        innerColor1,
        innerColor1
    );

    const uint32_t sliderColor0 = IM_COL32(57, 241, 0, 255 * alpha);
    const uint32_t sliderColor1 = IM_COL32(2, 106, 0, 255 * alpha);
    xPadding += Scale(1.0f);
    yPadding += Scale(1.0f);

    ImVec2 sliderMin = { min.x + xPadding, min.y + yPadding };
    ImVec2 sliderMax = { max.x - xPadding, max.y - yPadding };
    sliderMax.x = sliderMin.x + (sliderMax.x - sliderMin.x) * progressRatio;
    drawList->AddRectFilledMultiColor(sliderMin, sliderMax, sliderColor0, sliderColor0, sliderColor1, sliderColor1);
}

static bool ConvertPathSet(const nfdpathset_t *pathSet, std::list<std::filesystem::path> &filePaths)
{
    nfdpathsetsize_t pathSetCount = 0;
    if (NFD_PathSet_GetCount(pathSet, &pathSetCount) != NFD_OKAY)
    {
        return false;
    }

    for (nfdpathsetsize_t i = 0; i < pathSetCount; i++)
    {
        nfdnchar_t *pathSetPath = nullptr;
        if (NFD_PathSet_GetPathN(pathSet, i, &pathSetPath) != NFD_OKAY)
        {
            filePaths.clear();
            return false;
        }

        filePaths.emplace_back(std::filesystem::path(pathSetPath));
        NFD_PathSet_FreePathN(pathSetPath);
    }

    return true;
}

static void PickerThreadProcess()
{
    const nfdpathset_t *pathSet;
    nfdresult_t result = NFD_ERROR;
    if (g_currentPickerFolderMode)
    {
        result = NFD_PickFolderMultipleN(&pathSet, nullptr);
    }
    else
    {
        result = NFD_OpenDialogMultipleN(&pathSet, nullptr, 0, nullptr);
    }
    
    if (result == NFD_OKAY)
    {
        bool pathsConverted = ConvertPathSet(pathSet, g_currentPickerResults);
        NFD_PathSet_Free(pathSet);
    }
    else if (result == NFD_ERROR)
    {
        g_currentPickerErrorMessage = NFD_GetError();
    }

    g_currentPickerResultsReady = true;
}

static void PickerStart(bool folderMode) {
    if (g_currentPickerThread != nullptr)
    {
        g_currentPickerThread->join();
        g_currentPickerThread.reset();
    }

    g_currentPickerResults.clear();
    g_currentPickerFolderMode = folderMode;
    g_currentPickerResultsReady = false;
    g_currentPickerVisible = true;

    // Optional single thread mode for testing on systems that do not interact well with the separate thread being used for NFD.
    constexpr bool singleThreadMode = false;
    if (singleThreadMode)
        PickerThreadProcess();
    else
        g_currentPickerThread = std::make_unique<std::thread>(PickerThreadProcess);
}

static void PickerShow(bool folderMode)
{
    if (g_pickerTutorialCleared[folderMode])
    {
        PickerStart(folderMode);
    }
    else
    {
        g_currentMessagePrompt = Localise(folderMode ? "Installer_Message_FolderPickerTutorial" : "Installer_Message_FilePickerTutorial");
        g_currentMessagePromptConfirmation = false;
        g_pickerTutorialTriggered = true;
        g_pickerTutorialFolderMode = folderMode;
    }
}

static bool ParseSourcePaths(std::list<std::filesystem::path> &paths)
{
    assert((g_currentPage == WizardPage::SelectGameAndUpdate) || (g_currentPage == WizardPage::SelectDLC));

    constexpr size_t failedPathLimit = 5;
    bool isFailedPathsOverLimit = false;
    std::list<std::filesystem::path> failedPaths;
    if (g_currentPage == WizardPage::SelectGameAndUpdate)
    {
        for (const std::filesystem::path &path : paths)
        {
            if (Installer::parseGame(path))
            {
                g_gameSourcePath = path;
            }
            else if (Installer::parseUpdate(path))
            {
                g_updateSourcePath = path;
            }
            else if (failedPaths.size() < failedPathLimit)
            {
                failedPaths.push_back(path);
            }
            else
            {
                isFailedPathsOverLimit = true;
            }
        }
    }
    else if(g_currentPage == WizardPage::SelectDLC)
    {
        for (const std::filesystem::path &path : paths)
        {
            DLC dlc = Installer::parseDLC(path);
            if (dlc != DLC::Unknown)
            {
                g_dlcSourcePaths[DLCIndex(dlc)] = path;
            }
            else if (failedPaths.size() < failedPathLimit)
            {
                failedPaths.push_back(path);
            }
        }
    }

    if (!failedPaths.empty())
    {
        std::stringstream stringStream;
        stringStream << Localise("Installer_Message_InvalidFilesList") << std::endl;
        for (const std::filesystem::path &path : failedPaths)
        {
            std::u8string filenameU8 = path.filename().u8string();
            stringStream << std::endl << "- " << Truncate(std::string(filenameU8.begin(), filenameU8.end()), 32, true, true);
        }

        if (isFailedPathsOverLimit)
            stringStream << std::endl << "- [...]";

        g_currentMessagePrompt = stringStream.str();
        g_currentMessagePromptConfirmation = false;
    }

    return failedPaths.empty();
}

static void DrawLanguagePicker()
{
    bool buttonPressed = false;
    if (g_currentPage == WizardPage::SelectLanguage)
    {
        bool buttonPressed;
        float minX, maxX;
        for (int i = 0; i < 6; i++)
        {
            ComputeButtonColumnCoordinates((i < 3) ? ButtonColumnLeft : ButtonColumnRight, minX, maxX);

            float minusY = (CONTAINER_BUTTON_GAP + BUTTON_HEIGHT) * (float(i % 3));
            ImVec2 min = { minX, g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT - CONTAINER_BUTTON_GAP - BUTTON_HEIGHT - minusY) };
            ImVec2 max = { maxX, g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT - CONTAINER_BUTTON_GAP - minusY) };

            // TODO: The active button should change its style to show an enabled toggle if it matches the current language.

            DrawButton(min, max, LANGUAGE_TEXT[i], false, true, buttonPressed, FLT_MAX, LANGUAGE_ENUM[i] == ELanguage::English);
            if (buttonPressed)
            {
                Config::Language = LANGUAGE_ENUM[i];
            }
        }
    }
}

static void DrawSourcePickers()
{
    bool buttonPressed = false;
    std::list<std::filesystem::path> paths;
    if (g_currentPage == WizardPage::SelectGameAndUpdate || g_currentPage == WizardPage::SelectDLC)
    {
        constexpr float ADD_BUTTON_MAX_TEXT_WIDTH = 160.0f;
        const std::string &addFilesText = Localise("Installer_Button_AddFiles");
        float squashRatio;
        ImVec2 textSize = ComputeTextSize(g_dfsogeistdFont, addFilesText.c_str(), 20.0f, squashRatio, ADD_BUTTON_MAX_TEXT_WIDTH);
        textSize.x += BUTTON_TEXT_GAP;

        ImVec2 min = { g_aspectRatioOffsetX + Scale(CONTAINER_X + BOTTOM_X_GAP), g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT + BOTTOM_Y_GAP) };
        ImVec2 max = { g_aspectRatioOffsetX + Scale(CONTAINER_X + BOTTOM_X_GAP + textSize.x * squashRatio), g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT + BOTTOM_Y_GAP + BUTTON_HEIGHT) };
        DrawButton(min, max, addFilesText.c_str(), false, true, buttonPressed, ADD_BUTTON_MAX_TEXT_WIDTH);
        if (buttonPressed)
        {
            PickerShow(false);
        }

        min.x += Scale(BOTTOM_X_GAP + textSize.x * squashRatio);

        const std::string &addFolderText = Localise("Installer_Button_AddFolder");
        textSize = ComputeTextSize(g_dfsogeistdFont, addFolderText.c_str(), 20.0f, squashRatio, ADD_BUTTON_MAX_TEXT_WIDTH);
        textSize.x += BUTTON_TEXT_GAP;

        max.x = min.x + Scale(textSize.x * squashRatio);
        DrawButton(min, max, addFolderText.c_str(), false, true, buttonPressed, ADD_BUTTON_MAX_TEXT_WIDTH);
        if (buttonPressed)
        {
            PickerShow(true);
        }
    }
}

static void DrawSources()
{
    if (g_currentPage == WizardPage::SelectGameAndUpdate)
    {
        DrawSourceButton(ButtonColumnMiddle, 1.5f, Localise("Installer_Step_Game").c_str(), !g_gameSourcePath.empty());
        DrawSourceButton(ButtonColumnMiddle, 0.5f, Localise("Installer_Step_Update").c_str(), !g_updateSourcePath.empty());
    }

    if (g_currentPage == WizardPage::SelectDLC)
    {
        for (int i = 0; i < 6; i++)
        {
            DrawSourceButton((i < 3) ? ButtonColumnLeft : ButtonColumnRight, float(i % 3), DLC_SOURCE_TEXT[i], !g_dlcSourcePaths[i].empty() || g_dlcInstalled[i]);
        }
    }
}

static void DrawInstallingProgress()
{
    if (g_currentPage == WizardPage::Installing)
    {
        float ratioTarget = g_installerProgressRatioTarget.load();
        g_installerProgressRatioCurrent += (4.0f * ImGui::GetIO().DeltaTime * (ratioTarget - g_installerProgressRatioCurrent));
        DrawProgressBar(g_installerProgressRatioCurrent);

        if (g_installerFinished)
        {
            g_installerThread->join();
            g_installerThread.reset();
            g_installerEndTime = ImGui::GetTime();
            g_currentPage = g_installerFailed ? WizardPage::InstallFailed : WizardPage::InstallSucceeded;
        }
    }
}

static void InstallerThread()
{
    if (!Installer::install(g_installerSources, g_installPath, false, g_installerJournal, [&]() {
        g_installerProgressRatioTarget = float(double(g_installerJournal.progressCounter) / double(g_installerJournal.progressTotal));
    }))
    {
        g_installerFailed = true;
        g_installerErrorMessage = g_installerJournal.lastErrorMessage;

        // Delete all files that were copied.
        Installer::rollback(g_installerJournal);
    }

    // Rest for a bit after finishing the installation, the device is tired
    std::this_thread::sleep_for(std::chrono::seconds(1));
    g_installerFinished = true;
}

static void InstallerStart()
{
    g_currentPage = WizardPage::Installing;
    g_installerStartTime = ImGui::GetTime();
    g_installerEndTime = DBL_MAX;
    g_installerProgressRatioCurrent = 0.0f;
    g_installerProgressRatioTarget = 0.0f;
    g_installerFailed = false;
    g_installerFinished = false;
    g_installerThread = std::make_unique<std::thread>(InstallerThread);
}

static bool InstallerParseSources(std::string &errorMessage)
{
    std::error_code spaceErrorCode;
    std::filesystem::space_info spaceInfo = std::filesystem::space(g_installPath, spaceErrorCode);
    if (!spaceErrorCode)
    {
        g_installerAvailableSize = spaceInfo.available;
    }

    Installer::Input installerInput;
    installerInput.gameSource = g_gameSourcePath;
    installerInput.updateSource = g_updateSourcePath;

    for (std::filesystem::path &path : g_dlcSourcePaths)
    {
        if (!path.empty())
        {
            installerInput.dlcSources.push_back(path);
        }
    }

    bool sourcesParsed = Installer::parseSources(installerInput, g_installerJournal, g_installerSources);
    errorMessage = g_installerJournal.lastErrorMessage;
    return sourcesParsed;
}

static void DrawNextButton()
{
    if (g_currentPage != WizardPage::Installing)
    {
        bool nextButtonEnabled = !g_isDisappearing;
        if (nextButtonEnabled && g_currentPage == WizardPage::SelectGameAndUpdate)
        {
            nextButtonEnabled = !g_gameSourcePath.empty() && !g_updateSourcePath.empty();
        }

        bool skipButton = false;
        if (g_currentPage == WizardPage::SelectDLC)
        {
            skipButton = std::all_of(g_dlcSourcePaths.begin(), g_dlcSourcePaths.end(), [](const std::filesystem::path &path) { return path.empty(); });
        }

        float squashRatio;
        constexpr float NEXT_BUTTON_MAX_TEXT_WIDTH = 100.0f;
        const std::string &buttonText = Localise(skipButton ? "Installer_Button_Skip" : "Installer_Button_Next");
        ImVec2 textSize = ComputeTextSize(g_newRodinFont, buttonText.c_str(), 20.0f, squashRatio, NEXT_BUTTON_MAX_TEXT_WIDTH);
        textSize.x += BUTTON_TEXT_GAP;

        ImVec2 min = { g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH - textSize.x * squashRatio - BOTTOM_X_GAP), g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT + BOTTOM_Y_GAP) };
        ImVec2 max = { g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH - BOTTOM_X_GAP), g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT + BOTTOM_Y_GAP + BUTTON_HEIGHT) };

        bool buttonPressed = false;
        DrawButton(min, max, buttonText.c_str(), false, nextButtonEnabled, buttonPressed, NEXT_BUTTON_MAX_TEXT_WIDTH);

        if (buttonPressed)
        {
            XexPatcher::Result patcherResult;
            if (g_currentPage == WizardPage::SelectGameAndUpdate && (patcherResult = Installer::checkGameUpdateCompatibility(g_gameSourcePath, g_updateSourcePath), patcherResult != XexPatcher::Result::Success))
            {
                g_currentMessagePrompt = Localise("Installer_Message_IncompatibleGameData");
                g_currentMessagePromptConfirmation = false;
            }
            else if (g_currentPage == WizardPage::SelectDLC)
            {
                // Check if any of the DLC was not specified.
                bool dlcIncomplete = false;
                for (int i = 0; (i < int(DLC::Count)) && !dlcIncomplete; i++)
                {
                    if (g_dlcSourcePaths[i].empty() && !g_dlcInstalled[i])
                    {
                        dlcIncomplete = true;
                    }
                }

                bool dlcInstallerMode = g_gameSourcePath.empty();
                std::string sourcesErrorMessage;
                if (!InstallerParseSources(sourcesErrorMessage))
                {
                    // Some of the sources that were provided to the installer are not valid. Restart the file selection process.
                    std::stringstream stringStream;
                    stringStream << Localise("Installer_Message_InvalidFiles");
                    if (!sourcesErrorMessage.empty()) {
                        stringStream << std::endl << std::endl << sourcesErrorMessage;
                    }

                    g_currentMessagePrompt = stringStream.str();
                    g_currentMessagePromptConfirmation = false;
                    g_currentPage = dlcInstallerMode ? WizardPage::SelectDLC : WizardPage::SelectGameAndUpdate;
                }
                else if (dlcIncomplete && !dlcInstallerMode)
                {
                    // Not all the DLC was specified, we show a prompt and await a confirmation before starting the installer.
                    g_currentMessagePrompt = Localise("Installer_Message_DLCWarning");
                    g_currentMessagePromptConfirmation = true;
                }
                else if (skipButton && dlcInstallerMode)
                {
                    // Nothing was selected and the installer was in DLC mode, just close it.
                    g_isDisappearing = true;
                    g_disappearTime = ImGui::GetTime();
                }
                else
                {
                    g_currentPage = WizardPage::CheckSpace;
                }
            }
            else if (g_currentPage == WizardPage::CheckSpace)
            {
                InstallerStart();
            }
            else if (g_currentPage == WizardPage::InstallSucceeded)
            {
                g_isDisappearing = true;
                g_disappearTime = ImGui::GetTime();
            }
            else if (g_currentPage == WizardPage::InstallFailed)
            {
                g_currentPage = g_firstPage;
            }
            else
            {
                g_currentPage = WizardPage(int(g_currentPage) + 1);
            }
        }
    }
}

static void DrawHorizontalBorder(bool bottomBorder)
{
    const uint32_t FADE_COLOR_LEFT = IM_COL32(155, 155, 155, 0);
    const uint32_t SOLID_COLOR = IM_COL32(155, 200, 155, 255);
    const uint32_t FADE_COLOR_RIGHT = IM_COL32(155, 225, 155, 0);
    auto drawList = ImGui::GetForegroundDrawList();
    double borderScale = 1.0 - ComputeMotionInstaller(g_appearTime, g_disappearTime, CONTAINER_LINE_ANIMATION_TIME, CONTAINER_LINE_ANIMATION_DURATION);
    float midX = g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH / 5);
    float minX = std::lerp(g_aspectRatioOffsetX + Scale(CONTAINER_X - BORDER_SIZE - BORDER_OVERSHOOT), midX, borderScale);
    float maxX = std::lerp(g_aspectRatioOffsetX + Scale(CONTAINER_X + CONTAINER_WIDTH + SIDE_CONTAINER_WIDTH + BORDER_OVERSHOOT), midX, borderScale);
    float minY = g_aspectRatioOffsetY + (bottomBorder ? Scale(CONTAINER_Y + CONTAINER_HEIGHT) : Scale(CONTAINER_Y - BORDER_SIZE));
    float maxY = minY + Scale(BORDER_SIZE);
    drawList->AddRectFilledMultiColor
    (
        { minX, minY },
        { midX, maxY },
        FADE_COLOR_LEFT,
        SOLID_COLOR,
        SOLID_COLOR,
        FADE_COLOR_LEFT
    );

    drawList->AddRectFilledMultiColor
    (
        { midX, minY },
        { maxX, maxY },
        SOLID_COLOR,
        FADE_COLOR_RIGHT,
        FADE_COLOR_RIGHT,
        SOLID_COLOR
    );
}

static void DrawVerticalBorder(bool rightBorder)
{
    const uint32_t SOLID_COLOR = IM_COL32(155, rightBorder ? 225 : 155, 155, 255);
    const uint32_t FADE_COLOR = IM_COL32(155, rightBorder ? 225 : 155, 155, 0);
    auto drawList = ImGui::GetForegroundDrawList();
    double borderScale = 1.0 - ComputeMotionInstaller(g_appearTime, g_disappearTime, CONTAINER_LINE_ANIMATION_TIME, CONTAINER_LINE_ANIMATION_DURATION);
    float minX = g_aspectRatioOffsetX + (rightBorder ? Scale(CONTAINER_X + CONTAINER_WIDTH) : Scale(CONTAINER_X - BORDER_SIZE));
    float maxX = minX + Scale(BORDER_SIZE);
    float midY = g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT / 2);
    float minY = std::lerp(g_aspectRatioOffsetY + Scale(CONTAINER_Y - BORDER_OVERSHOOT), midY, borderScale);
    float maxY = std::lerp(g_aspectRatioOffsetY + Scale(CONTAINER_Y + CONTAINER_HEIGHT + BORDER_OVERSHOOT), midY, borderScale);
    drawList->AddRectFilledMultiColor
    (
        { minX, minY },
        { maxX, midY },
        FADE_COLOR,
        FADE_COLOR,
        SOLID_COLOR,
        SOLID_COLOR
    );

    drawList->AddRectFilledMultiColor
    (
        { minX, midY },
        { maxX, maxY },
        SOLID_COLOR,
        SOLID_COLOR,
        FADE_COLOR,
        FADE_COLOR
    );
}

static void DrawBorders()
{
    DrawHorizontalBorder(false);
    DrawHorizontalBorder(true);
    DrawVerticalBorder(false);
    DrawVerticalBorder(true);
}

static void DrawMessagePrompt()
{
    if (g_currentMessagePrompt.empty())
    {
        return;
    }

    bool messageWindowReturned = false;
    if (g_currentMessagePromptConfirmation)
    {
        std::array<std::string, 2> YesNoButtons = { Localise("Common_Yes"), Localise("Common_No") };
        messageWindowReturned = MessageWindow::Open(g_currentMessagePrompt, &g_currentMessageResult, YesNoButtons, 1);
    }
    else
    {
        messageWindowReturned = MessageWindow::Open(g_currentMessagePrompt, &g_currentMessageResult);
    }

    if (messageWindowReturned)
    {
        if (g_currentMessagePromptConfirmation && (g_currentMessageResult == 0) && (g_currentPage == WizardPage::SelectDLC))
        {
            // If user confirms the message prompt that they wish to skip installing the DLC, proceed to the next step.
            g_currentPage = WizardPage::CheckSpace;
        }

        g_currentMessagePrompt.clear();
        g_currentMessageResult = -1;
    }
}

static void PickerCheckTutorial()
{
    if (!g_pickerTutorialTriggered || !g_currentMessagePrompt.empty())
    {
        return;
    }

    PickerStart(g_pickerTutorialFolderMode);
    g_pickerTutorialTriggered = false;
}

static void PickerCheckResults()
{
    if (!g_currentPickerResultsReady)
    {
        return;
    }

    if (!g_currentPickerErrorMessage.empty())
    {
        g_currentMessagePrompt = g_currentPickerErrorMessage;
        g_currentMessagePromptConfirmation = false;
        g_currentPickerErrorMessage.clear();
    }

    if (!g_currentPickerResults.empty() && ParseSourcePaths(g_currentPickerResults))
    {
        g_pickerTutorialCleared[g_pickerTutorialFolderMode] = true;
    }

    g_currentPickerResultsReady = false;
    g_currentPickerVisible = false;
}

void InstallerWizard::Init()
{
    auto &io = ImGui::GetIO();

    g_seuratFont = ImFontAtlasSnapshot::GetFont("FOT-SeuratPro-M.otf");
    g_dfsogeistdFont = ImFontAtlasSnapshot::GetFont("DFSoGeiStd-W7.otf");
    g_newRodinFont = ImFontAtlasSnapshot::GetFont("FOT-NewRodinPro-DB.otf");
    g_installTextures[0] = LOAD_ZSTD_TEXTURE(g_install_001);
    g_installTextures[1] = LOAD_ZSTD_TEXTURE(g_install_002);
    g_installTextures[2] = LOAD_ZSTD_TEXTURE(g_install_003);
    g_installTextures[3] = LOAD_ZSTD_TEXTURE(g_install_004);
    g_installTextures[4] = LOAD_ZSTD_TEXTURE(g_install_005);
    g_installTextures[5] = LOAD_ZSTD_TEXTURE(g_install_006);
    g_installTextures[6] = LOAD_ZSTD_TEXTURE(g_install_007);
    g_installTextures[7] = LOAD_ZSTD_TEXTURE(g_install_008);
    g_milesElectricIcon = LOAD_ZSTD_TEXTURE(g_miles_electric_icon);
    g_arrowCircle = LOAD_ZSTD_TEXTURE(g_arrow_circle);
    g_pulseInstall = LOAD_ZSTD_TEXTURE(g_pulse_install);
    g_upHedgeDev = LOAD_ZSTD_TEXTURE(g_hedgedev);
}

void InstallerWizard::Draw()
{
    if (!s_isVisible)
    {
        return;
    }

    ResetCursorRects();
    DrawBackground();
    DrawLeftImage();
    DrawScanlineBars();
    DrawDescriptionContainer();
    DrawLanguagePicker();
    DrawSourcePickers();
    DrawSources();
    DrawInstallingProgress();
    DrawNextButton();
    DrawBorders();
    DrawMessagePrompt();
    PickerCheckTutorial();
    PickerCheckResults();

    if (g_isDisappearing)
    {
        const double disappearDuration = ALL_ANIMATIONS_FULL_DURATION / 60.0;
        if (ImGui::GetTime() > (g_disappearTime + disappearDuration))
        {
            s_isVisible = false;
        }
    }
}

void InstallerWizard::Shutdown()
{
    // Wait for and erase the threads.
    if (g_installerThread != nullptr)
    {
        g_installerThread->join();
        g_installerThread.reset();
    }

    if (g_currentPickerThread != nullptr)
    {
        g_currentPickerThread->join();
        g_currentPickerThread.reset();
    }

    // Erase the sources.
    g_installerSources.game.reset();
    g_installerSources.update.reset();
    g_installerSources.dlc.clear();
    
    // Make sure the GPU is not currently active before deleting these textures.
    Video::WaitForGPU();

    // Erase the textures.
    g_milesElectricIcon.reset();
    g_arrowCircle.reset();
    g_pulseInstall.reset();

    for (auto &texture : g_installTextures)
    {
        texture.reset();
    }
}

bool InstallerWizard::Run(std::filesystem::path installPath, bool skipGame)
{
    g_installPath = installPath;

    EmbeddedPlayer::Init();
    NFD_Init();

    // Guarantee one controller is initialized. We'll rely on SDL's event loop to get the controller events.
    XAMINPUT_STATE inputState;
    hid::GetState(0, &inputState);

    if (skipGame)
    {
        for (int i = 0; i < int(DLC::Count); i++)
        {
            g_dlcInstalled[i] = Installer::checkDLCInstall(g_installPath, DLC(i + 1));
        }

        g_firstPage = WizardPage::SelectDLC;
        g_currentPage = g_firstPage;
    }

    GameWindow::SetFullscreenCursorVisibility(true);
    s_isVisible = true;

    while (s_isVisible)
    {
        Video::WaitOnSwapChain();
        SDL_PumpEvents();
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        GameWindow::Update();
        Video::Present();
    }

    GameWindow::SetFullscreenCursorVisibility(false);
    NFD_Quit();

    InstallerWizard::Shutdown();
    EmbeddedPlayer::Shutdown();

    return true;
}
