#pragma once

#define NOMINMAX

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(SWA_D3D12)
#include <dxcapi.h>
#endif

#include <ShlObj_core.h>
#include <algorithm>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <chrono>
#include <span>
#include <xbox.h>
#include <xxhash.h>
#include <ankerl/unordered_dense.h>
#include <ddspp.h>
#include <ppc/ppc_recomp_shared.h>
#include <toml++/toml.hpp>
#include <zstd.h>
#include <stb_image.h>
#include <concurrentqueue/blockingconcurrentqueue.h>
#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_sdl2.h>
#include <o1heap.h>
#include <cstddef>
#include <wrl/client.h>
#include <smolv.h>
#include <set>
#include <miniaudio.h>
#include <extras/miniaudio_libvorbis.h>
#include <fmt/core.h>

using Microsoft::WRL::ComPtr;

#include "framework.h"
#include "mutex.h"
