#pragma once

#include <cfg/config.h>

inline static std::string g_localeMissing = "<missing string>";

inline static std::unordered_map<std::string, std::unordered_map<ELanguage, std::string>> g_locale
{
    {
        "Options_Category_System",
        {
            { ELanguage::English, "SYSTEM" }
        }
    },
    {
        "Options_Category_Input",
        {
            { ELanguage::English, "INPUT" }
        }
    },
    {
        "Options_Category_Audio",
        {
            { ELanguage::English, "AUDIO" }
        }
    },
    {
        "Options_Category_Video",
        {
            { ELanguage::English, "VIDEO" }
        }
    },
    {
        "Options_Value_Max",
        {
            { ELanguage::English, "MAX" }
        }
    },
    {
        "Options_Name_WindowSize",
        {
            { ELanguage::English, "Window Size" }
        }
    },
    {
        "Options_Desc_WindowSize",
        {
            { ELanguage::English, "Adjust the size of the game window in windowed mode." }
        }
    },
    {
        "Options_Desc_NotAvailable",
        {
            { ELanguage::English, "This option is not available at this location." }
        }
    },
    {
        "Options_Desc_NotAvailableMSAA",
        {
            { ELanguage::English, "This option is not available without MSAA." }
        }
    }
};

static std::string& Localise(const char* key)
{
    if (!g_locale.count(key))
        return g_localeMissing;

    if (!g_locale[key].count(Config::Language))
    {
        if (g_locale[key].count(ELanguage::English))
        {
            return g_locale[key][ELanguage::English];
        }
        else
        {
            return g_localeMissing;
        }
    }

    return g_locale[key][Config::Language];
}
