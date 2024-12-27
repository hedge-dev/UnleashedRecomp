#pragma once

struct ModLoader
{
    static std::filesystem::path RedirectPath(std::string_view path);

    static void Init();
};
