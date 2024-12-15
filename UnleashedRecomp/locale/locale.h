#pragma once

#include <user/config.h>

inline std::string g_localeMissing = "<missing string>";

extern std::unordered_map<std::string, std::unordered_map<ELanguage, std::string>> g_locale;

std::string& Localise(const char* key);
