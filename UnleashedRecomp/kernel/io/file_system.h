#pragma once

struct FileSystem
{
    static std::filesystem::path ResolvePath(const std::string_view& path, bool checkForMods);
};
