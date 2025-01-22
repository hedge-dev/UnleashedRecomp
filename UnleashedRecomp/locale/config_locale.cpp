#include <user/config.h>

/*
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! LOCALISATION NOTES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    - Ensure brand names are always presented on the same line.

      Correct:
      This is a string that contains a brand name like
      Xbox 360, which is one of the few consoles to have a port of
      Sonic Unleashed.

      Incorrect:
      This is a string that contains a brand name like Xbox
      360, which is one of the few consoles to have a port of Sonic
      Unleashed.

    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#define CONFIG_DEFINE_LOCALE(name) \
    CONFIG_LOCALE g_##name##_locale =

#define CONFIG_DEFINE_ENUM_LOCALE(type) \
    CONFIG_ENUM_LOCALE(type) g_##type##_locale =

CONFIG_DEFINE_LOCALE(Language)
{
    { ELanguage::English,  { "Language", "Change the language used for text and logos." } },
    { ELanguage::Japanese, { "言語", "[PLACEHOLDER]" } },
    { ELanguage::German,   { "Sprache", "[PLACEHOLDER]" } },
    { ELanguage::French,   { "Langue", "[PLACEHOLDER]" } },
    { ELanguage::Spanish,  { "Idioma", "[PLACEHOLDER]" } },
    { ELanguage::Italian,  { "Lingua", "[PLACEHOLDER]" } }
};

CONFIG_DEFINE_ENUM_LOCALE(ELanguage)
{
    {
        ELanguage::English,
        {
            { ELanguage::English,  { "ENGLISH", "" } },
            { ELanguage::Japanese, { "日本語", "" } },
            { ELanguage::German,   { "DEUTSCH", "" } },
            { ELanguage::French,   { "FRANÇAIS", "" } },
            { ELanguage::Spanish,  { "ESPAÑOL", "" } },
            { ELanguage::Italian,  { "ITALIANO", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(Hints)
{
    { ELanguage::English, { "Hints", "Show hint rings in stages." } }
};

CONFIG_DEFINE_LOCALE(ControlTutorial)
{
    { ELanguage::English, { "Control Tutorial", "Show controller hints in stages." } }
};

CONFIG_DEFINE_LOCALE(AchievementNotifications)
{
    { ELanguage::English, { "Achievement Notifications", "Show notifications for unlocking achievements.\n\nAchievements will still be rewarded with notifications disabled." } }
};

CONFIG_DEFINE_LOCALE(TimeOfDayTransition)
{
    { ELanguage::English, { "Time of Day Transition", "Change how the loading screen appears when switching time of day in the hub areas." } }
};

CONFIG_DEFINE_ENUM_LOCALE(ETimeOfDayTransition)
{
    {
        ELanguage::English,
        {
            { ETimeOfDayTransition::Xbox,        { "XBOX", "Xbox: the transformation cutscene will play with artificial loading times." } },
            { ETimeOfDayTransition::PlayStation, { "PLAYSTATION", "PlayStation: a spinning medal loading screen will be used instead." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ControllerIcons)
{
    { ELanguage::English, { "Controller Icons", "Change the icons to match your controller." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EControllerIcons)
{
    {
        ELanguage::English,
        {
            { EControllerIcons::Auto,        { "AUTO", "Auto: the game will determine which icons to use based on the current input device." } },
            { EControllerIcons::Xbox,        { "XBOX", "" } },
            { EControllerIcons::PlayStation, { "PLAYSTATION", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(InvertCameraX)
{
    { ELanguage::English, { "Invert Camera X", "Toggle between inverted left and right camera movement." } }
};

CONFIG_DEFINE_LOCALE(InvertCameraY)
{
    { ELanguage::English, { "Invert Camera Y", "Toggle between inverted up and down camera movement." } }
};

CONFIG_DEFINE_LOCALE(Vibration)
{
    { ELanguage::English, { "Vibration", "Toggle controller vibration." } }
};

CONFIG_DEFINE_LOCALE(AllowBackgroundInput)
{
    { ELanguage::English, { "Allow Background Input", "Allow controller input whilst the game window is unfocused." } }
};

CONFIG_DEFINE_LOCALE(MasterVolume)
{
    { ELanguage::English, { "Master Volume", "Adjust the overall volume." } }
};

CONFIG_DEFINE_LOCALE(MusicVolume)
{
    { ELanguage::English, { "Music Volume", "Adjust the volume for the music." } }
};

CONFIG_DEFINE_LOCALE(EffectsVolume)
{
    { ELanguage::English, { "Effects Volume", "Adjust the volume for sound effects." } }
};

CONFIG_DEFINE_LOCALE(MusicAttenuation)
{
    { ELanguage::English, { "Music Attenuation", "Fade out the game's music when external media is playing." } }
};

CONFIG_DEFINE_LOCALE(ChannelConfiguration)
{
    { ELanguage::English, { "Channel Configuration", "" } }
};

CONFIG_DEFINE_ENUM_LOCALE(EChannelConfiguration)
{
    {
        ELanguage::English,
        {
            { EChannelConfiguration::Stereo,   { "STEREO", "" } },
            { EChannelConfiguration::Surround, { "SURROUND", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(VoiceLanguage)
{
    { ELanguage::English, { "Voice Language", "Change the language used for character voices." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EVoiceLanguage)
{
    {
        ELanguage::English,
        {
            { EVoiceLanguage::English,  { "ENGLISH", "" } },
            { EVoiceLanguage::Japanese, { "日本語", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(Subtitles)
{
    { ELanguage::English, { "Subtitles", "Show subtitles during dialogue." } }
};

CONFIG_DEFINE_LOCALE(BattleTheme)
{
    { ELanguage::English, { "Battle Theme", "Play the Werehog battle theme during combat.\n\nThis option will apply the next time you're in combat." } }
};

CONFIG_DEFINE_LOCALE(WindowSize)
{
    { ELanguage::English, { "Window Size", "Adjust the size of the game window in windowed mode." } }
};

CONFIG_DEFINE_LOCALE(Monitor)
{
    { ELanguage::English, { "Monitor", "Change which monitor to display the game on." } }
};

CONFIG_DEFINE_LOCALE(AspectRatio)
{
    { ELanguage::English, { "Aspect Ratio", "Change the aspect ratio." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAspectRatio)
{
    {
        ELanguage::English,
        {
            { EAspectRatio::Auto, { "AUTO", "Auto: the aspect ratio will dynamically adjust to the window size." } },
            { EAspectRatio::OriginalNarrow, { "ORIGINAL 4:3", "" } }
        }
    }
};

CONFIG_DEFINE_LOCALE(ResolutionScale)
{
    { ELanguage::English, { "Resolution Scale", "Adjust the internal resolution of the game.\n\n%dx%d" } }
};

CONFIG_DEFINE_LOCALE(Fullscreen)
{
    { ELanguage::English, { "Fullscreen", "Toggle between borderless fullscreen or windowed mode." } }
};

CONFIG_DEFINE_LOCALE(VSync)
{
    { ELanguage::English, { "V-Sync", "Synchronize the game to the refresh rate of the display to prevent screen tearing." } }
};

CONFIG_DEFINE_LOCALE(FPS)
{
    { ELanguage::English, { "FPS", "Set the max frame rate the game can run at.\n\nWARNING: this may introduce glitches at frame rates higher than 60 FPS." } }
};

CONFIG_DEFINE_LOCALE(Brightness)
{
    { ELanguage::English, { "Brightness", "Adjust the brightness level of the game." } }
};

CONFIG_DEFINE_LOCALE(AntiAliasing)
{
    { ELanguage::English, { "Anti-Aliasing", "Adjust the amount of smoothing applied to jagged edges." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EAntiAliasing)
{
    {
        ELanguage::English,
        {
            { EAntiAliasing::None, { "NONE", "" } },
        }
    }
};

CONFIG_DEFINE_LOCALE(TransparencyAntiAliasing)
{
    { ELanguage::English, { "Transparency Anti-Aliasing", "Apply anti-aliasing to alpha transparent textures." } }
};

CONFIG_DEFINE_LOCALE(ShadowResolution)
{
    { ELanguage::English, { "Shadow Resolution", "Set the resolution of real-time shadows." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EShadowResolution)
{
    {
        ELanguage::English,
        {
            { EShadowResolution::Original, { "ORIGINAL", "Original: the game will automatically determine the resolution of the shadows." } },
            { EShadowResolution::x4096,    { "4096", "High resolutions can degrade performance significantly on lower end hardware." } },
            { EShadowResolution::x8192,    { "8192", "High resolutions can degrade performance significantly on lower end hardware." } },
        }
    }
};

CONFIG_DEFINE_LOCALE(GITextureFiltering)
{
    { ELanguage::English, { "GI Texture Filtering", "Change the quality of the filtering used for global illumination textures." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EGITextureFiltering)
{
    {
        ELanguage::English,
        {
            { EGITextureFiltering::Bilinear, { "BILINEAR", "" } },
            { EGITextureFiltering::Bicubic,  { "BICUBIC", "" } },
        }
    }
};

CONFIG_DEFINE_LOCALE(MotionBlur)
{
    { ELanguage::English, { "Motion Blur", "Change the quality of the motion blur." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EMotionBlur)
{
    {
        ELanguage::English,
        {
            { EMotionBlur::Off,      { "OFF", "" } },
            { EMotionBlur::Original, { "ORIGINAL", "" } },
            { EMotionBlur::Enhanced, { "ENHANCED", "Enhanced: uses more samples for smoother motion blur at the cost of performance." } }
        }
    }
};

CONFIG_DEFINE_LOCALE(XboxColorCorrection)
{
    { ELanguage::English, { "Xbox Color Correction", "Use the warm tint from the Xbox version of the game." } }
};

CONFIG_DEFINE_LOCALE(CutsceneAspectRatio)
{
    { ELanguage::English, { "Cutscene Aspect Ratio", "" } }
};

CONFIG_DEFINE_ENUM_LOCALE(ECutsceneAspectRatio)
{
    {
        ELanguage::English,
        {
            { ECutsceneAspectRatio::Original, { "ORIGINAL", "" } },
            { ECutsceneAspectRatio::Unlocked, { "UNLOCKED", "" } },
        }
    }
};

CONFIG_DEFINE_LOCALE(UIScaleMode)
{
    { ELanguage::English, { "UI Scale Mode", "Change how the UI scales to the display." } }
};

CONFIG_DEFINE_ENUM_LOCALE(EUIScaleMode)
{
    {
        ELanguage::English,
        {
            { EUIScaleMode::Edge,    { "EDGE", "Edge: the UI will anchor to the edges of the display." } },
            { EUIScaleMode::Centre,  { "CENTER", "Center: the UI will anchor to the center of the display." } },
        }
    }
};
