#pragma once

#include <gpu/video.h>
#include <xdbf_wrapper.h>

extern XDBFWrapper g_xdbfWrapper;
extern std::unordered_map<uint16_t, GuestTexture*> g_xdbfTextureCache;
