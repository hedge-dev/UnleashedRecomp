#!/usr/bin/env bash
#
# Build UnleashedRecomp on Linux.
#
# Usage: ./build.sh [preset] [target]
#   preset   linux-release (default) | linux-relwithdebinfo | linux-debug
#   target   CMake target (default: UnleashedRecomp)
#
# Env: SKIP_SUBMODULES=1  SKIP_CONFIGURE=1  JOBS=<n>

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

PRESET="${1:-linux-release}"
TARGET="${2:-UnleashedRecomp}"
BUILD_DIR="out/build/${PRESET}"
JOBS="${JOBS:-$(nproc)}"

log() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }

# Submodules: the presets' CMAKE_TOOLCHAIN_FILE lives in thirdparty/vcpkg.
if [[ "${SKIP_SUBMODULES:-0}" != "1" ]]; then
    log "Updating git submodules"
    git submodule update --init --recursive
fi

# Vendored SDL2 passes a pw_proxy* where PipeWire wants a pw_node*; modern
# Clang makes that a hard error. Same handle at runtime, so just cast.
SDL_PIPEWIRE="thirdparty/SDL/src/audio/pipewire/SDL_pipewire.c"
if [[ -f "$SDL_PIPEWIRE" ]] && grep -q 'pw_node_enum_params(node->proxy' "$SDL_PIPEWIRE"; then
    log "Patching $SDL_PIPEWIRE"
    sed -i 's/pw_node_enum_params(node->proxy,/pw_node_enum_params((struct pw_node *)node->proxy,/' "$SDL_PIPEWIRE"
fi

if [[ "${SKIP_CONFIGURE:-0}" != "1" || ! -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
    log "Configuring preset: $PRESET"
    cmake . --preset "$PRESET"
fi

log "Building '$TARGET' ($JOBS jobs)"
cmake --build "$BUILD_DIR" --target "$TARGET" -j "$JOBS"

BIN="${BUILD_DIR}/UnleashedRecomp/UnleashedRecomp"
if [[ -x "$BIN" ]]; then
    log "Done: $REPO_ROOT/$BIN"
else
    log "Done. Output under $REPO_ROOT/$BUILD_DIR"
fi
