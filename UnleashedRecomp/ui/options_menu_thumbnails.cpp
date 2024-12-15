#include <ui/options_menu_thumbnails.h>
#include <decompressor.h>

// TODO (Hyper): lower the resolution of these textures once final.
#include <res/images/options_menu/thumbnails/achievement_notifications.dds.h>
#include <res/images/options_menu/thumbnails/allow_background_input.dds.h>
#include <res/images/options_menu/thumbnails/allow_dpad_movement.dds.h>
#include <res/images/options_menu/thumbnails/antialiasing.dds.h>
#include <res/images/options_menu/thumbnails/aspect_ratio.dds.h>
#include <res/images/options_menu/thumbnails/battle_theme.dds.h>
#include <res/images/options_menu/thumbnails/brightness.dds.h>
#include <res/images/options_menu/thumbnails/control_tutorial.dds.h>
#include <res/images/options_menu/thumbnails/controller_icons.dds.h>
#include <res/images/options_menu/thumbnails/default.dds.h>
#include <res/images/options_menu/thumbnails/effects_volume.dds.h>
#include <res/images/options_menu/thumbnails/fps.dds.h>
#include <res/images/options_menu/thumbnails/fullscreen.dds.h>
#include <res/images/options_menu/thumbnails/gi_texture_filtering_bilinear.dds.h>
#include <res/images/options_menu/thumbnails/gi_texture_filtering_bicubic.dds.h>
#include <res/images/options_menu/thumbnails/hints.dds.h>
#include <res/images/options_menu/thumbnails/invert_camera_x.dds.h>
#include <res/images/options_menu/thumbnails/invert_camera_y.dds.h>
#include <res/images/options_menu/thumbnails/language.dds.h>
#include <res/images/options_menu/thumbnails/monitor.dds.h>
#include <res/images/options_menu/thumbnails/motion_blur_off.dds.h>
#include <res/images/options_menu/thumbnails/motion_blur_original.dds.h>
#include <res/images/options_menu/thumbnails/motion_blur_enhanced.dds.h>
#include <res/images/options_menu/thumbnails/movie_scale_mode.dds.h>
#include <res/images/options_menu/thumbnails/music_attenuation.dds.h>
#include <res/images/options_menu/thumbnails/music_volume.dds.h>
#include <res/images/options_menu/thumbnails/resolution_scale.dds.h>
#include <res/images/options_menu/thumbnails/shadow_resolution_original.dds.h>
#include <res/images/options_menu/thumbnails/shadow_resolution_x512.dds.h>
#include <res/images/options_menu/thumbnails/shadow_resolution_x1024.dds.h>
#include <res/images/options_menu/thumbnails/shadow_resolution_x2048.dds.h>
#include <res/images/options_menu/thumbnails/shadow_resolution_x4096.dds.h>
#include <res/images/options_menu/thumbnails/shadow_resolution_x8192.dds.h>
#include <res/images/options_menu/thumbnails/subtitles.dds.h>
#include <res/images/options_menu/thumbnails/time_of_day_transition_xbox.dds.h>
#include <res/images/options_menu/thumbnails/time_of_day_transition_playstation.dds.h>
#include <res/images/options_menu/thumbnails/transparency_antialiasing_false.dds.h>
#include <res/images/options_menu/thumbnails/transparency_antialiasing_true.dds.h>
#include <res/images/options_menu/thumbnails/ui_scale_mode.dds.h>
#include <res/images/options_menu/thumbnails/voice_language.dds.h>
#include <res/images/options_menu/thumbnails/vsync.dds.h>
#include <res/images/options_menu/thumbnails/window_size.dds.h>
#include <res/images/options_menu/thumbnails/xbox_color_correction_false.dds.h>
#include <res/images/options_menu/thumbnails/xbox_color_correction_true.dds.h>

#define VALUE_THUMBNAIL_MAP(type) std::unordered_map<type, std::unique_ptr<GuestTexture>>

static std::unordered_map<std::string_view, std::unique_ptr<GuestTexture>> g_namedThumbnails;
static std::unordered_map<const IConfigDef*, std::unique_ptr<GuestTexture>> g_configThumbnails;

static VALUE_THUMBNAIL_MAP(ETimeOfDayTransition) g_timeOfDayTransitionThumbnails;
static VALUE_THUMBNAIL_MAP(bool) g_transparencyAntiAliasingThumbnails;
static VALUE_THUMBNAIL_MAP(EShadowResolution) g_shadowResolutionThumbnails;
static VALUE_THUMBNAIL_MAP(EGITextureFiltering) g_giTextureFilteringThumbnails;
static VALUE_THUMBNAIL_MAP(EMotionBlur) g_motionBlurThumbnails;
static VALUE_THUMBNAIL_MAP(bool) g_xboxColorCorrectionThumbnails;

void LoadThumbnails()
{
    g_namedThumbnails["Default"] = LOAD_ZSTD_TEXTURE(g_default);
    g_namedThumbnails["WindowSize"] = LOAD_ZSTD_TEXTURE(g_window_size);

    g_configThumbnails[&Config::Language] = LOAD_ZSTD_TEXTURE(g_language);
    g_configThumbnails[&Config::Hints] = LOAD_ZSTD_TEXTURE(g_hints);
    g_configThumbnails[&Config::ControlTutorial] = LOAD_ZSTD_TEXTURE(g_control_tutorial);
    g_configThumbnails[&Config::AchievementNotifications] = LOAD_ZSTD_TEXTURE(g_achievement_notifications);

    g_timeOfDayTransitionThumbnails[ETimeOfDayTransition::Xbox] = LOAD_ZSTD_TEXTURE(g_time_of_day_transition_xbox);
    g_timeOfDayTransitionThumbnails[ETimeOfDayTransition::PlayStation] = LOAD_ZSTD_TEXTURE(g_time_of_day_transition_playstation);

    g_configThumbnails[&Config::InvertCameraX] = LOAD_ZSTD_TEXTURE(g_invert_camera_x);
    g_configThumbnails[&Config::InvertCameraY] = LOAD_ZSTD_TEXTURE(g_invert_camera_y);
    g_configThumbnails[&Config::AllowBackgroundInput] = LOAD_ZSTD_TEXTURE(g_allow_background_input);
    g_configThumbnails[&Config::AllowDPadMovement] = LOAD_ZSTD_TEXTURE(g_allow_dpad_movement);
    g_configThumbnails[&Config::ControllerIcons] = LOAD_ZSTD_TEXTURE(g_controller_icons);
    g_configThumbnails[&Config::MusicVolume] = LOAD_ZSTD_TEXTURE(g_music_volume);
    g_configThumbnails[&Config::EffectsVolume] = LOAD_ZSTD_TEXTURE(g_effects_volume);
    g_configThumbnails[&Config::VoiceLanguage] = LOAD_ZSTD_TEXTURE(g_voice_language);
    g_configThumbnails[&Config::Subtitles] = LOAD_ZSTD_TEXTURE(g_subtitles);
    g_configThumbnails[&Config::MusicAttenuation] = LOAD_ZSTD_TEXTURE(g_music_attenuation);
    g_configThumbnails[&Config::BattleTheme] = LOAD_ZSTD_TEXTURE(g_battle_theme);
    g_configThumbnails[&Config::Monitor] = LOAD_ZSTD_TEXTURE(g_monitor);
    g_configThumbnails[&Config::AspectRatio] = LOAD_ZSTD_TEXTURE(g_aspect_ratio);
    g_configThumbnails[&Config::ResolutionScale] = LOAD_ZSTD_TEXTURE(g_resolution_scale);
    g_configThumbnails[&Config::Fullscreen] = LOAD_ZSTD_TEXTURE(g_fullscreen);
    g_configThumbnails[&Config::VSync] = LOAD_ZSTD_TEXTURE(g_vsync);
    g_configThumbnails[&Config::FPS] = LOAD_ZSTD_TEXTURE(g_fps);
    g_configThumbnails[&Config::Brightness] = LOAD_ZSTD_TEXTURE(g_brightness);
    g_configThumbnails[&Config::AntiAliasing] = LOAD_ZSTD_TEXTURE(g_antialiasing);

    g_transparencyAntiAliasingThumbnails[false] = LOAD_ZSTD_TEXTURE(g_transparency_antialiasing_false);
    g_transparencyAntiAliasingThumbnails[true] = LOAD_ZSTD_TEXTURE(g_transparency_antialiasing_true);

    g_shadowResolutionThumbnails[EShadowResolution::Original] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_original);
    g_shadowResolutionThumbnails[EShadowResolution::x512] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x512);
    g_shadowResolutionThumbnails[EShadowResolution::x1024] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x1024);
    g_shadowResolutionThumbnails[EShadowResolution::x2048] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x2048);
    g_shadowResolutionThumbnails[EShadowResolution::x4096] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x4096);
    g_shadowResolutionThumbnails[EShadowResolution::x8192] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x8192);

    g_giTextureFilteringThumbnails[EGITextureFiltering::Bilinear] = LOAD_ZSTD_TEXTURE(g_gi_texture_filtering_bilinear);
    g_giTextureFilteringThumbnails[EGITextureFiltering::Bicubic] = LOAD_ZSTD_TEXTURE(g_gi_texture_filtering_bicubic);

    g_motionBlurThumbnails[EMotionBlur::Off] = LOAD_ZSTD_TEXTURE(g_motion_blur_off);
    g_motionBlurThumbnails[EMotionBlur::Original] = LOAD_ZSTD_TEXTURE(g_motion_blur_original);
    g_motionBlurThumbnails[EMotionBlur::Enhanced] = LOAD_ZSTD_TEXTURE(g_motion_blur_enhanced);

    g_xboxColorCorrectionThumbnails[false] = LOAD_ZSTD_TEXTURE(g_xbox_color_correction_false);
    g_xboxColorCorrectionThumbnails[true] = LOAD_ZSTD_TEXTURE(g_xbox_color_correction_true);

    g_configThumbnails[&Config::MovieScaleMode] = LOAD_ZSTD_TEXTURE(g_movie_scale_mode);
    g_configThumbnails[&Config::UIScaleMode] = LOAD_ZSTD_TEXTURE(g_ui_scale_mode);
}

template<typename T>
bool TryGetValueThumbnail(const IConfigDef* cfg, VALUE_THUMBNAIL_MAP(T)* thumbnails, GuestTexture** texture)
{
    if (!texture)
        return false;

    if (!cfg->GetValue())
        return false;

    auto result = thumbnails->at(*(T*)cfg->GetValue()).get();

    if (!result)
        return false;

    *texture = result;

    return true;
}

GuestTexture* GetThumbnail(const std::string_view name)
{
    if (!g_namedThumbnails.count(name))
        return g_namedThumbnails["Default"].get();

    return g_namedThumbnails[name].get();
}

GuestTexture* GetThumbnail(const IConfigDef* cfg)
{
    if (!g_configThumbnails.count(cfg))
    {
        auto texture = g_namedThumbnails["Default"].get();

        if (cfg == &Config::TimeOfDayTransition)
        {
            TryGetValueThumbnail<ETimeOfDayTransition>(cfg, &g_timeOfDayTransitionThumbnails, &texture);
        }
        else if (cfg == &Config::TransparencyAntiAliasing)
        {
            TryGetValueThumbnail<bool>(cfg, &g_transparencyAntiAliasingThumbnails, &texture);
        }
        else if (cfg == &Config::ShadowResolution)
        {
            TryGetValueThumbnail<EShadowResolution>(cfg, &g_shadowResolutionThumbnails, &texture);
        }
        else if (cfg == &Config::GITextureFiltering)
        {
            TryGetValueThumbnail<EGITextureFiltering>(cfg, &g_giTextureFilteringThumbnails, &texture);
        }
        else if (cfg == &Config::MotionBlur)
        {
            TryGetValueThumbnail<EMotionBlur>(cfg, &g_motionBlurThumbnails, &texture);
        }
        else if (cfg == &Config::XboxColorCorrection)
        {
            TryGetValueThumbnail<bool>(cfg, &g_xboxColorCorrectionThumbnails, &texture);
        }

        return texture;
    }

    return g_configThumbnails[cfg].get();
}
