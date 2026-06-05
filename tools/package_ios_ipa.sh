#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/out/build/ios-device-release"
HOST_BUILD_DIR="$ROOT/out/build/macos-release"
IPA_DIR="$ROOT/out/ipa"
PAYLOAD_DIR="$IPA_DIR/Payload"
APP_PATH="$BUILD_DIR/UnleashedRecomp/Unleashed Recompiled.app"
IPA_PATH="$IPA_DIR/UnleashedRecompiled.ipa"
DEFAULT_SIGNING_IDENTITY="Apple Development: aroblesalago@gmail.com (MK28YRUCCG)"

"$ROOT/tools/apply_ios_submodule_patches.sh"

if [[ -n "${ISO_PATH:-}" ]]; then
    cmake --preset macos-release
    cmake --build "$HOST_BUILD_DIR" --target iso_extract -j "${JOBS:-8}"
    "$HOST_BUILD_DIR/tools/iso_extract/iso_extract" "$ISO_PATH" \
        default.xex "$ROOT/UnleashedRecompLib/private/default.xex" \
        shader.ar "$ROOT/UnleashedRecompLib/private/shader.ar"
fi

if [[ -n "${XEXP_PATH:-}" ]]; then
    mkdir -p "$ROOT/UnleashedRecompLib/private"
    cp "$XEXP_PATH" "$ROOT/UnleashedRecompLib/private/default.xexp"
fi

if [[ -n "${UPDATE_PATH:-}" ]]; then
    cmake --preset macos-release
    cmake --build "$HOST_BUILD_DIR" --target xcontent_extract -j "${JOBS:-8}"
    "$HOST_BUILD_DIR/tools/xcontent_extract/xcontent_extract" "$UPDATE_PATH" \
        default.xexp "$ROOT/UnleashedRecompLib/private/default.xexp"
fi

missing=0
for file in \
    "$ROOT/UnleashedRecompLib/private/default.xex" \
    "$ROOT/UnleashedRecompLib/private/default.xexp" \
    "$ROOT/UnleashedRecompLib/private/shader.ar"; do
    if [[ ! -f "$file" ]]; then
        printf 'Missing required file: %s\n' "$file" >&2
        missing=1
    fi
done

if [[ "$missing" -ne 0 ]]; then
    printf '\nAdd these files from your own compatible Sonic Unleashed Xbox 360 dump before building an IPA.\n' >&2
    exit 1
fi

cmake --preset macos-release
cmake --build "$HOST_BUILD_DIR" --target XenonRecomp XenosRecomp x_decompress file_to_c -j "${JOBS:-8}"

cmake --preset ios-device-release
cmake --build "$BUILD_DIR" --target UnleashedRecomp -j "${JOBS:-8}"

if [[ ! -d "$APP_PATH" ]]; then
    printf 'Expected app bundle was not produced: %s\n' "$APP_PATH" >&2
    exit 1
fi

rm -rf "$IPA_DIR"
mkdir -p "$PAYLOAD_DIR"
cp -R "$APP_PATH" "$PAYLOAD_DIR/"

if [[ -z "${CODESIGN_IDENTITY:-}" ]] && security find-identity -v -p codesigning | grep -q "$DEFAULT_SIGNING_IDENTITY"; then
    CODESIGN_IDENTITY="$DEFAULT_SIGNING_IDENTITY"
fi

if [[ -n "${MOBILEPROVISION:-}" && ! -f "${MOBILEPROVISION:-}" ]]; then
    printf 'Provisioning profile not found: %s\n' "$MOBILEPROVISION" >&2
    exit 1
fi

if [[ -n "${CODESIGN_IDENTITY:-}" ]]; then
    if [[ -z "${MOBILEPROVISION:-}" ]]; then
        printf 'CODESIGN_IDENTITY is set, but MOBILEPROVISION is not. Producing an unsigned IPA for sideloading tools that sign on import.\n' >&2
    else
        cp "$MOBILEPROVISION" "$PAYLOAD_DIR/Unleashed Recompiled.app/embedded.mobileprovision"
    fi
fi

if [[ -n "${CODESIGN_IDENTITY:-}" && -n "${MOBILEPROVISION:-}" ]]; then
    codesign --force --sign "$CODESIGN_IDENTITY" \
        ${ENTITLEMENTS:+--entitlements "$ENTITLEMENTS"} \
        "$PAYLOAD_DIR/Unleashed Recompiled.app"
fi

(
    cd "$IPA_DIR"
    zip -qry "$IPA_PATH" Payload
)

printf '%s\n' "$IPA_PATH"
