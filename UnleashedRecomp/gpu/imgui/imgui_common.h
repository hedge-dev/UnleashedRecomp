#pragma once

#define IMGUI_SHADER_MODIFIER_NONE            0
#define IMGUI_SHADER_MODIFIER_SCANLINE        1
#define IMGUI_SHADER_MODIFIER_CHECKERBOARD    2
#define IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON 3
#define IMGUI_SHADER_MODIFIER_TEXT_SKEW       4
#define IMGUI_SHADER_MODIFIER_MARQUEE_FADE    5
#define IMGUI_SHADER_MODIFIER_GRAYSCALE       6
#define IMGUI_SHADER_MODIFIER_TITLE_BEVEL     7
#define IMGUI_SHADER_MODIFIER_CATEGORY_BEVEL  8

#ifdef __cplusplus

enum class ImGuiCallback : int32_t
{
    SetGradient = -1,
    SetShaderModifier = -2,
    SetOrigin = -3,
    SetScale = -4,
    SetMarqueeFade = -5,
    SetOutline = -6,
    // -8 is ImDrawCallback_ResetRenderState, don't use!
};

union ImGuiCallbackData
{
    struct
    {
        float boundsMin[2];
        float boundsMax[2];
        uint32_t gradientTop;
        uint32_t gradientBottom;
    } setGradient;

    struct
    {
        uint32_t shaderModifier;
    } setShaderModifier;

    struct
    {
        float origin[2];
    } setOrigin;

    struct
    {
        float scale[2];
    } setScale;

    struct
    {
        float boundsMin[2];
        float boundsMax[2];
    } setMarqueeFade;

    struct
    {
        float outline;
    } setOutline;
};

extern ImGuiCallbackData* AddImGuiCallback(ImGuiCallback callback);

extern void ResetImGuiCallbacks();

#endif
