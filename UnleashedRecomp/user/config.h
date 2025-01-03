#pragma once

#include <locale/locale.h>
#include <user/paths.h>
#include <exports.h>

class IConfigDef
{
public:
    virtual ~IConfigDef() = default;
    virtual void ReadValue(toml::v3::ex::parse_result& toml) = 0;
    virtual void MakeDefault() = 0;
    virtual std::string_view GetSection() const = 0;
    virtual std::string_view GetName() const = 0;
    virtual std::string GetNameLocalised(ELanguage language) const = 0;
    virtual std::string GetDescription(ELanguage language) const = 0;
    virtual bool IsDefaultValue() const = 0;
    virtual const void* GetValue() const = 0;
    virtual std::string GetValueLocalised(ELanguage language) const = 0;
    virtual std::string GetValueDescription(ELanguage language) const = 0;
    virtual std::string GetDefinition(bool withSection = false) const = 0;
    virtual std::string ToString(bool strWithQuotes = true) const = 0;
    virtual void GetLocaleStrings(std::vector<std::string_view>& localeStrings) const = 0;
};

#define CONFIG_LOCALE std::unordered_map<ELanguage, std::tuple<std::string, std::string>>
#define CONFIG_ENUM_LOCALE(type) std::unordered_map<ELanguage, std::unordered_map<type, std::tuple<std::string, std::string>>>

#define CONFIG_DEFINE(section, type, name, defaultValue) \
    static inline ConfigDef<type> name{section, #name, defaultValue};

#define CONFIG_DEFINE_LOCALISED(section, type, name, defaultValue) \
    static CONFIG_LOCALE g_##name##_locale; \
    static inline ConfigDef<type> name{section, #name, &g_##name##_locale, defaultValue};

#define CONFIG_DEFINE_ENUM(section, type, name, defaultValue) \
    static inline ConfigDef<type> name{section, #name, defaultValue, &g_##type##_template};

#define CONFIG_DEFINE_ENUM_LOCALISED(section, type, name, defaultValue) \
    static CONFIG_LOCALE g_##name##_locale; \
    static CONFIG_ENUM_LOCALE(type) g_##type##_locale; \
    static inline ConfigDef<type> name{section, #name, &g_##name##_locale, defaultValue, &g_##type##_template, &g_##type##_locale};

#define CONFIG_DEFINE_CALLBACK(section, type, name, defaultValue, readCallback) \
    static CONFIG_LOCALE g_##name##_locale; \
    static inline ConfigDef<type> name{section, #name, defaultValue, [](ConfigDef<type>* def) readCallback};

#define CONFIG_DEFINE_ENUM_TEMPLATE(type) \
    inline std::unordered_map<std::string, type> g_##type##_template =

#define WINDOWPOS_CENTRED 0x2FFF0000

inline std::vector<IConfigDef*> g_configDefinitions;

CONFIG_DEFINE_ENUM_TEMPLATE(ELanguage)
{
    { "English",  ELanguage::English },
    { "Japanese", ELanguage::Japanese },
    { "German",   ELanguage::German },
    { "French",   ELanguage::French },
    { "Spanish",  ELanguage::Spanish },
    { "Italian",  ELanguage::Italian }
};

enum class EUnleashGaugeBehaviour : uint32_t
{
    Original,
    Revised
};

CONFIG_DEFINE_ENUM_TEMPLATE(EUnleashGaugeBehaviour)
{
    { "Original", EUnleashGaugeBehaviour::Original },
    { "Revised",  EUnleashGaugeBehaviour::Revised }
};

enum class ETimeOfDayTransition : uint32_t
{
    Xbox,
    PlayStation
};

CONFIG_DEFINE_ENUM_TEMPLATE(ETimeOfDayTransition)
{
    { "Xbox",        ETimeOfDayTransition::Xbox },
    { "PlayStation", ETimeOfDayTransition::PlayStation }
};

enum class EControllerIcons : uint32_t
{
    Auto,
    Xbox,
    PlayStation
};

CONFIG_DEFINE_ENUM_TEMPLATE(EControllerIcons)
{
    { "Auto",        EControllerIcons::Auto },
    { "Xbox",        EControllerIcons::Xbox },
    { "PlayStation", EControllerIcons::PlayStation }
};

enum class EVoiceLanguage : uint32_t
{
    English,
    Japanese
};

CONFIG_DEFINE_ENUM_TEMPLATE(EVoiceLanguage)
{
    { "English",  EVoiceLanguage::English },
    { "Japanese", EVoiceLanguage::Japanese }
};

enum class EGraphicsAPI : uint32_t
{
#ifdef SWA_D3D12
    D3D12,
#endif
    Vulkan
};

CONFIG_DEFINE_ENUM_TEMPLATE(EGraphicsAPI)
{
#ifdef SWA_D3D12
    { "D3D12",  EGraphicsAPI::D3D12 },
#endif
    { "Vulkan", EGraphicsAPI::Vulkan }
};

enum class EWindowState : uint32_t
{
    Normal,
    Maximised
};

CONFIG_DEFINE_ENUM_TEMPLATE(EWindowState)
{
    { "Normal",    EWindowState::Normal },
    { "Maximised", EWindowState::Maximised },
    { "Maximized", EWindowState::Maximised }
};

enum class EAspectRatio : uint32_t
{
    Auto,
    Square,
    Widescreen
};

CONFIG_DEFINE_ENUM_TEMPLATE(EAspectRatio)
{
    { "Auto", EAspectRatio::Auto },
    { "4:3",  EAspectRatio::Square },
    { "16:9", EAspectRatio::Widescreen }
};

enum class ETripleBuffering : uint32_t
{
    Auto,
    On,
    Off
};

CONFIG_DEFINE_ENUM_TEMPLATE(ETripleBuffering)
{
    { "Auto", ETripleBuffering::Auto },
    { "On",   ETripleBuffering::On },
    { "Off",  ETripleBuffering::Off }
};

static constexpr int32_t FPS_MIN = 15;
static constexpr int32_t FPS_MAX = 240;

enum class EAntiAliasing : uint32_t
{
    None = 0,
    MSAA2x = 2,
    MSAA4x = 4,
    MSAA8x = 8
};

CONFIG_DEFINE_ENUM_TEMPLATE(EAntiAliasing)
{
    { "None",    EAntiAliasing::None },
    { "2x MSAA", EAntiAliasing::MSAA2x },
    { "4x MSAA", EAntiAliasing::MSAA4x },
    { "8x MSAA", EAntiAliasing::MSAA8x }
};

enum class EShadowResolution : int32_t
{
    Original = -1,
    x512 = 512,
    x1024 = 1024,
    x2048 = 2048,
    x4096 = 4096,
    x8192 = 8192
};

CONFIG_DEFINE_ENUM_TEMPLATE(EShadowResolution)
{
    { "Original", EShadowResolution::Original },
    { "512",      EShadowResolution::x512 },
    { "1024",     EShadowResolution::x1024 },
    { "2048",     EShadowResolution::x2048 },
    { "4096",     EShadowResolution::x4096 },
    { "8192",     EShadowResolution::x8192 },
};

enum class EGITextureFiltering : uint32_t
{
    Bilinear,
    Bicubic
};

CONFIG_DEFINE_ENUM_TEMPLATE(EGITextureFiltering)
{
    { "Bilinear", EGITextureFiltering::Bilinear },
    { "Bicubic",  EGITextureFiltering::Bicubic }
};

enum class EDepthOfFieldQuality : uint32_t
{
    Auto,
    Low,
    Medium,
    High,
    Ultra
};

CONFIG_DEFINE_ENUM_TEMPLATE(EDepthOfFieldQuality)
{
    { "Auto",   EDepthOfFieldQuality::Auto },
    { "Low",    EDepthOfFieldQuality::Low },
    { "Medium", EDepthOfFieldQuality::Medium },
    { "High",   EDepthOfFieldQuality::High },
    { "Ultra",  EDepthOfFieldQuality::Ultra }
};

enum class EMotionBlur : uint32_t
{
    Off,
    Original,
    Enhanced
};

CONFIG_DEFINE_ENUM_TEMPLATE(EMotionBlur)
{
    { "Off",      EMotionBlur::Off },
    { "Original", EMotionBlur::Original },
    { "Enhanced", EMotionBlur::Enhanced }
};

enum class EUIScaleMode : uint32_t
{
    Stretch,
    Edge,
    Centre
};

CONFIG_DEFINE_ENUM_TEMPLATE(EUIScaleMode)
{
    { "Stretch", EUIScaleMode::Stretch },
    { "Edge",    EUIScaleMode::Edge },
    { "Centre",  EUIScaleMode::Centre },
    { "Center",  EUIScaleMode::Centre }
};

template<typename T>
class ConfigDef final : public IConfigDef
{
public:
    std::string Section{};
    std::string Name{};
    CONFIG_LOCALE* Locale{};
    T DefaultValue{};
    T Value{ DefaultValue };
    std::unordered_map<std::string, T>* EnumTemplate;
    std::map<T, std::string> EnumTemplateReverse{};
    CONFIG_ENUM_LOCALE(T)* EnumLocale{};
    std::function<void(ConfigDef<T>*)> Callback;
    std::function<void(ConfigDef<T>*)> LockCallback;
    std::function<void(ConfigDef<T>*)> ApplyCallback;

    // CONFIG_DEFINE
    ConfigDef(std::string section, std::string name, T defaultValue) : Section(section), Name(name), DefaultValue(defaultValue)
    {
        g_configDefinitions.emplace_back(this);
    }

    // CONFIG_DEFINE_LOCALISED
    ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, T defaultValue) : Section(section), Name(name), Locale(nameLocale), DefaultValue(defaultValue)
    {
        g_configDefinitions.emplace_back(this);
    }

    // CONFIG_DEFINE_ENUM
    ConfigDef(std::string section, std::string name, T defaultValue, std::unordered_map<std::string, T>* enumTemplate) : Section(section), Name(name), DefaultValue(defaultValue), EnumTemplate(enumTemplate)
    {
        for (const auto& pair : *EnumTemplate)
            EnumTemplateReverse[pair.second] = pair.first;

        g_configDefinitions.emplace_back(this);
    }

    // CONFIG_DEFINE_ENUM_LOCALISED
    ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, T defaultValue, std::unordered_map<std::string, T>* enumTemplate, CONFIG_ENUM_LOCALE(T)* enumLocale) : Section(section), Name(name), Locale(nameLocale), DefaultValue(defaultValue), EnumTemplate(enumTemplate), EnumLocale(enumLocale)
    {
        for (const auto& pair : *EnumTemplate)
            EnumTemplateReverse[pair.second] = pair.first;

        g_configDefinitions.emplace_back(this);
    }

    // CONFIG_DEFINE_CALLBACK
    ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T>*)> callback) : Section(section), Name(name), DefaultValue(defaultValue), Callback(callback)
    {
        g_configDefinitions.emplace_back(this);
    }

    void ReadValue(toml::v3::ex::parse_result& toml) override
    {
        if (auto pSection = toml[Section].as_table())
        {
            const auto& section = *pSection;

            if constexpr (std::is_same<T, std::string>::value)
            {
                Value = section[Name].value_or<std::string>(DefaultValue);
            }
            else if constexpr (std::is_enum_v<T>)
            {
                std::string value = section[Name].value_or(std::string());
                auto it = EnumTemplate->find(value);
                if (it != EnumTemplate->end())
                {
                    Value = it->second;
                }
                else
                {
                    Value = DefaultValue;
                }
            }
            else
            {
                Value = section[Name].value_or(DefaultValue);
            }

            if (Callback)
                Callback(this);
        }
    }

    void MakeDefault() override
    {
        Value = DefaultValue;
    }

    std::string_view GetSection() const override
    {
        return Section;
    }

    std::string_view GetName() const override
    {
        return Name;
    }

    std::string GetNameLocalised(ELanguage language) const override
    {
        if (!Locale)
            return Name;

        if (!Locale->count(language))
        {
            if (Locale->count(ELanguage::English))
            {
                return std::get<0>(Locale->at(ELanguage::English));
            }
            else
            {
                return Name;
            }
        }

        return std::get<0>(Locale->at(language));
    }

    std::string GetDescription(ELanguage language) const override
    {
        if (!Locale)
            return "";

        if (!Locale->count(language))
        {
            if (Locale->count(ELanguage::English))
            {
                return std::get<1>(Locale->at(ELanguage::English));
            }
            else
            {
                return "";
            }
        }

        return std::get<1>(Locale->at(language));
    }

    bool IsDefaultValue() const override
    {
        return Value == DefaultValue;
    }

    const void* GetValue() const override
    {
        return &Value;
    }

    std::string GetValueLocalised(ELanguage language) const override
    {
        CONFIG_ENUM_LOCALE(T)* locale = nullptr;

        if constexpr (std::is_enum_v<T>)
        {
            locale = EnumLocale;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return Value
                ? Localise("Common_On")
                : Localise("Common_Off");
        }

        if (!locale)
            return ToString(false);

        if (!locale->count(language))
        {
            if (locale->count(ELanguage::English))
            {
                language = ELanguage::English;
            }
            else
            {
                return ToString(false);
            }
        }

        auto strings = locale->at(language);

        if (!strings.count(Value))
            return ToString(false);

        return std::get<0>(strings.at(Value));
    }

    std::string GetValueDescription(ELanguage language) const override
    {
        CONFIG_ENUM_LOCALE(T)* locale = nullptr;

        if constexpr (std::is_enum_v<T>)
        {
            locale = EnumLocale;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return "";
        }

        if (!locale)
            return "";

        if (!locale->count(language))
        {
            if (locale->count(ELanguage::English))
            {
                language = ELanguage::English;
            }
            else
            {
                return "";
            }
        }

        auto strings = locale->at(language);

        if (!strings.count(Value))
            return "";

        return std::get<1>(strings.at(Value));
    }

    std::string GetDefinition(bool withSection = false) const override
    {
        std::string result;

        if (withSection)
            result += "[" + Section + "]\n";

        result += Name + " = " + ToString();

        return result;
    }

    std::string ToString(bool strWithQuotes = true) const override
    {
        std::string result = "N/A";

        if constexpr (std::is_same_v<T, std::string>)
        {
            result = fmt::format("{}", Value);

            if (strWithQuotes)
                result = fmt::format("\"{}\"", result);
        }
        else if constexpr (std::is_enum_v<T>)
        {
            auto it = EnumTemplateReverse.find(Value);

            if (it != EnumTemplateReverse.end())
                result = fmt::format("{}", it->second);

            if (strWithQuotes)
                result = fmt::format("\"{}\"", result);
        }
        else
        {
            result = fmt::format("{}", Value);
        }

        return result;
    }

    void GetLocaleStrings(std::vector<std::string_view>& localeStrings) const override
    {
        if (Locale != nullptr)
        {
            for (auto& [language, nameAndDesc] : *Locale)
            {
                localeStrings.push_back(std::get<0>(nameAndDesc));
                localeStrings.push_back(std::get<1>(nameAndDesc));
            }
        }

        if (EnumLocale != nullptr)
        {
            for (auto& [language, locale] : *EnumLocale)
            {
                for (auto& [value, nameAndDesc] : locale)
                {
                    localeStrings.push_back(std::get<0>(nameAndDesc));
                    localeStrings.push_back(std::get<1>(nameAndDesc));
                }
            }
        }
    }

    ConfigDef& operator=(const ConfigDef& other)
    {
        if (this != &other)
            Value = other.Value;

        return *this;
    }

    operator T() const
    {
        return Value;
    }

    void operator=(const T& other)
    {
        Value = other;
    }
};

class Config
{
public:
    CONFIG_DEFINE_ENUM_LOCALISED("System", ELanguage, Language, ELanguage::English);
    CONFIG_DEFINE_LOCALISED("System", bool, Hints, true);
    CONFIG_DEFINE_LOCALISED("System", bool, ControlTutorial, true);
    CONFIG_DEFINE_LOCALISED("System", bool, AchievementNotifications, true);
    CONFIG_DEFINE_ENUM_LOCALISED("System", ETimeOfDayTransition, TimeOfDayTransition, ETimeOfDayTransition::Xbox);

    CONFIG_DEFINE_LOCALISED("Input", bool, InvertCameraX, false);
    CONFIG_DEFINE_LOCALISED("Input", bool, InvertCameraY, false);
    CONFIG_DEFINE_LOCALISED("Input", bool, Vibration, true);
    CONFIG_DEFINE_LOCALISED("Input", bool, AllowBackgroundInput, false);
    CONFIG_DEFINE_LOCALISED("Input", bool, AllowDPadMovement, false);
    CONFIG_DEFINE_ENUM_LOCALISED("Input", EControllerIcons, ControllerIcons, EControllerIcons::Auto);

    CONFIG_DEFINE_LOCALISED("Audio", float, MasterVolume, 1.0f);
    CONFIG_DEFINE_LOCALISED("Audio", float, MusicVolume, 1.0f);
    CONFIG_DEFINE_LOCALISED("Audio", float, EffectsVolume, 1.0f);
    CONFIG_DEFINE_ENUM_LOCALISED("Audio", EVoiceLanguage, VoiceLanguage, EVoiceLanguage::English);
    CONFIG_DEFINE_LOCALISED("Audio", bool, Subtitles, true);
    CONFIG_DEFINE_LOCALISED("Audio", bool, MusicAttenuation, false);
    CONFIG_DEFINE_LOCALISED("Audio", bool, BattleTheme, true);

#ifdef SWA_D3D12
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
    CONFIG_DEFINE_ENUM_LOCALISED("Video", EUIScaleMode, UIScaleMode, EUIScaleMode::Edge);

    // TODO: remove these once the exports are implemented.
    CONFIG_DEFINE("Exports", bool, AllowCancellingUnleash, false);
    CONFIG_DEFINE("Exports", bool, FixUnleashOutOfControlDrain, false);
    CONFIG_DEFINE("Exports", bool, HomingAttackOnBoost, true);
    CONFIG_DEFINE("Exports", bool, SaveScoreAtCheckpoints, false);
    CONFIG_DEFINE("Exports", bool, SkipIntroLogos, false);

    static std::filesystem::path GetConfigPath()
    {
        return GetUserPath() / "config.toml";
    }

    static void Load();
    static void Save();
};
