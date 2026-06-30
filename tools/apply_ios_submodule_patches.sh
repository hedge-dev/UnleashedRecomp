#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PLUME_DIR="$ROOT/thirdparty/plume"
PLUME_PATCH_FILE="$ROOT/tools/patches/plume-ios-metal.patch"
XENON_RECOMP_DIR="$ROOT/tools/XenonRecomp"
XENON_RECOMP_PATCH_FILE="$ROOT/tools/patches/xenonrecomp-ios-streaming-memory-map.patch"

if [[ ! -d "$PLUME_DIR/.git" && ! -f "$PLUME_DIR/.git" ]]; then
    printf 'Missing Plume submodule. Run: git submodule update --init --recursive\n' >&2
    exit 1
fi

if [[ ! -d "$XENON_RECOMP_DIR/.git" && ! -f "$XENON_RECOMP_DIR/.git" ]]; then
    printf 'Missing XenonRecomp submodule. Run: git submodule update --init --recursive\n' >&2
    exit 1
fi

if git -C "$PLUME_DIR" apply --check "$PLUME_PATCH_FILE" >/dev/null 2>&1; then
    git -C "$PLUME_DIR" apply "$PLUME_PATCH_FILE"
    printf 'Applied Plume iOS patch.\n'
else
    if git -C "$PLUME_DIR" apply --reverse --check "$PLUME_PATCH_FILE" >/dev/null 2>&1; then
        printf 'Plume iOS patch already applied.\n'
    else
        printf 'Plume iOS patch cannot be applied cleanly. Check thirdparty/plume for local changes.\n' >&2
        exit 1
    fi
fi

if git -C "$XENON_RECOMP_DIR" apply --check "$XENON_RECOMP_PATCH_FILE" >/dev/null 2>&1; then
    git -C "$XENON_RECOMP_DIR" apply "$XENON_RECOMP_PATCH_FILE"
    printf 'Applied XenonRecomp iOS patch.\n'
else
    if git -C "$XENON_RECOMP_DIR" apply --reverse --check "$XENON_RECOMP_PATCH_FILE" >/dev/null 2>&1; then
        printf 'XenonRecomp iOS patch already applied.\n'
    else
        printf 'XenonRecomp iOS patch cannot be applied cleanly. Check tools/XenonRecomp for local changes.\n' >&2
        exit 1
    fi
fi
