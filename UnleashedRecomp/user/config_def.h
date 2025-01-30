// This file gets included in both config.h and config.cpp, with their own macros changing
// the preprocessed output. The header is only going to have the declarations this way.

CONFIG_DEFINE_ENUM_LOCALISED("System", ELanguage, Language, ELanguage::English);
CONFIG_DEFINE_ENUM_LOCALISED("System", EVoiceLanguage, VoiceLanguage, EVoiceLanguage::English);
CONFIG_DEFINE_LOCALISED("System", bool, Subtitles, true);
CONFIG_DEFINE_LOCALISED("System", bool, Hints, true);
CONFIG_DEFINE_LOCALISED("System", bool, ControlTutorial, true);
CONFIG_DEFINE_LOCALISED("System", bool, AchievementNotifications, true);
CONFIG_DEFINE_ENUM_LOCALISED("System", ETimeOfDayTransition, TimeOfDayTransition, ETimeOfDayTransition::Xbox);
CONFIG_DEFINE("System", bool, ShowConsole, false);

CONFIG_DEFINE_LOCALISED("Input", bool, InvertCameraX, false);
CONFIG_DEFINE_LOCALISED("Input", bool, InvertCameraY, false);
CONFIG_DEFINE_LOCALISED("Input", bool, Vibration, true);
CONFIG_DEFINE_LOCALISED("Input", bool, AllowBackgroundInput, false);
CONFIG_DEFINE_ENUM_LOCALISED("Input", EControllerIcons, ControllerIcons, EControllerIcons::Auto);

CONFIG_DEFINE_LOCALISED("Audio", float, MasterVolume, 1.0f);
CONFIG_DEFINE_LOCALISED("Audio", float, MusicVolume, 1.0f);
CONFIG_DEFINE_LOCALISED("Audio", float, EffectsVolume, 1.0f);
CONFIG_DEFINE_ENUM_LOCALISED("Audio", EChannelConfiguration, ChannelConfiguration, EChannelConfiguration::Stereo);
CONFIG_DEFINE_LOCALISED("Audio", bool, MusicAttenuation, false);
CONFIG_DEFINE_LOCALISED("Audio", bool, BattleTheme, true);

#ifdef UNLEASHED_RECOMP_D3D12
CONFIG_DEFINE_ENUM("Video", EGraphicsAPI, GraphicsAPI, EGraphicsAPI::D3D12);
#else
CONFIG_DEFINE_ENUM("Video", EGraphicsAPI, GraphicsAPI, EGraphicsAPI::Vulkan);
#endif

CONFIG_DEFINE("Video", int32_t, WindowX, WINDOWPOS_CENTRED);
CONFIG_DEFINE("Video", int32_t, WindowY, WINDOWPOS_CENTRED);
CONFIG_DEFINE_LOCALISED("Video", int32_t, WindowSize, -1);
CONFIG_DEFINE("Video", int32_t, WindowWidth, 1280);
CONFIG_DEFINE("Video", int32_t, WindowHeight, 720);
CONFIG_DEFINE_ENUM("Video", EWindowState, WindowState, EWindowState::Normal);

CONFIG_DEFINE_CALLBACK("Video", int32_t, Monitor, 0,
{
    def->Locale = &g_Monitor_locale;

    Window_SetDisplay(def->Value);
});

CONFIG_DEFINE_ENUM_LOCALISED("Video", EAspectRatio, AspectRatio, EAspectRatio::Auto);

CONFIG_DEFINE_CALLBACK("Video", float, ResolutionScale, 1.0f,
{
    def->Locale = &g_ResolutionScale_locale;
    def->Value = std::clamp(def->Value, 0.25f, 2.0f);
});

CONFIG_DEFINE_CALLBACK("Video", bool, Fullscreen, true,
{
    def->Locale = &g_Fullscreen_locale;

    Window_SetFullscreen(def->Value);
    Window_SetDisplay(Monitor);
});

CONFIG_DEFINE_LOCALISED("Video", bool, VSync, true);
CONFIG_DEFINE_ENUM("Video", ETripleBuffering, TripleBuffering, ETripleBuffering::Auto);
CONFIG_DEFINE_LOCALISED("Video", int32_t, FPS, 60);
CONFIG_DEFINE("Video", uint32_t, MaxFrameLatency, 2);
CONFIG_DEFINE_LOCALISED("Video", float, Brightness, 0.5f);
CONFIG_DEFINE_ENUM_LOCALISED("Video", EAntiAliasing, AntiAliasing, EAntiAliasing::MSAA4x);
CONFIG_DEFINE_LOCALISED("Video", bool, TransparencyAntiAliasing, true);
CONFIG_DEFINE("Video", uint32_t, AnisotropicFiltering, 16);
CONFIG_DEFINE_ENUM_LOCALISED("Video", EShadowResolution, ShadowResolution, EShadowResolution::x4096);
CONFIG_DEFINE_ENUM_LOCALISED("Video", EGITextureFiltering, GITextureFiltering, EGITextureFiltering::Bicubic);
CONFIG_DEFINE_ENUM("Video", EDepthOfFieldQuality, DepthOfFieldQuality, EDepthOfFieldQuality::Auto);
CONFIG_DEFINE_ENUM_LOCALISED("Video", EMotionBlur, MotionBlur, EMotionBlur::Original);
CONFIG_DEFINE_LOCALISED("Video", bool, XboxColorCorrection, false);
CONFIG_DEFINE_ENUM_LOCALISED("Video", ECutsceneAspectRatio, CutsceneAspectRatio, ECutsceneAspectRatio::Original);
CONFIG_DEFINE_ENUM_LOCALISED("Video", EUIScaleMode, UIScaleMode, EUIScaleMode::Edge);

CONFIG_DEFINE_HIDDEN("Exports", bool, AllowCancellingUnleash, false);
CONFIG_DEFINE_HIDDEN("Exports", bool, DisableAutoSaveWarning, false);
CONFIG_DEFINE_HIDDEN("Exports", bool, DisableDLCIcon, false);
CONFIG_DEFINE_HIDDEN("Exports", bool, DisableDWMRoundedCorners, false);
CONFIG_DEFINE_HIDDEN("Exports", bool, FixUnleashOutOfControlDrain, false);
CONFIG_DEFINE_HIDDEN("Exports", bool, HomingAttackOnBoost, true);
CONFIG_DEFINE_HIDDEN("Exports", bool, SaveScoreAtCheckpoints, false);
CONFIG_DEFINE_HIDDEN("Exports", bool, SkipIntroLogos, false);
CONFIG_DEFINE_HIDDEN("Exports", bool, UseOfficialTitleOnTitleBar, false);
