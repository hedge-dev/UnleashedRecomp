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
    { ELanguage::English, "Change the language used for text and logos." }
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
            { ELanguage::Italian,  "ITALIANO" }
        }
    }
};

CONFIG_DEFINE_LOCALE(Hints)
{
    { ELanguage::English, "Hints" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Hints)
{
    { ELanguage::English, "Show hint rings in stages." }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English, "Control Tutorial" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(ControlTutorial)
{
    { ELanguage::English, "Show controller hints in stages." }
};

CONFIG_DEFINE_LOCALE(SaveScoreAtCheckpoints)
{
    { ELanguage::English, "Save Score at Checkpoints" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(SaveScoreAtCheckpoints)
{
    { ELanguage::English, "Keep your score from the last checkpoint upon respawning." }
};

CONFIG_DEFINE_LOCALE(UnleashGaugeBehaviour)
{
    { ELanguage::English, "Unleash Gauge Behavior" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(UnleashGaugeBehaviour)
{
    { ELanguage::English, "Change how the Unleash gauge behaves." }
};

CONFIG_DEFINE_ENUM_LOCALE(EUnleashGaugeBehaviour)
{
    {
        ELanguage::English,
        {
            { EUnleashGaugeBehaviour::Original, "ORIGINAL" },
            { EUnleashGaugeBehaviour::Revised,  "REVISED" }
        }
    }
};

CONFIG_DEFINE_LOCALE(WerehogHubTransformVideo)
{
    { ELanguage::English, "Transformation Cutscene" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(WerehogHubTransformVideo)
{
    { ELanguage::English, "Play the Werehog transformation cutscene when changing the time of day in the hub areas." }
};

CONFIG_DEFINE_LOCALE(LogoSkip)
{
    { ELanguage::English, "Skip Intro Logos" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(LogoSkip)
{
    { ELanguage::English, "Show the logos during the game's boot sequence." }
};

CONFIG_DEFINE_LOCALE(CameraXInvert)
{
    { ELanguage::English, "Invert Camera X" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(CameraXInvert)
{
    { ELanguage::English, "Toggle between inverted left and right camera movement." }
};

CONFIG_DEFINE_LOCALE(CameraYInvert)
{
    { ELanguage::English, "Invert Camera Y" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(CameraYInvert)
{
    { ELanguage::English, "Toggle between inverted up and down camera movement." }
};

CONFIG_DEFINE_LOCALE(XButtonHoming)
{
    { ELanguage::English, "Homing Attack on Boost" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(XButtonHoming)
{
    { ELanguage::English, "Toggle between using the boost button or the jump button for homing attack." }
};

CONFIG_DEFINE_LOCALE(UnleashCancel)
{
    { ELanguage::English, "Allow Cancelling Unleash" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(UnleashCancel)
{
    { ELanguage::English, "Allow Unleash to be cancelled at the cost of some energy by pressing the input again." }
};

CONFIG_DEFINE_LOCALE(BackgroundInput)
{
    { ELanguage::English, "Allow Background Input" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(BackgroundInput)
{
    { ELanguage::English, "Accept controller input whilst the game window is unfocused." }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English, "Music Volume" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(MusicVolume)
{
    { ELanguage::English, "Adjust the volume for the music." }
};

CONFIG_DEFINE_LOCALE(SEVolume)
{
    { ELanguage::English, "SE Volume" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(SEVolume)
{
    { ELanguage::English, "Adjust the volume for sound effects." }
};

CONFIG_DEFINE_LOCALE(MusicAttenuation)
{
    { ELanguage::English, "Music Attenuation" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(MusicAttenuation)
{
    { ELanguage::English, "Fade out the game's music when external media is playing.\n\nRequires Windows 10 or later." }
};

CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English, "Voice Language" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(VoiceLanguage)
{
    { ELanguage::English, "Change the language used for character voices." }
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
    { ELanguage::English, "Show subtitles during dialogue." }
};

CONFIG_DEFINE_LOCALE(WerehogBattleMusic)
{
    { ELanguage::English, "Battle Theme" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(WerehogBattleMusic)
{
    { ELanguage::English, "Play the Werehog battle theme during combat.\n\nThis option will apply the next time you're in combat." }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English, "Window Size" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(WindowSize)
{
    { ELanguage::English, "Adjust the size of the game window in windowed mode." }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English, "Resolution Scale" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(ResolutionScale)
{
    { ELanguage::English, "Adjust the internal resolution of the game.\n\n%dx%d" }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English, "Fullscreen" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Fullscreen)
{
    { ELanguage::English, "Toggle between borderless fullscreen or windowed mode." }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English, "V-Sync" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(VSync)
{
    { ELanguage::English, "[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(TripleBuffering)
{
    { ELanguage::English, "Triple Buffering" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(TripleBuffering)
{
    { ELanguage::English, "[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English, "FPS" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(FPS)
{
    { ELanguage::English, "[PLACEHOLDER]" }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English, "Brightness" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Brightness)
{
    { ELanguage::English, "Adjust the brightness level of the game." }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English, "Anti-Aliasing" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(AntiAliasing)
{
    { ELanguage::English, "Adjust the amount of smoothing applied to jagged edges." }
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

CONFIG_DEFINE_LOCALE(AlphaToCoverage)
{
    { ELanguage::English, "Transparency Anti-Aliasing" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(AlphaToCoverage)
{
    { ELanguage::English, "Apply anti-aliasing to alpha transparent textures." }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English, "Shadow Resolution" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(ShadowResolution)
{
    { ELanguage::English, "[PLACEHOLDER]" }
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
    { ELanguage::English, "[PLACEHOLDER]" }
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

CONFIG_DEFINE_LOCALE(MotionBlur)
{
    { ELanguage::English, "Motion Blur" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(MotionBlur)
{
    { ELanguage::English, "Use per-object motion blur and radial blur." }
};

CONFIG_DEFINE_LOCALE(Xbox360ColourCorrection)
{
    { ELanguage::English, "Xbox 360 Color Correction" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(Xbox360ColourCorrection)
{
    { ELanguage::English, "Use the warm tint from the Xbox 360 version of the game." }
};

CONFIG_DEFINE_LOCALE(MovieScaleMode)
{
    { ELanguage::English, "Movie Scale Mode" }
};

CONFIG_DEFINE_DESCRIPTION_LOCALE(MovieScaleMode)
{
    { ELanguage::English, "Change how the movie player scales to the display." }
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
    { ELanguage::English, "Change how the UI scales to the display." }
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
