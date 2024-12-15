#include <ui/options_menu_thumbnails.h>

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
#include <res/images/options_menu/thumbnails/placeholder.dds.h>
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

#define VALUE_THUMBNAIL_MAP(type) std::unordered_map<std::string_view, std::unordered_map<type, std::unique_ptr<GuestTexture>>>

static std::unordered_map<std::string_view, std::unique_ptr<GuestTexture>> g_thumbnails;

static VALUE_THUMBNAIL_MAP(ETimeOfDayTransition) g_timeOfDayTransitionThumbnails;
static VALUE_THUMBNAIL_MAP(bool) g_transparencyAntiAliasingThumbnails;
static VALUE_THUMBNAIL_MAP(EShadowResolution) g_shadowResolutionThumbnails;
static VALUE_THUMBNAIL_MAP(EGITextureFiltering) g_giTextureFilteringThumbnails;
static VALUE_THUMBNAIL_MAP(EMotionBlur) g_motionBlurThumbnails;
static VALUE_THUMBNAIL_MAP(bool) g_xboxColorCorrectionThumbnails;

void LoadThumbnails()
{
    g_thumbnails["placeholder"] = LOAD_ZSTD_TEXTURE(g_placeholder);

    g_thumbnails[Config::Language.Name] = LOAD_ZSTD_TEXTURE(g_language);
    g_thumbnails[Config::Hints.Name] = LOAD_ZSTD_TEXTURE(g_hints);
    g_thumbnails[Config::ControlTutorial.Name] = LOAD_ZSTD_TEXTURE(g_control_tutorial);
    g_thumbnails[Config::AchievementNotifications.Name] = LOAD_ZSTD_TEXTURE(g_achievement_notifications);

    g_timeOfDayTransitionThumbnails[Config::TimeOfDayTransition.Name][ETimeOfDayTransition::Xbox] = LOAD_ZSTD_TEXTURE(g_time_of_day_transition_xbox);
    g_timeOfDayTransitionThumbnails[Config::TimeOfDayTransition.Name][ETimeOfDayTransition::PlayStation] = LOAD_ZSTD_TEXTURE(g_time_of_day_transition_playstation);

    g_thumbnails[Config::InvertCameraX.Name] = LOAD_ZSTD_TEXTURE(g_invert_camera_x);
    g_thumbnails[Config::InvertCameraY.Name] = LOAD_ZSTD_TEXTURE(g_invert_camera_y);
    g_thumbnails[Config::AllowBackgroundInput.Name] = LOAD_ZSTD_TEXTURE(g_allow_background_input);
    g_thumbnails[Config::AllowDPadMovement.Name] = LOAD_ZSTD_TEXTURE(g_allow_dpad_movement);
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

    g_transparencyAntiAliasingThumbnails[Config::TransparencyAntiAliasing.Name][false] = LOAD_ZSTD_TEXTURE(g_transparency_antialiasing_false);
    g_transparencyAntiAliasingThumbnails[Config::TransparencyAntiAliasing.Name][true] = LOAD_ZSTD_TEXTURE(g_transparency_antialiasing_true);

    g_shadowResolutionThumbnails[Config::ShadowResolution.Name][EShadowResolution::Original] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_original);
    g_shadowResolutionThumbnails[Config::ShadowResolution.Name][EShadowResolution::x512] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x512);
    g_shadowResolutionThumbnails[Config::ShadowResolution.Name][EShadowResolution::x1024] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x1024);
    g_shadowResolutionThumbnails[Config::ShadowResolution.Name][EShadowResolution::x2048] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x2048);
    g_shadowResolutionThumbnails[Config::ShadowResolution.Name][EShadowResolution::x4096] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x4096);
    g_shadowResolutionThumbnails[Config::ShadowResolution.Name][EShadowResolution::x8192] = LOAD_ZSTD_TEXTURE(g_shadow_resolution_x8192);

    g_giTextureFilteringThumbnails[Config::GITextureFiltering.Name][EGITextureFiltering::Bilinear] = LOAD_ZSTD_TEXTURE(g_gi_texture_filtering_bilinear);
    g_giTextureFilteringThumbnails[Config::GITextureFiltering.Name][EGITextureFiltering::Bicubic] = LOAD_ZSTD_TEXTURE(g_gi_texture_filtering_bicubic);

    g_motionBlurThumbnails[Config::MotionBlur.Name][EMotionBlur::Off] = LOAD_ZSTD_TEXTURE(g_motion_blur_off);
    g_motionBlurThumbnails[Config::MotionBlur.Name][EMotionBlur::Original] = LOAD_ZSTD_TEXTURE(g_motion_blur_original);
    g_motionBlurThumbnails[Config::MotionBlur.Name][EMotionBlur::Enhanced] = LOAD_ZSTD_TEXTURE(g_motion_blur_enhanced);

    g_xboxColorCorrectionThumbnails[Config::XboxColorCorrection.Name][false] = LOAD_ZSTD_TEXTURE(g_xbox_color_correction_false);
    g_xboxColorCorrectionThumbnails[Config::XboxColorCorrection.Name][true] = LOAD_ZSTD_TEXTURE(g_xbox_color_correction_true);

    g_thumbnails[Config::MovieScaleMode.Name] = LOAD_ZSTD_TEXTURE(g_movie_scale_mode);
    g_thumbnails[Config::UIScaleMode.Name] = LOAD_ZSTD_TEXTURE(g_ui_scale_mode);
}

template<typename T>
bool TryGetValueThumbnail(const IConfigDef* cfg, VALUE_THUMBNAIL_MAP(T)* thumbnails, GuestTexture** texture)
{
    if (!texture)
        return false;

    if (!thumbnails->count(cfg->GetName()))
        return false;

    if (!cfg->GetValue())
        return false;

    auto result = thumbnails->at(cfg->GetName())[*(T*)cfg->GetValue()].get();

    if (!result)
        return false;

    *texture = result;

    return true;
}

GuestTexture* GetThumbnail(const IConfigDef* cfg)
{
    if (!g_thumbnails.count(cfg->GetName()))
    {
        auto texture = g_thumbnails["placeholder"].get();

        TryGetValueThumbnail<ETimeOfDayTransition>(cfg, &g_timeOfDayTransitionThumbnails, &texture);
        TryGetValueThumbnail<bool>(cfg, &g_transparencyAntiAliasingThumbnails, &texture);
        TryGetValueThumbnail<EShadowResolution>(cfg, &g_shadowResolutionThumbnails, &texture);
        TryGetValueThumbnail<EGITextureFiltering>(cfg, &g_giTextureFilteringThumbnails, &texture);
        TryGetValueThumbnail<EMotionBlur>(cfg, &g_motionBlurThumbnails, &texture);
        TryGetValueThumbnail<bool>(cfg, &g_xboxColorCorrectionThumbnails, &texture);

        return texture;
    }

    return g_thumbnails[cfg->GetName()].get();
}
