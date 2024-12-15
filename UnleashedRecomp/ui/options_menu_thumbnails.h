#pragma once

#include <gpu/video.h>
#include <user/config.h>
#include <decompressor.h>

void LoadThumbnails();

GuestTexture* GetThumbnail(const std::string_view name);
GuestTexture* GetThumbnail(const IConfigDef* cfg);
