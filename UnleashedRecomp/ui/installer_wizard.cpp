#include "installer_wizard.h"

#include <nfd.h>

#include <install/installer.h>
#include <gpu/video.h>
#include <locale/locale.h>
#include <ui/imgui_utils.h>
#include <ui/window.h>

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

static constexpr double ARROW_CIRCLE_SPIN_FACTOR = 2;

static constexpr double ALL_ANIMATIONS_FULL_DURATION = CONTAINER_INNER_TIME + CONTAINER_INNER_DURATION;

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

static std::filesystem::path g_installPath = ".";
static std::filesystem::path g_gameSourcePath;
static std::filesystem::path g_updateSourcePath;
static std::array<std::filesystem::path, int(DLC::Count)> g_dlcSourcePaths;
static std::array<bool, int(DLC::Count)> g_dlcInstalled = {};
static std::array<std::unique_ptr<GuestTexture>, 8> g_installTextures;
static std::unique_ptr<GuestTexture> g_milesElectricIcon;
static std::unique_ptr<GuestTexture> g_arrowCircle;
static Journal g_installerJournal;
static Installer::Sources g_installerSources;
static uint64_t g_installerAvailableSize = 0;
static std::unique_ptr<std::thread> g_installerThread;
static double g_installerStartTime = 0.0;
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

const char CREDITS_TEXT[] = "Sajid (RIP)";

const char *WIZARD_TEXT[] =
{
    "Please select a language.\n",
    "Welcome to Unleashed Recompiled!\n\nMake sure you have a copy of Sonic Unleashed's files for Xbox 360 before proceeding with the installation.",
    "Select the files for the Game and the Update. You can use digital dumps (PIRS), a folder with the game's contents or a disc image (ISO).",
    "Select the files for the DLC Packs. These can be digital dumps (PIRS) or a folder with their contents.",
    "The content will be installed to the program's folder. Please confirm you have enough space available.\n\n",
    "Please wait while the content is being installed...",
    "Installation is complete.\n\nThis project's been brought to you by:\n\n",
    "Installation has failed.\n\nError:\n\n"
};

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

const char GAME_SOURCE_TEXT[] = "FULL GAME";
const char UPDATE_SOURCE_TEXT[] = "UPDATE";
const char *DLC_SOURCE_TEXT[] =
{
    "SPAGONIA",
    "CHUN-NAN",
    "MAZURI",
    "HOLOSKA",
    "APOTOS & SHAMAR",
    "EMPIRE CITY & ADABAT",
};

const char REQUIRED_SPACE_TEXT[] = "Required space";
const char AVAILABLE_SPACE_TEXT[] = "Available space";

static int DLCIndex(DLC dlc)
{
    assert(dlc != DLC::Unknown);
    return (int)(dlc) - 1;
}

static double ComputeMotionInstaller(double timeAppear, double timeDisappear, double offset, double total)
{
    return ComputeMotion(timeAppear, offset, total) * (1.0 - ComputeMotion(timeDisappear, ALL_ANIMATIONS_FULL_DURATION - offset - total, total));
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
    ImVec2 min = { Scale(IMAGE_X), Scale(IMAGE_Y) };
    ImVec2 max = { Scale(IMAGE_X + IMAGE_WIDTH), Scale(IMAGE_Y + IMAGE_HEIGHT) };
    drawList->AddImage(guestTexture, min, max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, a));

    min.y = (min.y + max.y) / 2.0f;
    drawList->AddRectFilledMultiColor(min, max, IM_COL32_BLACK_TRANS, IM_COL32_BLACK_TRANS, IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 255));
}

static void DrawHeaderIcons()
{
    auto drawList = ImGui::GetForegroundDrawList();

    float iconsPosX = 255.0f;
    float iconsPosY = 79.0f;
    float iconsScale = 58;

    // Miles Electric Icon
    float milesIconMotion = ComputeMotionInstaller(g_appearTime, g_disappearTime, MILES_ICON_ANIMATION_TIME, MILES_ICON_ANIMATION_DURATION);
    float milesIconScale = iconsScale * (2 - milesIconMotion);

    GuestTexture* milesElectricIconTexture = g_milesElectricIcon.get();
    ImVec2 milesElectricMin = { Scale(iconsPosX - milesIconScale / 2), Scale(iconsPosY - milesIconScale / 2) };
    ImVec2 milesElectricMax = { Scale(iconsPosX + milesIconScale / 2), Scale(iconsPosY + milesIconScale / 2) };
    drawList->AddImage(milesElectricIconTexture, milesElectricMin, milesElectricMax, ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255 * milesIconMotion));

    // Arrow Circle Icon
    if (g_currentPage == WizardPage::Installing)
    {
        GuestTexture* arrowCircleTexture = g_arrowCircle.get();
        ImVec2 arrowCircleMin = { Scale(iconsPosX - iconsScale / 2), Scale(iconsPosY - iconsScale / 2) };
        ImVec2 arrowCircleMax = { Scale(iconsPosX + iconsScale / 2), Scale(iconsPosY + iconsScale / 2) };

        ImVec2 center = { Scale(iconsPosX), Scale(iconsPosY) };
        float currentAngle = g_arrowCircleCurrentRotation * (3.14159f / 180.0f); // Rotation angle in radians
        float cos_a = cosf(currentAngle);
        float sin_a = sinf(currentAngle);

        // Calculate rotated corners
        ImVec2 corners[4] = {
            ImRotate(ImVec2(arrowCircleMin.x - center.x, arrowCircleMin.y - center.y), cos_a, sin_a),
            ImRotate(ImVec2(arrowCircleMax.x - center.x, arrowCircleMin.y - center.y), cos_a, sin_a),
            ImRotate(ImVec2(arrowCircleMax.x - center.x, arrowCircleMax.y - center.y), cos_a, sin_a),
            ImRotate(ImVec2(arrowCircleMin.x - center.x, arrowCircleMax.y - center.y), cos_a, sin_a),
        };

        for (int i = 0; i < 4; ++i) {
            corners[i].x += center.x; // Add center.x to corner.x
            corners[i].y += center.y; // Add center.y to corner.y
        }

        drawList->AddImageQuad(arrowCircleTexture, corners[0], corners[1], corners[2], corners[3], ImVec2(0, 0), ImVec2(1, 0), ImVec2(1, 1), ImVec2(0, 1), IM_COL32(255, 255, 255, 96));

        // Update rotation for next frame
        g_arrowCircleCurrentRotation = fmodf(g_arrowCircleCurrentRotation - ARROW_CIRCLE_SPIN_FACTOR, 360.0f);
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
    int textAlpha = std::lround(255.0f * ComputeMotionInstaller(g_appearTime, g_disappearTime, TITLE_ANIMATION_TIME, TITLE_ANIMATION_DURATION));
    DrawTextWithOutline<int>(g_dfsogeistdFont, Scale(42.0f), { Scale(285.0f), Scale(57.0f) }, IM_COL32(255, 195, 0, textAlpha), headerText.c_str(), 4, IM_COL32(0, 0, 0, textAlpha));

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
}

static float AlignToNextGrid(float value)
{
    return floor(value / GRID_SIZE) * GRID_SIZE;
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

    const uint32_t gridColor = IM_COL32(0, 33, 0, (isTextArea ? 128 : 255) * gridAlpha);
    const uint32_t gridOverlayColor = IM_COL32(0, 32, 0, 128 * gridOverlayAlpha);

    float gridSize = Scale(GRID_SIZE);

    SetShaderModifier(IMGUI_SHADER_MODIFIER_CHECKERBOARD);
    drawList->AddRectFilled(min, max, gridColor);
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

    ImVec2 descriptionMin = { Scale(AlignToNextGrid(CONTAINER_X)), Scale(AlignToNextGrid(CONTAINER_Y)) };
    ImVec2 descriptionMax = { Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH)), Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT)) };
    DrawContainer(descriptionMin, descriptionMax, true);

    char descriptionText[512];
    strncpy(descriptionText, WIZARD_TEXT[int(g_currentPage)], sizeof(descriptionText) - 1);

    if (g_currentPage == WizardPage::CheckSpace)
    {
        constexpr double DivisorGiB = (1024.0 * 1024.0 * 1024.0);
        double requiredGiB = double(g_installerSources.totalSize) / DivisorGiB;
        double availableGiB = double(g_installerAvailableSize) / DivisorGiB;
        snprintf(descriptionText, sizeof(descriptionText), "%s%s: %2.2f GiB\n\n%s: %2.2f GiB", WIZARD_TEXT[int(g_currentPage)], REQUIRED_SPACE_TEXT, requiredGiB, AVAILABLE_SPACE_TEXT, availableGiB);
    }
    else if (g_currentPage == WizardPage::InstallSucceeded)
    {
        strncat(descriptionText, CREDITS_TEXT, sizeof(descriptionText) - 1);
    }
    else if (g_currentPage == WizardPage::InstallFailed)
    {
        strncat(descriptionText, g_installerErrorMessage.c_str(), sizeof(descriptionText) - 1);
    }

    double textAlpha = ComputeMotionInstaller(g_appearTime, g_disappearTime, CONTAINER_INNER_TIME, CONTAINER_INNER_DURATION);
    auto clipRectMin = drawList->GetClipRectMin();
    auto clipRectMax = drawList->GetClipRectMax();
    auto size = Scale(26.0f);
    drawList->AddText
    (
        g_seuratFont,
        size,
        { clipRectMin.x, clipRectMin.y },
        IM_COL32(255, 255, 255, 255 * textAlpha),
        descriptionText,
        0,
        clipRectMax.x - clipRectMin.x
    );

    drawList->PopClipRect();

    ImVec2 sideMin = { descriptionMax.x, descriptionMin.y };
    ImVec2 sideMax = { Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH + SIDE_CONTAINER_WIDTH)), descriptionMax.y };
    DrawContainer(sideMin, sideMax, false);
    drawList->PopClipRect();
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

static ImVec2 ComputeTextSizeAndRatio(ImFont *font, const char *text, float &size, float maxTextWidth = FLT_MAX)
{
    ImVec2 textSize = font->CalcTextSizeA(size, FLT_MAX, 0.0f, text);
    if (textSize.x > maxTextWidth)
    {
        float shrinkRatio = maxTextWidth / textSize.x;
        size *= shrinkRatio;
        textSize.x *= shrinkRatio;
        textSize.y *= shrinkRatio;
    }

    return textSize;
}

static ImVec2 ComputeTextSize(ImFont *font, const char *text, float size, float maxTextWidth = FLT_MAX)
{
    return ComputeTextSizeAndRatio(font, text, size, maxTextWidth);
}

static void DrawButton(ImVec2 min, ImVec2 max, const char *buttonText, bool sourceButton, bool buttonEnabled, bool &buttonPressed, float maxTextWidth = FLT_MAX)
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
    if (g_currentMessagePrompt.empty() && !sourceButton && buttonEnabled && (alpha >= 1.0f) && ImGui::IsMouseHoveringRect(min, max, false))
    {
        baser = 48;
        baseg = 32;

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            buttonPressed = true;
        }
    }

    DrawButtonContainer(min, max, baser, baseg, alpha);

    ImFont *font = sourceButton ? g_newRodinFont : g_dfsogeistdFont;
    float size = Scale(sourceButton ? 15.0f : 20.0f);
    ImVec2 textSize = ComputeTextSizeAndRatio(font, buttonText, size, Scale(maxTextWidth));
    min.x += ((max.x - min.x) - textSize.x) / 2.0f;
    min.y += ((max.y - min.y) - textSize.y) / 2.0f;

    if (!sourceButton)
    {
        // Fixes slight misalignment caused by this particular font.
        min.y -= Scale(1.0f);
    }
    
    SetGradient
    (
        min,
        { min.x + textSize.x, min.y + textSize.y },
        IM_COL32(baser + 192, 255, 0, 255),
        IM_COL32(baser + 128, baseg + 170, 0, 255)
    );

    DrawTextWithOutline<int>
    (
        font,
        size,
        min,
        IM_COL32(255, 255, 255, 255 * alpha),
        buttonText,
        2,
        IM_COL32(baser, baseg, 0, 255 * alpha)
    );

    ResetGradient();
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
        minX = Scale(AlignToNextGrid(CONTAINER_X) + CONTAINER_BUTTON_GAP);
        maxX = Scale(AlignToNextGrid(CONTAINER_X) + CONTAINER_BUTTON_GAP + CONTAINER_BUTTON_WIDTH);
        break;
    case ButtonColumnMiddle:
        minX = Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH / 2.0f) - CONTAINER_BUTTON_WIDTH / 2.0f);
        maxX = Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH / 2.0f) + CONTAINER_BUTTON_WIDTH / 2.0f);
        break;
    case ButtonColumnRight:
        minX = Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH) - CONTAINER_BUTTON_GAP - CONTAINER_BUTTON_WIDTH);
        maxX = Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH) - CONTAINER_BUTTON_GAP);
        break;
    }
}

static void DrawSourceButton(ButtonColumn buttonColumn, float yRatio, const char *sourceText, bool sourceSet)
{
    bool buttonPressed;
    float minX, maxX;
    ComputeButtonColumnCoordinates(buttonColumn, minX, maxX);

    float minusY = (CONTAINER_BUTTON_GAP + BUTTON_HEIGHT) * yRatio;
    ImVec2 min = { minX, Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) - CONTAINER_BUTTON_GAP - BUTTON_HEIGHT - minusY) };
    ImVec2 max = { maxX, Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) - CONTAINER_BUTTON_GAP - minusY) };
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
    ImVec2 min = { Scale(AlignToNextGrid(CONTAINER_X) + BOTTOM_X_GAP), Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) + BOTTOM_Y_GAP) };
    ImVec2 max = { Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH) - BOTTOM_X_GAP), Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) + BOTTOM_Y_GAP + BUTTON_HEIGHT) };

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
        char *pathSetPath = nullptr;
        if (NFD_PathSet_GetPathU8(pathSet, i, &pathSetPath) != NFD_OKAY)
        {
            filePaths.clear();
            return false;
        }

        filePaths.emplace_back(std::filesystem::path(std::u8string_view((const char8_t *)(pathSetPath))));
        NFD_PathSet_FreePathU8(pathSetPath);
    }

    return true;
}

static bool ShowFilesPicker(std::list<std::filesystem::path> &filePaths)
{
    filePaths.clear();

    const nfdpathset_t *pathSet;
    nfdresult_t result = NFD_OpenDialogMultipleU8(&pathSet, nullptr, 0, nullptr);
    if (result == NFD_OKAY)
    {
        bool pathsConverted = ConvertPathSet(pathSet, filePaths);
        NFD_PathSet_Free(pathSet);
        return pathsConverted;
    }
    else
    {
        return false;
    }
}

static bool ShowFoldersPicker(std::list<std::filesystem::path> &folderPaths)
{
    folderPaths.clear();

    const nfdpathset_t *pathSet;
    nfdresult_t result = NFD_PickFolderMultipleU8(&pathSet, nullptr);
    if (result == NFD_OKAY)
    {
        bool pathsConverted = ConvertPathSet(pathSet, folderPaths);
        NFD_PathSet_Free(pathSet);
        return pathsConverted;
    }
    else
    {
        return false;
    }
}

static void ParseSourcePaths(std::list<std::filesystem::path> &paths)
{
    assert((g_currentPage == WizardPage::SelectGameAndUpdate) || (g_currentPage == WizardPage::SelectDLC));

    constexpr size_t failedPathLimit = 5;
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
        stringStream << "Some of the files that were selected are not valid." << std::endl;
        for (const std::filesystem::path &path : failedPaths)
        {
            stringStream << std::endl << path.filename().string();
        }

        g_currentMessagePrompt = stringStream.str();
        g_currentMessagePromptConfirmation = false;
    }
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
            ImVec2 min = { minX, Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) - CONTAINER_BUTTON_GAP - BUTTON_HEIGHT - minusY) };
            ImVec2 max = { maxX, Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) - CONTAINER_BUTTON_GAP - minusY) };

            // TODO: The active button should change its style to show an enabled toggle if it matches the current language.

            DrawButton(min, max, LANGUAGE_TEXT[i], false, true, buttonPressed);
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
        ImVec2 textSize = ComputeTextSize(g_dfsogeistdFont, addFilesText.c_str(), 20.0f, ADD_BUTTON_MAX_TEXT_WIDTH);
        textSize.x += BUTTON_TEXT_GAP;

        ImVec2 min = { Scale(AlignToNextGrid(CONTAINER_X) + BOTTOM_X_GAP), Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) + BOTTOM_Y_GAP) };
        ImVec2 max = { Scale(AlignToNextGrid(CONTAINER_X) + BOTTOM_X_GAP + textSize.x), Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) + BOTTOM_Y_GAP + BUTTON_HEIGHT) };
        DrawButton(min, max, addFilesText.c_str(), false, true, buttonPressed, ADD_BUTTON_MAX_TEXT_WIDTH);
        if (buttonPressed && ShowFilesPicker(paths))
        {
            ParseSourcePaths(paths);
        }

        min.x += Scale(BOTTOM_X_GAP + textSize.x);

        const std::string &addFolderText = Localise("Installer_Button_AddFolder");
        textSize = ComputeTextSize(g_dfsogeistdFont, addFolderText.c_str(), 20.0f, ADD_BUTTON_MAX_TEXT_WIDTH);
        textSize.x += BUTTON_TEXT_GAP;

        max.x = min.x + Scale(textSize.x);
        DrawButton(min, max, addFolderText.c_str(), false, true, buttonPressed, ADD_BUTTON_MAX_TEXT_WIDTH);
        if (buttonPressed && ShowFoldersPicker(paths))
        {
            ParseSourcePaths(paths);
        }
    }
}

static void DrawSources()
{
    if (g_currentPage == WizardPage::SelectGameAndUpdate)
    {
        DrawSourceButton(ButtonColumnMiddle, 1.5f, GAME_SOURCE_TEXT, !g_gameSourcePath.empty());
        DrawSourceButton(ButtonColumnMiddle, 0.5f, UPDATE_SOURCE_TEXT, !g_updateSourcePath.empty());
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

    g_installerFinished = true;
}

static void InstallerStart()
{
    g_currentPage = WizardPage::Installing;
    g_installerStartTime = ImGui::GetTime();
    g_installerProgressRatioCurrent = 0.0f;
    g_installerProgressRatioTarget = 0.0f;
    g_installerFailed = false;
    g_installerFinished = false;
    g_installerThread = std::make_unique<std::thread>(InstallerThread);
}

static bool InstallerParseSources()
{
    std::filesystem::space_info spaceInfo = std::filesystem::space(g_installPath);
    g_installerAvailableSize = spaceInfo.available;

    Installer::Input installerInput;
    installerInput.gameSource = g_gameSourcePath;
    installerInput.updateSource = g_updateSourcePath;

    for (std::filesystem::path &path : g_dlcSourcePaths) {
        if (!path.empty())
        {
            installerInput.dlcSources.push_back(path);
        }
    }

    return Installer::parseSources(installerInput, g_installerJournal, g_installerSources);
}

static void DrawNextButton()
{
    if (g_currentPage != WizardPage::Installing) {
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

        const std::string &buttonText = Localise(skipButton ? "Installer_Button_Skip" : "Installer_Button_Next");
        ImVec2 textSize = g_newRodinFont->CalcTextSizeA(20.0f, FLT_MAX, 0.0f, buttonText.c_str());
        textSize.x += BUTTON_TEXT_GAP;

        ImVec2 min = { Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH) - textSize.x - BOTTOM_X_GAP), Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) + BOTTOM_Y_GAP) };
        ImVec2 max = { Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH) - BOTTOM_X_GAP), Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) + BOTTOM_Y_GAP + BUTTON_HEIGHT) };

        bool buttonPressed = false;
        DrawButton(min, max, buttonText.c_str(), false, nextButtonEnabled, buttonPressed);

        if (buttonPressed)
        {
            XexPatcher::Result patcherResult;
            if (g_currentPage == WizardPage::SelectGameAndUpdate && (patcherResult = Installer::checkGameUpdateCompatibility(g_gameSourcePath, g_updateSourcePath), patcherResult != XexPatcher::Result::Success))
            {
                g_currentMessagePrompt = "The specified game and update file are incompatible.\n\nPlease ensure the files are for the same version and region and try again.";
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
                if (!InstallerParseSources())
                {
                    // Some of the sources that were provided to the installer are not valid. Restart the file selection process.
                    g_currentMessagePrompt = "Some of the files that have been provided are not valid.\n\nPlease make sure all the specified files are correct and try again.";
                    g_currentMessagePromptConfirmation = false;
                    g_currentPage = dlcInstallerMode ? WizardPage::SelectDLC : WizardPage::SelectGameAndUpdate;
                }
                else if (dlcIncomplete && !dlcInstallerMode)
                {
                    // Not all the DLC was specified, we show a prompt and await a confirmation before starting the installer.
                    g_currentMessagePrompt = "It is highly recommended that you install all of the DLC, as it includes high quality lighting textures for the stages in each pack.\n\nAre you sure you want to skip this step?";
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
                    // Start the installer outright, this switches to the right page on its own.
                    InstallerStart();
                }
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
    float midX = Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH / 5));
    float minX = std::lerp(Scale(AlignToNextGrid(CONTAINER_X) - BORDER_SIZE - BORDER_OVERSHOOT), midX, borderScale);
    float maxX = std::lerp(Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH + SIDE_CONTAINER_WIDTH) + BORDER_OVERSHOOT), midX, borderScale);
    float minY = bottomBorder ? Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT)) : Scale(AlignToNextGrid(CONTAINER_Y) - BORDER_SIZE);
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
    float minX = rightBorder ? Scale(AlignToNextGrid(CONTAINER_X + CONTAINER_WIDTH)) : Scale(AlignToNextGrid(CONTAINER_X) - BORDER_SIZE);
    float maxX = minX + Scale(BORDER_SIZE);
    float midY = Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT / 2));
    float minY = std::lerp(Scale(AlignToNextGrid(CONTAINER_Y) - BORDER_OVERSHOOT), midY, borderScale);
    float maxY = std::lerp(Scale(AlignToNextGrid(CONTAINER_Y + CONTAINER_HEIGHT) + BORDER_OVERSHOOT), midY, borderScale);
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

    // TODO: Put message prompt here.

    if (false && g_currentPage == WizardPage::SelectDLC)
    {
        // If user confirms the message prompt that they wish to skip installing the DLC, start the installer.
        InstallerStart();
    }
}

void InstallerWizard::Init()
{
    auto &io = ImGui::GetIO();
    constexpr float FONT_SCALE = 2.0f;
    g_seuratFont = io.Fonts->AddFontFromFileTTF("FOT-SeuratPro-M.otf", 26.0f * FONT_SCALE);
    g_dfsogeistdFont = io.Fonts->AddFontFromFileTTF("DFSoGeiStd-W7.otf", 48.0f * FONT_SCALE);
    g_newRodinFont = io.Fonts->AddFontFromFileTTF("FOT-NewRodinPro-DB.otf", 20.0f * FONT_SCALE);
    g_installTextures[0] = LoadTexture(g_install_001, sizeof(g_install_001));
    g_installTextures[1] = LoadTexture(g_install_002, sizeof(g_install_002));
    g_installTextures[2] = LoadTexture(g_install_003, sizeof(g_install_003));
    g_installTextures[3] = LoadTexture(g_install_004, sizeof(g_install_004));
    g_installTextures[4] = LoadTexture(g_install_005, sizeof(g_install_005));
    g_installTextures[5] = LoadTexture(g_install_006, sizeof(g_install_006));
    g_installTextures[6] = LoadTexture(g_install_007, sizeof(g_install_007));
    g_installTextures[7] = LoadTexture(g_install_008, sizeof(g_install_008));
    g_milesElectricIcon = LoadTexture(g_miles_electric_icon, sizeof(g_miles_electric_icon));
    g_arrowCircle = LoadTexture(g_arrow_circle, sizeof(g_arrow_circle));
}

void InstallerWizard::Draw()
{
    if (!s_isVisible)
    {
        return;
    }

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
    // Wait for and erase the thread.
    if (g_installerThread != nullptr)
    {
        g_installerThread->join();
        g_installerThread.reset();
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

    for (auto &texture : g_installTextures)
    {
        texture.reset();
    }
}

bool InstallerWizard::Run(bool skipGame)
{
    NFD_Init();

    if (skipGame)
    {
        for (int i = 0; i < int(DLC::Count); i++)
        {
            g_dlcInstalled[i] = Installer::checkDLCInstall(g_installPath, DLC(i + 1));
        }

        g_firstPage = WizardPage::SelectDLC;
        g_currentPage = g_firstPage;
    }

    Window::SetCursorAllowed(true);
    s_isVisible = true;

    while (s_isVisible)
    {
        SDL_PumpEvents();
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
        Window::Update();
        Video::HostPresent();
    }

    Window::SetCursorAllowed(false);
    NFD_Quit();

    InstallerWizard::Shutdown();

    return true;
}
