#pragma once

#define NOMINMAX

#include <windows.h>
#include <dxcapi.h>
#include <ShlObj_core.h>
#include <algorithm>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <format>
#include <vector>
#include <string>
#include <cassert>
#include <chrono>
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

using Microsoft::WRL::ComPtr;

#include "framework.h"
#include "mutex.h"
