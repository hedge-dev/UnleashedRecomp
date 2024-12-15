#pragma once

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

enum class ELanguage : uint32_t
{
    English = 1,
    Japanese,
    German,
    French,
    Spanish,
    Italian
};

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
    { "Auto", EControllerIcons::Auto },
    { "Xbox", EControllerIcons::Xbox },
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
    D3D12,
    Vulkan
};

CONFIG_DEFINE_ENUM_TEMPLATE(EGraphicsAPI)
{
    { "D3D12",  EGraphicsAPI::D3D12 },
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
    { "On",  ETripleBuffering::On },
    { "Off", ETripleBuffering::Off }
};

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

enum class EMovieScaleMode : uint32_t
{
    Stretch,
    Fit,
    Fill
};

CONFIG_DEFINE_ENUM_TEMPLATE(EMovieScaleMode)
{
    { "Stretch", EMovieScaleMode::Stretch },
    { "Fit",     EMovieScaleMode::Fit },
    { "Fill",    EMovieScaleMode::Fill }
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

class IConfigDef
{
public:
    virtual ~IConfigDef() = default;
    virtual void ReadValue(toml::v3::ex::parse_result& toml) = 0;
    virtual void MakeDefault() = 0;
    virtual std::string_view GetSection() const = 0;
    virtual std::string_view GetName() const = 0;
    virtual std::string GetNameLocalised() const = 0;
    virtual std::string GetDescription() const = 0;
    virtual bool IsDefaultValue() const = 0;
    virtual const void* GetValue() const = 0;
    virtual std::string GetValueLocalised() const = 0;
    virtual std::string GetValueDescription() const = 0;
    virtual std::string GetDefinition(bool withSection = false) const = 0;
    virtual std::string ToString(bool strWithQuotes = true) const = 0;
    virtual void GetLocaleStrings(std::vector<std::string_view>& localeStrings) const = 0;
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

    // CONFIG_DEFINE
    ConfigDef(std::string section, std::string name, T defaultValue);

    // CONFIG_DEFINE_LOCALISED
    ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, T defaultValue);

    // CONFIG_DEFINE_ENUM
    ConfigDef(std::string section, std::string name, T defaultValue, std::unordered_map<std::string, T>* enumTemplate);

    // CONFIG_DEFINE_ENUM_LOCALISED
    ConfigDef(std::string section, std::string name, CONFIG_LOCALE* nameLocale, T defaultValue, std::unordered_map<std::string, T>* enumTemplate, CONFIG_ENUM_LOCALE(T)* enumLocale);

    // CONFIG_DEFINE_CALLBACK
    ConfigDef(std::string section, std::string name, T defaultValue, std::function<void(ConfigDef<T>*)> callback);

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

    std::string GetNameLocalised() const override;

    std::string GetDescription() const override;

    bool IsDefaultValue() const override
    {
        return Value == DefaultValue;
    }

    const void* GetValue() const override
    {
        return &Value;
    }

    std::string GetValueLocalised() const override;

    std::string GetValueDescription() const override;

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

    void GetLocaleStrings(std::vector<std::string_view>& localeStrings) const override;

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
