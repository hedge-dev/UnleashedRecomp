#pragma once

#include "config_detail.h"

class Config
{
public:
    inline static std::vector<ConfigDefBase*> Definitions{};

    CONFIG_DEFINE_ENUM("System", ELanguage, Language, ELanguage::English);
    CONFIG_DEFINE("System", bool, Hints, true);
    CONFIG_DEFINE("System", bool, ControlTutorial, true);
    CONFIG_DEFINE_ENUM("System", EScoreBehaviour, ScoreBehaviour, EScoreBehaviour::CheckpointReset);
    CONFIG_DEFINE("System", bool, UnleashOutOfControlDrain, true);
    CONFIG_DEFINE("System", bool, WerehogHubTransformVideo, true);
    CONFIG_DEFINE_HIDE("System", bool, LogoSkip, false);

    CONFIG_DEFINE("Controls", bool, CameraXInvert, false);
    CONFIG_DEFINE("Controls", bool, CameraYInvert, false);
    CONFIG_DEFINE("Controls", bool, XButtonHoming, true);
    CONFIG_DEFINE("Controls", bool, UnleashCancel, false);

    CONFIG_DEFINE("Audio", float, MusicVolume, 1.0f);
    CONFIG_DEFINE("Audio", float, SEVolume, 1.0f);
    CONFIG_DEFINE_ENUM("Audio", EVoiceLanguage, VoiceLanguage, EVoiceLanguage::English);
    CONFIG_DEFINE("Audio", bool, Subtitles, true);
    CONFIG_DEFINE("Audio", bool, WerehogBattleMusic, true);

    CONFIG_DEFINE_ENUM("Video", EGraphicsAPI, GraphicsAPI, EGraphicsAPI::D3D12);
    CONFIG_DEFINE_HIDE("Video", int32_t, WindowX, WINDOWPOS_CENTRED);
    CONFIG_DEFINE_HIDE("Video", int32_t, WindowY, WINDOWPOS_CENTRED);
    CONFIG_DEFINE("Video", int32_t, WindowWidth, 1280);
    CONFIG_DEFINE("Video", int32_t, WindowHeight, 720);
    CONFIG_DEFINE_ENUM_HIDE("Video", EWindowState, WindowState, EWindowState::Normal);

    CONFIG_DEFINE_CALLBACK("Video", float, ResolutionScale, 1.0f,
    {
        def->Value = std::clamp(def->Value, 0.25f, 2.0f);
    });

    CONFIG_DEFINE("Video", bool, Fullscreen, false);
    CONFIG_DEFINE("Video", bool, VSync, true);
    CONFIG_DEFINE("Video", bool, TripleBuffering, true);
    CONFIG_DEFINE("Video", int32_t, FPS, 60);
    CONFIG_DEFINE("Video", float, Brightness, 0.5f);
    CONFIG_DEFINE("Video", size_t, MSAA, 4);
    CONFIG_DEFINE_HIDE("Video", size_t, AnisotropicFiltering, 16);
    CONFIG_DEFINE_ENUM("Video", EShadowResolution, ShadowResolution, EShadowResolution::x4096);
    CONFIG_DEFINE_ENUM("Video", EGITextureFiltering, GITextureFiltering, EGITextureFiltering::Bicubic);
    CONFIG_DEFINE("Video", bool, AlphaToCoverage, true);
    CONFIG_DEFINE("Video", bool, MotionBlur, true);
    CONFIG_DEFINE("Video", bool, Xbox360ColourCorrection, false);
    CONFIG_DEFINE_ENUM("Video", EMovieScaleMode, MovieScaleMode, EMovieScaleMode::Fit);
    CONFIG_DEFINE_ENUM("Video", EUIScaleMode, UIScaleMode, EUIScaleMode::Centre);

    static std::filesystem::path GetUserPath()
    {
        if (std::filesystem::exists("portable.txt"))
            return std::filesystem::current_path();

        std::filesystem::path userPath{};

        // TODO: handle platform-specific paths.
        PWSTR knownPath = NULL;
        if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &knownPath) == S_OK)
            userPath = std::filesystem::path{ knownPath } / USER_DIRECTORY;

        CoTaskMemFree(knownPath);

        return userPath;
    }

    static std::filesystem::path GetConfigPath()
    {
        return GetUserPath() / TOML_FILE;
    }

    static std::filesystem::path GetSavePath()
    {
        return GetUserPath() / "save";
    }

    static void Load();
    static void Save();
};
