#pragma once

#include "config_detail.h"

#define CONFIG_DEFINE_LOCALE(name) \
    inline static std::unordered_map<ELanguage, std::string> g_##name##_locale =

#define CONFIG_DEFINE_DESCRIPTION_LOCALE(name) \
    inline static std::unordered_map<ELanguage, std::string> g_##name##_desc_locale =

#define CONFIG_DEFINE_ENUM_LOCALE(type) \
    inline static std::unordered_map<ELanguage, std::unordered_map<type, std::string>> g_##type##_locale =

#define CONFIG_DEFINE_ENUM_DESCRIPTION_LOCALE(type) \
    inline static std::unordered_map<ELanguage, std::unordered_map<type, std::string>> g_##type##_desc_locale =

CONFIG_DEFINE_ENUM_LOCALE(bool)
{
    {
        ELanguage::English, { { true, "ON" }, { false, "OFF" } }
    },
    {
        ELanguage::Japanese, { { true, "オン" }, { false, "オフ" } }
    },
    {
        ELanguage::German, { { true, "EIN" }, { false, "AUS" } }
    },
    {
        ELanguage::French, { { true, "OUI" }, { false, "NON" } }
    },
    {
        ELanguage::Spanish, { { true, "SÍ" }, { false, "NO" } }
    },
    {
        ELanguage::Italian, { { true, "SÌ" }, { false, "NO" } }
    }
};

CONFIG_DEFINE_LOCALE(Language)
{
    { ELanguage::English,  "Language" },
    { ELanguage::Japanese, "言語" },
    { ELanguage::German,   "Sprache" },
    { ELanguage::French,   "Langue" },
    { ELanguage::Spanish,  "Idioma" },
    { ELanguage::Italian,  "Lingua" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Language)
{
    { ELanguage::English, "The language used for text and logos." }
};

CONFIG_DEFINE_ENUM_LOCALE(ELanguage)
{
    {
        ELanguage::English,
        {
            { ELanguage::English,  "ENGLISH" },
            { ELanguage::Japanese, "日本語" },
            { ELanguage::German,   "DEUTSCH" },
            { ELanguage::French,   "FRANÇAIS" },
            { ELanguage::Spanish,  "ESPAÑOL" },
            { ELanguage::Italian,  "ITALIANO" },
        }
    }
};

CONFIG_DEFINE_LOCALE(Hints)
{
    { ELanguage::English, "Hints" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Hints)
{
    { ELanguage::English, "Determines whether to show hint rings in stages." }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English, "Control Tutorial" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(ControlTutorial)
{
    { ELanguage::English, "Determines whether to show controller hints in stages." }
};

CONFIG_DEFINE_LOCALE(SaveScoreAtCheckpoints)
{
    { ELanguage::English, "Save Score at Checkpoints" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(SaveScoreAtCheckpoints)
{
    { ELanguage::English, "Determines whether to save your score when reaching checkpoints.\n\nUpon respawning, your last checkpoint score will be restored." }
};

CONFIG_DEFINE_LOCALE(UnleashOutOfControlDrain)
{
    { ELanguage::English, "Deplete Unleash Gauge whilst Out Of Control" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(UnleashOutOfControlDrain)
{
    { ELanguage::English, "Determines whether to continue depleting the Unleash gauge whilst the player is locked in place during transitions." }
};

CONFIG_DEFINE_LOCALE(WerehogHubTransformVideo)
{
    { ELanguage::English, "Werehog Transformation Cutscene" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(WerehogHubTransformVideo)
{
    { ELanguage::English, "Determines whether to play the Werehog transformation cutscene when changing time of day in the hub areas." }
};

CONFIG_DEFINE_LOCALE(LogoSkip)
{
    { ELanguage::English, "Skip Intro Logos" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(LogoSkip)
{
    { ELanguage::English, "Determines whether to skip the logos during the game's boot sequence." }
};

CONFIG_DEFINE_LOCALE(CameraXInvert)
{
    { ELanguage::English, "Invert Camera X" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(CameraXInvert)
{
    { ELanguage::English, "Determines whether to invert the camera's left and right movement." }
};

CONFIG_DEFINE_LOCALE(CameraYInvert)
{
    { ELanguage::English, "Invert Camera Y" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(CameraYInvert)
{
    { ELanguage::English, "Determines whether to invert the camera's up and down movement." }
};

CONFIG_DEFINE_LOCALE(XButtonHoming)
{
    { ELanguage::English, "Homing Attack on Boost" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(XButtonHoming)
{
    { ELanguage::English, "Determines whether to bind the homing attack to the boost button." }
};

CONFIG_DEFINE_LOCALE(UnleashCancel)
{
    { ELanguage::English, "Allow Cancelling Unleash" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(UnleashCancel)
{
    { ELanguage::English, "Determines whether to allow cancelling Unleash by pressing the input again." }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English, "Music Volume" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(MusicVolume)
{
    { ELanguage::English, "The volume used for the music." }
};

CONFIG_DEFINE_LOCALE(SEVolume)
{
    { ELanguage::English, "SE Volume" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(SEVolume)
{
    { ELanguage::English, "The volume used for sound effects." }
};

CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English, "Voice Language" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(VoiceLanguage)
{
    { ELanguage::English, "The language used for character voices." }
};

CONFIG_DEFINE_ENUM_LOCALE(EVoiceLanguage)
{
    {
        ELanguage::English,
        {
            { EVoiceLanguage::English,  "ENGLISH" },
            { EVoiceLanguage::Japanese, "日本語" }
        }
    }
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { ELanguage::English, "Subtitles" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Subtitles)
{
    { ELanguage::English, "Determines whether to display subtitles during events." }
};

CONFIG_DEFINE_LOCALE(WerehogBattleMusic)
{
    { ELanguage::English, "Werehog Battle Theme" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(WerehogBattleMusic)
{
    { ELanguage::English, "Determines whether to play the battle theme whilst in combat as the Werehog." }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English, "Window Size" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(WindowSize)
{
    { ELanguage::English, "The size of the game window in windowed mode." }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English, "Resolution Scale" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(ResolutionScale)
{
    { ELanguage::English, "The internal resolution the game will render at.\n\n%dx%d" }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English, "Fullscreen" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Fullscreen)
{
    { ELanguage::English, "Determines whether to use borderless fullscreen.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English, "V-Sync" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(VSync)
{
    { ELanguage::English, "Determines whether to use V-Sync.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(TripleBuffering)
{
    { ELanguage::English, "Triple Buffering" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(TripleBuffering)
{
    { ELanguage::English, "Determines whether to use triple buffering.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English, "Brightness" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Brightness)
{
    { ELanguage::English, "The brightness level of the game.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English, "Anti-Aliasing" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(AntiAliasing)
{
    { ELanguage::English, "The amount of smoothing applied to jagged edges.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_ENUM_LOCALE(EAntiAliasing)
{
    {
        ELanguage::English,
        {
            { EAntiAliasing::None, "NONE" },
        }
    }
};

CONFIG_DEFINE_LOCALE(AnisotropicFiltering)
{
    { ELanguage::English, "Anisotropic Filtering" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(AnisotropicFiltering)
{
    { ELanguage::English, "The amount of detail retained by distant textures.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English, "Shadow Resolution" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(ShadowResolution)
{
    { ELanguage::English, "The resolution of real-time shadows.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_ENUM_LOCALE(EShadowResolution)
{
    {
        ELanguage::English,
        {
            { EShadowResolution::Original, "ORIGINAL" },
        }
    }
};

CONFIG_DEFINE_LOCALE(GITextureFiltering)
{
    { ELanguage::English, "GI Texture Filtering" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(GITextureFiltering)
{
    { ELanguage::English, "The method used for filtering baked lighting.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_ENUM_LOCALE(EGITextureFiltering)
{
    {
        ELanguage::English,
        {
            { EGITextureFiltering::Bilinear, "BILINEAR" },
            { EGITextureFiltering::Bicubic,  "BICUBIC" },
        }
    }
};

CONFIG_DEFINE_LOCALE(AlphaToCoverage)
{
    { ELanguage::English, "Transparency Anti-Aliasing" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(AlphaToCoverage)
{
    { ELanguage::English, "Determines whether to smooth out jagged edges on alpha transparent textures." }
};

CONFIG_DEFINE_LOCALE(MotionBlur)
{
    { ELanguage::English, "Motion Blur" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(MotionBlur)
{
    { ELanguage::English, "Determines whether to draw per-object motion blur and radial blur." }
};

CONFIG_DEFINE_LOCALE(Xbox360ColourCorrection)
{
    { ELanguage::English, "Xbox 360 Colour Correction" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Xbox360ColourCorrection)
{
    { ELanguage::English, "Determines whether to tint the overall appearance of the game with a more warm hue like the original Xbox 360 version." }
};

CONFIG_DEFINE_LOCALE(MovieScaleMode)
{
    { ELanguage::English, "Movie Scale Mode" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(MovieScaleMode)
{
    { ELanguage::English, "Determines how to scale the movie player.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_ENUM_LOCALE(EMovieScaleMode)
{
    {
        ELanguage::English,
        {
            { EMovieScaleMode::Stretch, "STRETCH" },
            { EMovieScaleMode::Fit,     "FIT" },
            { EMovieScaleMode::Fill,    "FILL" },
        }
    }
};

CONFIG_DEFINE_LOCALE(UIScaleMode)
{
    { ELanguage::English, "UI Scale Mode" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(UIScaleMode)
{
    { ELanguage::English, "Determines how to scale the UI.\n\n[PLACEHOLDER]" }
};

CONFIG_DEFINE_ENUM_LOCALE(EUIScaleMode)
{
    {
        ELanguage::English,
        {
            { EUIScaleMode::Stretch, "STRETCH" },
            { EUIScaleMode::Edge,    "EDGE" },
            { EUIScaleMode::Centre,  "CENTRE" },
        }
    }
};
