#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/out/build/ios-device-release"
IPA_DIR="$ROOT/out/ipa"
PAYLOAD_DIR="$IPA_DIR/Payload"
APP_PATH="$BUILD_DIR/Unleashed Recompiled.app"
IPA_PATH="$IPA_DIR/UnleashedRecompiled.ipa"

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
    exit 1
fi

cmake --preset ios-device-release
cmake --build "$BUILD_DIR" --target UnleashedRecomp -j "${JOBS:-8}"

if [[ ! -d "$APP_PATH" ]]; then
    printf 'Expected app bundle was not produced: %s\n' "$APP_PATH" >&2
    exit 1
fi

rm -rf "$IPA_DIR"
mkdir -p "$PAYLOAD_DIR"
cp -R "$APP_PATH" "$PAYLOAD_DIR/"

if [[ -n "${MOBILEPROVISION:-}" ]]; then
    cp "$MOBILEPROVISION" "$PAYLOAD_DIR/Unleashed Recompiled.app/embedded.mobileprovision"
fi

if [[ -n "${CODESIGN_IDENTITY:-}" ]]; then
    codesign --force --sign "$CODESIGN_IDENTITY" \
        ${ENTITLEMENTS:+--entitlements "$ENTITLEMENTS"} \
        "$PAYLOAD_DIR/Unleashed Recompiled.app"
fi

(
    cd "$IPA_DIR"
    zip -qry "$IPA_PATH" Payload
)

printf '%s\n' "$IPA_PATH"
