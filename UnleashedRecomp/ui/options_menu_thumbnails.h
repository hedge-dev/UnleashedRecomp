#pragma once

#include <gpu/video.h>
#include <user/config.h>
#include <decompressor.h>

#include <res/images/options_menu/thumbnails/achievement_notifications.dds.h>
#include <res/images/options_menu/thumbnails/allow_background_input.dds.h>
#include <res/images/options_menu/thumbnails/antialiasing.dds.h>
#include <res/images/options_menu/thumbnails/aspect_ratio.dds.h>
#include <res/images/options_menu/thumbnails/battle_theme.dds.h>
#include <res/images/options_menu/thumbnails/brightness.dds.h>
#include <res/images/options_menu/thumbnails/control_tutorial.dds.h>
#include <res/images/options_menu/thumbnails/controller_icons.dds.h>
#include <res/images/options_menu/thumbnails/effects_volume.dds.h>
#include <res/images/options_menu/thumbnails/fps.dds.h>
#include <res/images/options_menu/thumbnails/fullscreen.dds.h>
#include <res/images/options_menu/thumbnails/gi_texture_filtering.dds.h>
#include <res/images/options_menu/thumbnails/hints.dds.h>
#include <res/images/options_menu/thumbnails/invert_camera_x.dds.h>
#include <res/images/options_menu/thumbnails/invert_camera_y.dds.h>
#include <res/images/options_menu/thumbnails/language.dds.h>
#include <res/images/options_menu/thumbnails/monitor.dds.h>
#include <res/images/options_menu/thumbnails/motion_blur.dds.h>
#include <res/images/options_menu/thumbnails/movie_scale_mode.dds.h>
#include <res/images/options_menu/thumbnails/music_attenuation.dds.h>
#include <res/images/options_menu/thumbnails/music_volume.dds.h>
#include <res/images/options_menu/thumbnails/resolution_scale.dds.h>
#include <res/images/options_menu/thumbnails/shadow_resolution.dds.h>
#include <res/images/options_menu/thumbnails/subtitles.dds.h>
#include <res/images/options_menu/thumbnails/time_of_day_transition.dds.h>
#include <res/images/options_menu/thumbnails/transparency_antialiasing.dds.h>
#include <res/images/options_menu/thumbnails/ui_scale_mode.dds.h>
#include <res/images/options_menu/thumbnails/voice_language.dds.h>
#include <res/images/options_menu/thumbnails/vsync.dds.h>
#include <res/images/options_menu/thumbnails/window_size.dds.h>
#include <res/images/options_menu/thumbnails/xbox_color_correction.dds.h>

inline static std::unordered_map<std::string_view, std::unique_ptr<GuestTexture>> g_thumbnails;

static void LoadThumbnails()
{
    g_thumbnails[Config::Language.Name] = LOAD_ZSTD_TEXTURE(g_language);
    g_thumbnails[Config::Hints.Name] = LOAD_ZSTD_TEXTURE(g_hints);
    g_thumbnails[Config::ControlTutorial.Name] = LOAD_ZSTD_TEXTURE(g_control_tutorial);
    g_thumbnails[Config::AchievementNotifications.Name] = LOAD_ZSTD_TEXTURE(g_achievement_notifications);
    g_thumbnails[Config::TimeOfDayTransition.Name] = LOAD_ZSTD_TEXTURE(g_time_of_day_transition);
    g_thumbnails[Config::InvertCameraX.Name] = LOAD_ZSTD_TEXTURE(g_invert_camera_x);
    g_thumbnails[Config::InvertCameraY.Name] = LOAD_ZSTD_TEXTURE(g_invert_camera_y);
    g_thumbnails[Config::AllowBackgroundInput.Name] = LOAD_ZSTD_TEXTURE(g_allow_background_input);
    g_thumbnails[Config::ControllerIcons.Name] = LOAD_ZSTD_TEXTURE(g_controller_icons);
    g_thumbnails[Config::MusicVolume.Name] = LOAD_ZSTD_TEXTURE(g_music_volume);
    g_thumbnails[Config::EffectsVolume.Name] = LOAD_ZSTD_TEXTURE(g_effects_volume);
    g_thumbnails[Config::VoiceLanguage.Name] = LOAD_ZSTD_TEXTURE(g_voice_language);
    g_thumbnails[Config::Subtitles.Name] = LOAD_ZSTD_TEXTURE(g_subtitles);
    g_thumbnails[Config::MusicAttenuation.Name] = LOAD_ZSTD_TEXTURE(g_music_attenuation);
    g_thumbnails[Config::BattleTheme.Name] = LOAD_ZSTD_TEXTURE(g_battle_theme);
    g_thumbnails["WindowSize"] = LOAD_ZSTD_TEXTURE(g_window_size);
    g_thumbnails["Monitor"] = LOAD_ZSTD_TEXTURE(g_monitor);
    g_thumbnails[Config::AspectRatio.Name] = LOAD_ZSTD_TEXTURE(g_aspect_ratio);
    g_thumbnails[Config::ResolutionScale.Name] = LOAD_ZSTD_TEXTURE(g_resolution_scale);
    g_thumbnails[Config::Fullscreen.Name] = LOAD_ZSTD_TEXTURE(g_fullscreen);
    g_thumbnails[Config::VSync.Name] = LOAD_ZSTD_TEXTURE(g_vsync);
    g_thumbnails[Config::FPS.Name] = LOAD_ZSTD_TEXTURE(g_fps);
    g_thumbnails[Config::Brightness.Name] = LOAD_ZSTD_TEXTURE(g_brightness);
    g_thumbnails[Config::AntiAliasing.Name] = LOAD_ZSTD_TEXTURE(g_antialiasing);
    g_thumbnails[Config::TransparencyAntiAliasing.Name] = LOAD_ZSTD_TEXTURE(g_transparency_antialiasing);
    g_thumbnails[Config::ShadowResolution.Name] = LOAD_ZSTD_TEXTURE(g_shadow_resolution);
    g_thumbnails[Config::GITextureFiltering.Name] = LOAD_ZSTD_TEXTURE(g_gi_texture_filtering);
    g_thumbnails[Config::MotionBlur.Name] = LOAD_ZSTD_TEXTURE(g_motion_blur);
    g_thumbnails[Config::XboxColourCorrection.Name] = LOAD_ZSTD_TEXTURE(g_xbox_color_correction);
    g_thumbnails[Config::MovieScaleMode.Name] = LOAD_ZSTD_TEXTURE(g_movie_scale_mode);
    g_thumbnails[Config::UIScaleMode.Name] = LOAD_ZSTD_TEXTURE(g_ui_scale_mode);
}

static GuestTexture* GetThumbnail(const std::string_view name)
{
    if (!g_thumbnails.count(name))
        return nullptr;

    return g_thumbnails[name].get();
}
