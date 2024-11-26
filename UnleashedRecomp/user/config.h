#pragma once

#include <user/config_detail.h>
#include <locale/config_locale.h>
#include <user/paths.h>
#include <exports.h>

class Config
{
public:
    inline static std::vector<IConfigDef*> Definitions{};

    CONFIG_DEFINE_ENUM_LOCALISED("System", ELanguage, Language, ELanguage::English);
    CONFIG_DEFINE_LOCALISED("System", bool, Hints, true);
    CONFIG_DEFINE_LOCALISED("System", bool, ControlTutorial, true);
    CONFIG_DEFINE_LOCALISED("System", bool, AchievementNotifications, true);
    CONFIG_DEFINE_LOCALISED("System", bool, SaveScoreAtCheckpoints, false);
    CONFIG_DEFINE_ENUM_LOCALISED("System", EUnleashGaugeBehaviour, UnleashGaugeBehaviour, EUnleashGaugeBehaviour::Original);
    CONFIG_DEFINE_ENUM_LOCALISED("System", ETimeOfDayTransition, TimeOfDayTransition, ETimeOfDayTransition::Xbox);
    CONFIG_DEFINE_LOCALISED("System", bool, SkipIntroLogos, false);

    CONFIG_DEFINE_LOCALISED("Input", bool, InvertCameraX, false);
    CONFIG_DEFINE_LOCALISED("Input", bool, InvertCameraY, false);
    CONFIG_DEFINE_LOCALISED("Input", bool, XButtonHoming, true);
    CONFIG_DEFINE_LOCALISED("Input", bool, AllowCancellingUnleash, false);
    CONFIG_DEFINE_LOCALISED("Input", bool, AllowBackgroundInput, false);

    CONFIG_DEFINE_LOCALISED("Audio", float, MusicVolume, 1.0f);
    CONFIG_DEFINE_LOCALISED("Audio", float, EffectsVolume, 1.0f);
    CONFIG_DEFINE_ENUM_LOCALISED("Audio", EVoiceLanguage, VoiceLanguage, EVoiceLanguage::English);
    CONFIG_DEFINE_LOCALISED("Audio", bool, Subtitles, true);
    CONFIG_DEFINE_LOCALISED("Audio", bool, MusicAttenuation, false);
    CONFIG_DEFINE_LOCALISED("Audio", bool, BattleTheme, true);

    CONFIG_DEFINE_ENUM("Video", EGraphicsAPI, GraphicsAPI, EGraphicsAPI::D3D12);
    CONFIG_DEFINE("Video", int32_t, WindowX, WINDOWPOS_CENTRED);
    CONFIG_DEFINE("Video", int32_t, WindowY, WINDOWPOS_CENTRED);
    CONFIG_DEFINE("Video", int32_t, WindowWidth, 1280);
    CONFIG_DEFINE("Video", int32_t, WindowHeight, 720);
    CONFIG_DEFINE_ENUM("Video", EWindowState, WindowState, EWindowState::Normal);
    CONFIG_DEFINE_ENUM_LOCALISED("Video", EAspectRatio, AspectRatio, EAspectRatio::Auto);

    CONFIG_DEFINE_CALLBACK("Video", float, ResolutionScale, 1.0f,
    {
        def->Locale = &g_ResolutionScale_locale;
        def->Value = std::clamp(def->Value, 0.25f, 2.0f);
    });

    CONFIG_DEFINE_CALLBACK("Video", bool, Fullscreen, false,
    {
        def->Locale = &g_Fullscreen_locale;

        Window_SetFullscreen(def->Value);
    });

    CONFIG_DEFINE_LOCALISED("Video", bool, VSync, true);
    CONFIG_DEFINE_ENUM("Video", ETripleBuffering, TripleBuffering, ETripleBuffering::Auto);
    CONFIG_DEFINE_LOCALISED("Video", int32_t, FPS, 60);
    CONFIG_DEFINE_LOCALISED("Video", float, Brightness, 0.5f);
    CONFIG_DEFINE_ENUM_LOCALISED("Video", EAntiAliasing, AntiAliasing, EAntiAliasing::MSAA4x);
    CONFIG_DEFINE_LOCALISED("Video", bool, TransparencyAntiAliasing, true);
    CONFIG_DEFINE("Video", size_t, AnisotropicFiltering, 16);
    CONFIG_DEFINE_ENUM_LOCALISED("Video", EShadowResolution, ShadowResolution, EShadowResolution::x4096);
    CONFIG_DEFINE_ENUM_LOCALISED("Video", EGITextureFiltering, GITextureFiltering, EGITextureFiltering::Bicubic);
    CONFIG_DEFINE_LOCALISED("Video", bool, MotionBlur, true);
    CONFIG_DEFINE_LOCALISED("Video", bool, XboxColourCorrection, false);
    CONFIG_DEFINE_ENUM_LOCALISED("Video", EMovieScaleMode, MovieScaleMode, EMovieScaleMode::Fit);
    CONFIG_DEFINE_ENUM_LOCALISED("Video", EUIScaleMode, UIScaleMode, EUIScaleMode::Centre);

    static std::filesystem::path GetConfigPath()
    {
        return GetUserPath() / "config.toml";
    }

    static void Load();
    static void Save();
};
