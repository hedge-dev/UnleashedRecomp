#pragma once

#include "config_detail.h"

#define CONFIG_DEFINE_LOCALE(name) \
    inline static std::unordered_map<ELanguage, std::string> g_##name##_locale =

#define CONFIG_DEFINE_ENUM_LOCALE(type) \
    inline static std::unordered_map<ELanguage, std::unordered_map<type, std::string>> g_##type##_locale =

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
    { ELanguage::Japanese, "言語"      },
    { ELanguage::German,   "Sprache"  },
    { ELanguage::French,   "Langue"   },
    { ELanguage::Spanish,  "Idioma"   },
    { ELanguage::Italian,  "Lingua"   }
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

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English, "Control Tutorial" }
};

CONFIG_DEFINE_LOCALE(SaveScoreAtCheckpoints)
{
    { ELanguage::English, "Save Score at Checkpoints" }
};

CONFIG_DEFINE_LOCALE(UnleashOutOfControlDrain)
{
    { ELanguage::English, "Decrease Unleash Gauge whilst Out Of Control" }
};

CONFIG_DEFINE_LOCALE(WerehogHubTransformVideo)
{
    { ELanguage::English, "Werehog Transformation Video" }
};

CONFIG_DEFINE_LOCALE(LogoSkip)
{
    { ELanguage::English, "Skip Intro Logos" }
};

CONFIG_DEFINE_LOCALE(CameraXInvert)
{
    { ELanguage::English, "Invert Camera X" }
};

CONFIG_DEFINE_LOCALE(CameraYInvert)
{
    { ELanguage::English, "Invert Camera Y" }
};

CONFIG_DEFINE_LOCALE(XButtonHoming)
{
    { ELanguage::English, "Homing Attack on Boost" }
};

CONFIG_DEFINE_LOCALE(UnleashCancel)
{
    { ELanguage::English, "Allow Cancelling Unleash" }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English, "Music Volume" }
};

CONFIG_DEFINE_LOCALE(SEVolume)
{
    { ELanguage::English, "SE Volume" }
};

CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English, "Voice Language" }
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

CONFIG_DEFINE_LOCALE(WerehogBattleMusic)
{
    { ELanguage::English, "Werehog Battle Theme" }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English, "Window Size" }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English, "Resolution Scale" }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English, "Fullscreen" }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English, "V-Sync" }
};

CONFIG_DEFINE_LOCALE(TripleBuffering)
{
    { ELanguage::English, "Triple Buffering" }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English, "Brightness" }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English, "Anti-Aliasing" }
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

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English, "Shadow Resolution" }
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

CONFIG_DEFINE_LOCALE(MotionBlur)
{
    { ELanguage::English, "Motion Blur" }
};

CONFIG_DEFINE_LOCALE(Xbox360ColourCorrection)
{
    { ELanguage::English, "Xbox 360 Colour Correction" }
};

CONFIG_DEFINE_LOCALE(MovieScaleMode)
{
    { ELanguage::English, "Movie Scale Mode" }
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
