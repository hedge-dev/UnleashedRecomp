#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PLUME_DIR="$ROOT/thirdparty/plume"
PATCH_FILE="$ROOT/tools/patches/plume-ios-sdl-vulkan.patch"

if [[ ! -d "$PLUME_DIR/.git" && ! -f "$PLUME_DIR/.git" ]]; then
    printf 'Missing Plume submodule. Run: git submodule update --init --recursive\n' >&2
    exit 1
fi

if git -C "$PLUME_DIR" apply --check "$PATCH_FILE" >/dev/null 2>&1; then
    git -C "$PLUME_DIR" apply "$PATCH_FILE"
    printf 'Applied Plume iOS patch.\n'
else
    if git -C "$PLUME_DIR" apply --reverse --check "$PATCH_FILE" >/dev/null 2>&1; then
        printf 'Plume iOS patch already applied.\n'
    else
        printf 'Plume iOS patch cannot be applied cleanly. Check thirdparty/plume for local changes.\n' >&2
        exit 1
    fi
fi
