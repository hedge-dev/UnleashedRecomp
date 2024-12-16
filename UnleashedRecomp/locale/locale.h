#pragma once

enum class ELanguage : uint32_t
{
    English = 1,
    Japanese,
    German,
    French,
    Spanish,
    Italian
};

inline std::string g_localeMissing = "<missing string>";

extern std::unordered_map<std::string, std::unordered_map<ELanguage, std::string>> g_locale;

std::string& Localise(const char* key);
