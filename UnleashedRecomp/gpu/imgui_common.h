#pragma once

#define IMGUI_SHADER_MODIFIER_NONE            0
#define IMGUI_SHADER_MODIFIER_SCANLINE        1
#define IMGUI_SHADER_MODIFIER_CHECKERBOARD    2
#define IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON 3

#ifdef __cplusplus

enum class ImGuiCallback : int32_t
{
    SetGradient = -1,
    SetShaderModifier = -2
};

union ImGuiCallbackData
{
    struct
    {
        float gradientMin[2];
        float gradientMax[2];
        uint32_t gradientTop;
        uint32_t gradientBottom;
    } setGradient;

    struct
    {
        uint32_t shaderModifier;
    } setShaderModifier;
};

#endif
