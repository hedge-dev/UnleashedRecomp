#include <ui/options_menu_thumbnails.h>
#include <decompressor.h>
#include <hid/hid.h>

// TODO (Hyper): lower the resolution of these textures once final.
#include <res/images/options_menu/thumbnails/achievement_notifications.dds.h>
#include <res/images/options_menu/thumbnails/allow_background_input.dds.h>
#include <res/images/options_menu/thumbnails/antialiasing_none.dds.h>
#include <res/images/options_menu/thumbnails/antialiasing_2x.dds.h>
#include <res/images/options_menu/thumbnails/antialiasing_4x.dds.h>
#include <res/images/options_menu/thumbnails/antialiasing_8x.dds.h>
#include <res/images/options_menu/thumbnails/aspect_ratio.dds.h>
#include <res/images/options_menu/thumbnails/battle_theme.dds.h>
#include <res/images/options_menu/thumbnails/brightness.dds.h>
#include <res/images/options_menu/thumbnails/channel_stereo.dds.h>
#include <res/images/options_menu/thumbnails/channel_surround.dds.h>
#include <res/images/options_menu/thumbnails/control_tutorial_xb.dds.h>
#include <res/images/options_menu/thumbnails/control_tutorial_ps.dds.h>
#include <res/images/options_menu/thumbnails/controller_icons.dds.h>
#include <res/images/options_menu/thumbnails/default.dds.h>
#include <res/images/options_menu/thumbnails/effects_volume.dds.h>
#include <res/images/options_menu/thumbnails/fps.dds.h>
#include <res/images/options_menu/thumbnails/fullscreen.dds.h>
#include <res/images/options_menu/thumbnails/gi_texture_filtering_bilinear.dds.h>
#include <res/images/options_menu/thumbnails/gi_texture_filtering_bicubic.dds.h>
#include <res/images/options_menu/thumbnails/hints.dds.h>
#include <res/images/options_menu/thumbnails/horizontal_camera.dds.h>
#include <res/images/options_menu/thumbnails/language.dds.h>
#include <res/images/options_menu/thumbnails/master_volume.dds.h>
#include <res/images/options_menu/thumbnails/monitor.dds.h>
#include <res/images/options_menu/thumbnails/motion_blur_off.dds.h>
#include <res/images/options_menu/thumbnails/motion_blur_original.dds.h>
#include <res/images/options_menu/thumbnails/motion_blur_enhanced.dds.h>
#include <res/images/options_menu/thumbnails/movie_scale_fit.dds.h>
#include <res/images/options_menu/thumbnails/movie_scale_fill.dds.h>
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
#include <res/images/options_menu/thumbnails/time_transition_xb.dds.h>
#include <res/images/options_menu/thumbnails/time_transition_ps.dds.h>
#include <res/images/options_menu/thumbnails/transparency_antialiasing_false.dds.h>
#include <res/images/options_menu/thumbnails/transparency_antialiasing_true.dds.h>
#include <res/images/options_menu/thumbnails/ui_alignment_centre.dds.h>
#include <res/images/options_menu/thumbnails/ui_alignment_edge.dds.h>
#include <res/images/options_menu/thumbnails/vertical_camera.dds.h>
#include <res/images/options_menu/thumbnails/voice_language.dds.h>
#include <res/images/options_menu/thumbnails/vibration.dds.h>
#include <res/images/options_menu/thumbnails/vsync_on.dds.h>
#include <res/images/options_menu/thumbnails/vsync_off.dds.h>
#include <res/images/options_menu/thumbnails/window_size.dds.h>
#include <res/images/options_menu/thumbnails/xbox_color_correction.dds.h>

#define VALUE_THUMBNAIL_MAP(type) std::unordered_map<type, std::unique_ptr<GuestTexture>>

static std::unique_ptr<GuestTexture> g_defaultThumbnail;

static std::unique_ptr<GuestTexture> g_controlTutorialXBThumbnail;
static std::unique_ptr<GuestTexture> g_controlTutorialPSThumbnail;

static std::unordered_map<const IConfigDef*, std::unique_ptr<GuestTexture>> g_configThumbnails;

static VALUE_THUMBNAIL_MAP(ETimeOfDayTransition) g_timeOfDayTransitionThumbnails;
static VALUE_THUMBNAIL_MAP(EChannelConfiguration) g_channelConfigurationThumbnails;
static VALUE_THUMBNAIL_MAP(EAntiAliasing) g_msaaAntiAliasingThumbnails;
static VALUE_THUMBNAIL_MAP(bool) g_vsyncThumbnails;
static VALUE_THUMBNAIL_MAP(bool) g_transparencyAntiAliasingThumbnails;
static VALUE_THUMBNAIL_MAP(EShadowResolution) g_shadowResolutionThumbnails;
static VALUE_THUMBNAIL_MAP(EGITextureFiltering) g_giTextureFilteringThumbnails;
static VALUE_THUMBNAIL_MAP(EMotionBlur) g_motionBlurThumbnails;
static VALUE_THUMBNAIL_MAP(bool) g_xboxColorCorrectionThumbnails;
static VALUE_THUMBNAIL_MAP(ECutsceneAspectRatio) g_cutsceneAspectRatioThumbnails;
static VALUE_THUMBNAIL_MAP(EUIAlignmentMode) g_uiAlignmentThumbnails;

void LoadThumbnails()
{
    g_defaultThumbnail = LOAD_ZSTD_TEXTURE(g_default);

    g_controlTutorialXBThumbnail = LOAD_ZSTD_TEXTURE(g_control_tutorial_xb);
    g_controlTutorialPSThumbnail = LOAD_ZSTD_TEXTURE(g_control_tutorial_ps);

    g_configThumbnails[&Config::Language] = LOAD_ZSTD_TEXTURE(g_language);
    g_configThumbnails[&Config::VoiceLanguage] = LOAD_ZSTD_TEXTURE(g_voice_language);
    g_configThumbnails[&Config::Subtitles] = LOAD_ZSTD_TEXTURE(g_subtitles);
    g_configThumbnails[&Config::Hints] = LOAD_ZSTD_TEXTURE(g_hints);
    g_configThumbnails[&Config::AchievementNotifications] = LOAD_ZSTD_TEXTURE(g_achievement_notifications);

    g_timeOfDayTransitionThumbnails[ETimeOfDayTransition::Xbox] = LOAD_ZSTD_TEXTURE(g_time_of_day_transition_xbox);
    g_timeOfDayTransitionThumbnails[ETimeOfDayTransition::PlayStation] = LOAD_ZSTD_TEXTURE(g_time_of_day_transition_playstation);

    g_configThumbnails[&Config::HorizontalCamera] = LOAD_ZSTD_TEXTURE(g_horizontal_camera);
    g_configThumbnails[&Config::VerticalCamera] = LOAD_ZSTD_TEXTURE(g_vertical_camera);
    g_configThumbnails[&Config::Vibration] = LOAD_ZSTD_TEXTURE(g_vibration);
    g_configThumbnails[&Config::AllowBackgroundInput] = LOAD_ZSTD_TEXTURE(g_allow_background_input);
    g_configThumbnails[&Config::ControllerIcons] = LOAD_ZSTD_TEXTURE(g_controller_icons);
    g_configThumbnails[&Config::MasterVolume] = LOAD_ZSTD_TEXTURE(g_master_volume);
    g_configThumbnails[&Config::MusicVolume] = LOAD_ZSTD_TEXTURE(g_music_volume);
    g_configThumbnails[&Config::EffectsVolume] = LOAD_ZSTD_TEXTURE(g_effects_volume);

    g_channelConfigurationThumbnails[EChannelConfiguration::Stereo] = LOAD_ZSTD_TEXTURE(g_channel_stereo);
    g_channelConfigurationThumbnails[EChannelConfiguration::Surround] = LOAD_ZSTD_TEXTURE(g_channel_surround);

    g_configThumbnails[&Config::MusicAttenuation] = LOAD_ZSTD_TEXTURE(g_music_attenuation);
    g_configThumbnails[&Config::BattleTheme] = LOAD_ZSTD_TEXTURE(g_battle_theme);
    g_configThumbnails[&Config::WindowSize] = LOAD_ZSTD_TEXTURE(g_window_size);
    g_configThumbnails[&Config::Monitor] = LOAD_ZSTD_TEXTURE(g_monitor);
    g_configThumbnails[&Config::AspectRatio] = LOAD_ZSTD_TEXTURE(g_aspect_ratio);
    g_configThumbnails[&Config::ResolutionScale] = LOAD_ZSTD_TEXTURE(g_resolution_scale);
    g_configThumbnails[&Config::Fullscreen] = LOAD_ZSTD_TEXTURE(g_fullscreen);
    g_configThumbnails[&Config::XboxColorCorrection] = LOAD_ZSTD_TEXTURE(g_xbox_color_correction);

    g_vsyncThumbnails[false] = LOAD_ZSTD_TEXTURE(g_vsync_off);
    g_vsyncThumbnails[true] = LOAD_ZSTD_TEXTURE(g_vsync_on);

    g_configThumbnails[&Config::FPS] = LOAD_ZSTD_TEXTURE(g_fps);
    g_configThumbnails[&Config::Brightness] = LOAD_ZSTD_TEXTURE(g_brightness);

    g_msaaAntiAliasingThumbnails[EAntiAliasing::None] = LOAD_ZSTD_TEXTURE(g_antialiasing_none);
    g_msaaAntiAliasingThumbnails[EAntiAliasing::MSAA2x] = LOAD_ZSTD_TEXTURE(g_antialiasing_2x);
    g_msaaAntiAliasingThumbnails[EAntiAliasing::MSAA4x] = LOAD_ZSTD_TEXTURE(g_antialiasing_4x);
    g_msaaAntiAliasingThumbnails[EAntiAliasing::MSAA8x] = LOAD_ZSTD_TEXTURE(g_antialiasing_8x);

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

    g_cutsceneAspectRatioThumbnails[ECutsceneAspectRatio::Original] = LOAD_ZSTD_TEXTURE(g_movie_scale_fit);
    g_cutsceneAspectRatioThumbnails[ECutsceneAspectRatio::Unlocked] = LOAD_ZSTD_TEXTURE(g_movie_scale_fill);

    g_uiAlignmentThumbnails[EUIAlignmentMode::Centre] = LOAD_ZSTD_TEXTURE(g_ui_alignment_centre);
    g_uiAlignmentThumbnails[EUIAlignmentMode::Edge] = LOAD_ZSTD_TEXTURE(g_ui_alignment_edge);
}

template<typename T>
bool TryGetValueThumbnail(const IConfigDef* cfg, VALUE_THUMBNAIL_MAP(T)* thumbnails, GuestTexture** texture)
{
    if (!texture)
        return false;

    if (!cfg->GetValue())
        return false;

    auto findResult = thumbnails->find(*(T*)cfg->GetValue());

    if (findResult != thumbnails->end())
    {
        *texture = findResult->second.get();
        return true;
    }

    return false;
}

GuestTexture* GetThumbnail(const IConfigDef* cfg)
{
    auto findResult = g_configThumbnails.find(cfg);
    if (findResult == g_configThumbnails.end())
    {
        auto texture = g_defaultThumbnail.get();

        if (cfg == &Config::ControlTutorial)
        {
            bool isPlayStation = Config::ControllerIcons == EControllerIcons::PlayStation;

            if (Config::ControllerIcons == EControllerIcons::Auto)
                isPlayStation = hid::g_inputDeviceController == hid::EInputDevice::PlayStation;

            texture = isPlayStation ? g_controlTutorialPSThumbnail.get() : g_controlTutorialXBThumbnail.get();
        }
        if (cfg == &Config::TimeOfDayTransition)
        {
            TryGetValueThumbnail<ETimeOfDayTransition>(cfg, &g_timeOfDayTransitionThumbnails, &texture);
        }
        else if (cfg == &Config::AntiAliasing)
        {
            TryGetValueThumbnail<EAntiAliasing>(cfg, &g_msaaAntiAliasingThumbnails, &texture);
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
        else if (cfg == &Config::CutsceneAspectRatio)
        {
            TryGetValueThumbnail<ECutsceneAspectRatio>(cfg, &g_cutsceneAspectRatioThumbnails, &texture);
        }
        else if (cfg == &Config::VSync)
        {
            TryGetValueThumbnail<bool>(cfg, &g_vsyncThumbnails, &texture);
        }
        else if (cfg == &Config::ChannelConfiguration)
        {
            TryGetValueThumbnail<EChannelConfiguration>(cfg, &g_channelConfigurationThumbnails, &texture);
        }
        else if (cfg == &Config::UIAlignmentMode)
        {
            TryGetValueThumbnail<EUIAlignmentMode>(cfg, &g_uiAlignmentThumbnails, &texture);
        }

        return texture;
    }

    return findResult->second.get();
}
