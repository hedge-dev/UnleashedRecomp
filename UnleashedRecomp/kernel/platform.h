#pragma once

struct PlatformVersion
{
public:
    uint32_t Major{};
    uint32_t Minor{};
    uint32_t Build{};
};

extern PlatformVersion GetPlatformVersion();
