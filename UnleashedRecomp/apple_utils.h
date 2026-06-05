#pragma once

#include <filesystem>

namespace apple {

/// Get the user data directory (Documents folder on iOS, ~/Documents on macOS)
/// Returns absolute path suitable for user-editable files like saves and mods
std::filesystem::path GetUserDataDirectory();

/// Get the application support directory (Library/Application Support)
/// Returns absolute path suitable for app configuration and preferences
std::filesystem::path GetApplicationSupportDirectory();

/// Get the caches directory (Library/Caches)
/// Returns absolute path suitable for temporary/cached data
std::filesystem::path GetCachesDirectory();

/// Open a URL in the system browser
/// Returns true if the URL was opened successfully, false otherwise
bool OpenBrowser(const char* url);

/// Check if the current Apple device supports BC texture formats
/// (Apple 9 family and later, select Apple 7/8 family on iPadOS 16.4+)
bool SupportsBCTextures();

}  // namespace cocoa
