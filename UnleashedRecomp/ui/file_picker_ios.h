#pragma once

#include <filesystem>
#include <list>
#include <string>

namespace ios
{
    bool PickPaths(bool folderMode, std::list<std::filesystem::path>& outPaths, std::string& outError);
    void ReleaseAllAccess();
}
