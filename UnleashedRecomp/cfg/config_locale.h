#pragma once

#include "config_detail.h"

#define CONFIG_DEFINE_LOCALE(name) \
    inline static std::unordered_map<ELanguage, std::tuple<std::string, std::string>> g_##name##_locale =

#define CONFIG_DEFINE_ENUM_LOCALE(type) \
    inline static std::unordered_map<ELanguage, std::unordered_map<type, std::tuple<std::string, std::string>>> g_##type##_locale =

CONFIG_DEFINE_ENUM_LOCALE(bool)
{
    {
        ELanguage::English,
        {
            { true,  { "ON", "" } },
            { false, { "OFF", "" } }
        }
    },
    {
        ELanguage::Japanese,
        {
            { true,  { "オン", "" } },
            { false, { "オフ", "" } }
        }
    },
    {
        ELanguage::German,
        {
            { true,  { "EIN", "" } },
            { false, { "AUS", "" } }
        }
    },
    {
        ELanguage::French,
        {
            { true,  { "OUI", "" } },
            { false, { "NON", "" } }
        }
    },
    {
        ELanguage::Spanish,
        {
            { true,  { "SÍ", "" } },
            { false, { "NO", "" } }
        }
    },
    {
        ELanguage::Italian,
        {
            { true,  { "SÌ", "" } },
            { false, { "NO", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(Language)
{
    { ELanguage::English,  { "Language", "Change the language used for text and logos." } },
    { ELanguage::Japanese, { "言語", "[PLACEHOLDER]" } },
    { ELanguage::German,   { "Sprache", "[PLACEHOLDER]" } },
    { ELanguage::French,   { "Langue", "[PLACEHOLDER]" } },
    { ELanguage::Spanish,  { "Idioma", "[PLACEHOLDER]" } },
    { ELanguage::Italian,  { "Lingua", "[PLACEHOLDER]" } }
};

CONFIG_DEFINE_ENUM_LOCALE(ELanguage)
{
    {
        ELanguage::English,
        {
            { ELanguage::English,  { "ENGLISH", "" } },
            { ELanguage::Japanese, { "日本語", "" } },
            { ELanguage::German,   { "DEUTSCH", "" } },
            { ELanguage::French,   { "FRANÇAIS", "" } },
            { ELanguage::Spanish,  { "ESPAÑOL", "" } },
            { ELanguage::Italian,  { "ITALIANO", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(Hints)
{
    { ELanguage::English, { "Hints", "Show hint rings in stages." } }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English, { "Control Tutorial", "Show controller hints in stages." } }
};

CONFIG_DEFINE_LOCALE(SaveScoreAtCheckpoints)
{
    { ELanguage::English, { "Save Score at Checkpoints", "Keep your score from the last checkpoint upon respawning." } }
};

CONFIG_DEFINE_LOCALE(UnleashGaugeBehaviour)
{
    { ELanguage::English, { "Unleash Gauge Behavior", "Change how the Unleash gauge behaves." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EUnleashGaugeBehaviour)
{
    {
        ELanguage::English,
        {
            { EUnleashGaugeBehaviour::Original, { "ORIGINAL", "Original: the gauge will drain at all times regardless." } },
            { EUnleashGaugeBehaviour::Revised,  { "REVISED", "Revised: the gauge will only drain when the player can move." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(WerehogHubTransformVideo)
{
    { ELanguage::English, { "Transformation Cutscene", "Play the Werehog transformation cutscene when changing the time of day in the hub areas." } }
};

CONFIG_DEFINE_LOCALE(LogoSkip)
{
    { ELanguage::English, { "Skip Intro Logos", "Skip the logos during the game's boot sequence." } }
};

CONFIG_DEFINE_LOCALE(CameraXInvert)
{
    { ELanguage::English, { "Invert Camera X", "Toggle between inverted left and right camera movement." } }
};

CONFIG_DEFINE_LOCALE(CameraYInvert)
{
    { ELanguage::English, { "Invert Camera Y", "Toggle between inverted up and down camera movement." } }
};

CONFIG_DEFINE_LOCALE(XButtonHoming)
{
    { ELanguage::English, { "Homing Attack on Boost", "Toggle between using the boost button or the jump button for homing attack." } }
};

CONFIG_DEFINE_LOCALE(UnleashCancel)
{
    { ELanguage::English, { "Allow Cancelling Unleash", "Allow Unleash to be cancelled at the cost of some energy by pressing the input again." } }
};

CONFIG_DEFINE_LOCALE(BackgroundInput)
{
    { ELanguage::English, { "Allow Background Input", "Accept controller input whilst the game window is unfocused." } }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English, { "Music Volume", "Adjust the volume for the music." } }
};

CONFIG_DEFINE_LOCALE(SEVolume)
{
    { ELanguage::English, { "Effects Volume", "Adjust the volume for sound effects." } }
};

CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English, { "Voice Language", "Change the language used for character voices." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EVoiceLanguage)
{
    {
        ELanguage::English,
        {
            { EVoiceLanguage::English,  { "ENGLISH", "" } },
            { EVoiceLanguage::Japanese, { "日本語", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { ELanguage::English, { "Subtitles", "Show subtitles during dialogue." } }
};

CONFIG_DEFINE_LOCALE(WerehogBattleMusic)
{
    { ELanguage::English, { "Battle Theme", "Play the Werehog battle theme during combat.\n\nThis option will apply the next time you're in combat." } }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English, { "Window Size", "Adjust the size of the game window in windowed mode." } }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English, { "Resolution Scale", "Adjust the internal resolution of the game.\n\n%dx%d" } }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English, { "Fullscreen", "Toggle between borderless fullscreen or windowed mode." } }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English, { "V-Sync", "[PLACEHOLDER]" } }
};

CONFIG_DEFINE_LOCALE(TripleBuffering)
{
    { ELanguage::English, { "Triple Buffering", "[PLACEHOLDER]" } }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English, { "FPS", "[PLACEHOLDER]" } }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English, { "Brightness", "Adjust the brightness level of the game." } }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English, { "Anti-Aliasing", "Adjust the amount of smoothing applied to jagged edges." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAntiAliasing)
{
    {
        ELanguage::English,
        {
            { EAntiAliasing::None, { "NONE", "" } },
        }
    }
};

CONFIG_DEFINE_LOCALE(AlphaToCoverage)
{
    { ELanguage::English, { "Transparency Anti-Aliasing", "Apply anti-aliasing to alpha transparent textures." } }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English, { "Shadow Resolution", "[PLACEHOLDER]" } }
};

CONFIG_DEFINE_ENUM_LOCALE(EShadowResolution)
{
    {
        ELanguage::English,
        {
            { EShadowResolution::Original, { "ORIGINAL", "Original: the game will automatically determine the resolution of the shadows." } },
        }
    }
};

CONFIG_DEFINE_LOCALE(GITextureFiltering)
{
    { ELanguage::English, { "GI Texture Filtering", "[PLACEHOLDER]" } }
};

CONFIG_DEFINE_ENUM_LOCALE(EGITextureFiltering)
{
    {
        ELanguage::English,
        {
            { EGITextureFiltering::Bilinear, { "BILINEAR", "" } },
            { EGITextureFiltering::Bicubic,  { "BICUBIC", "" } },
        }
    }
};

CONFIG_DEFINE_LOCALE(MotionBlur)
{
    { ELanguage::English, { "Motion Blur", "Use per-object motion blur and radial blur." } }
};

CONFIG_DEFINE_LOCALE(Xbox360ColourCorrection)
{
    { ELanguage::English, { "Xbox 360 Color Correction", "Use the warm tint from the Xbox 360 version of the game." } }
};

CONFIG_DEFINE_LOCALE(MovieScaleMode)
{
    { ELanguage::English, { "Movie Scale Mode", "Change how the movie player scales to the display." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EMovieScaleMode)
{
    {
        ELanguage::English,
        {
            { EMovieScaleMode::Stretch, { "STRETCH", "Stretch: the movie will stretch to the display." } },
            { EMovieScaleMode::Fit,     { "FIT", "Fit: the movie will maintain its aspect ratio and fit to the display." } },
            { EMovieScaleMode::Fill,    { "FILL", "Fill: the movie will scale past the bounds of the display if it doesn't match the aspect ratio." } },
        }
    }
};

CONFIG_DEFINE_LOCALE(UIScaleMode)
{
    { ELanguage::English, { "UI Scale Mode", "Change how the UI scales to the display." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EUIScaleMode)
{
    {
        ELanguage::English,
        {
            { EUIScaleMode::Stretch, { "STRETCH", "Stretch: the UI will stretch to the display." } },
            { EUIScaleMode::Edge,    { "EDGE", "Edge: the UI will anchor to the edges of the display." } },
            { EUIScaleMode::Centre,  { "CENTER", "Center: the UI will anchor to the center of the display." } },
        }
    }
};
