#!/usr/bin/env bash
#
# Package the local build into an AppImage (linuxdeploy + gtk plugin + appimagetool).
#
# Usage: ./make-appimage.sh [preset]        (preset passed to build.sh)
#
# Env: REBUILD=1  OUTDIR=<dir>  UPDATE_INFORMATION=<zsync string>
#
# The AppImage reads game data (game/ update/ dlc/) from, in order:
#   $UR_DATA_DIR  |  $PWD if it has game//portable.txt  |  ~/.local/share/UnleashedRecomp
# Config and saves stay in ~/.config/UnleashedRecomp.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

PRESET="${1:-linux-release}"
ARCH="$(uname -m)"
APPID="io.github.hedge_dev.unleashedrecomp"
BUILD_DIR="out/build/${PRESET}"
BIN="${BUILD_DIR}/UnleashedRecomp/UnleashedRecomp"
APPDIR="out/AppDir"
TOOLS="out/appimage-tools"
OUTDIR="${OUTDIR:-dist}"

log() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }

for cmd in curl git sed install; do
    command -v "$cmd" >/dev/null || { echo "missing required tool: $cmd" >&2; exit 1; }
done

if [[ ! -x "$BIN" || "${REBUILD:-0}" == "1" ]]; then
    log "Building UnleashedRecomp ($PRESET)"
    ./build.sh "$PRESET"
fi

# Fetch tooling (cached in $TOOLS).
mkdir -p "$TOOLS"
fetch() { # url dest
    if [[ ! -f "$2" ]]; then
        log "Downloading $(basename "$2")"
        curl -fL --retry 5 --retry-connrefused -o "$2" "$1"
        chmod +x "$2"
    fi
}
fetch "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage" "$TOOLS/linuxdeploy"
fetch "https://raw.githubusercontent.com/linuxdeploy/linuxdeploy-plugin-gtk/master/linuxdeploy-plugin-gtk.sh" "$TOOLS/linuxdeploy-plugin-gtk.sh"
fetch "https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${ARCH}.AppImage" "$TOOLS/appimagetool"

export PATH="$REPO_ROOT/$TOOLS:$PATH"
export APPIMAGE_EXTRACT_AND_RUN=1   # run the tool AppImages without FUSE

log "Assembling AppDir"
rm -rf "$APPDIR"
install -Dm755 "$BIN" "$APPDIR/usr/bin/UnleashedRecomp"
install -Dm644 "UnleashedRecompResources/images/game_icon.png" \
    "$APPDIR/usr/share/icons/hicolor/128x128/apps/${APPID}.png"
[[ -f "flatpak/${APPID}.metainfo.xml" ]] && install -Dm644 "flatpak/${APPID}.metainfo.xml" \
    "$APPDIR/usr/share/metainfo/${APPID}.metainfo.xml"

# Upstream .desktop, minus the Flatpak-absolute Exec path.
mkdir -p "$APPDIR/usr/share/applications"
sed 's|^Exec=.*|Exec=UnleashedRecomp|' "flatpak/${APPID}.desktop" \
    > "$APPDIR/usr/share/applications/${APPID}.desktop"

# Custom AppRun: run from a writable data dir with --use-cwd so the game
# doesn't chdir into the read-only AppImage mount.
APPRUN="out/AppRun"
cat > "$APPRUN" <<'EOF'
#!/bin/bash
set -e
APPDIR="$(dirname "$(readlink -f "$0")")"
export APPDIR

# GTK env from the gtk plugin's hooks (skip if the outer AppRun already ran them).
if [ -z "${GDK_PIXBUF_MODULE_FILE:-}" ]; then
    for hook in "$APPDIR"/apprun-hooks/*.sh; do
        [ -e "$hook" ] && . "$hook"
    done
fi

export LD_LIBRARY_PATH="$APPDIR/usr/lib:${LD_LIBRARY_PATH:-}"
export PATH="$APPDIR/usr/bin:$PATH"

if [ -n "${UR_DATA_DIR:-}" ]; then
    data_dir="$UR_DATA_DIR"
elif [ -d "$PWD/game" ] || [ -e "$PWD/portable.txt" ]; then
    data_dir="$PWD"
else
    data_dir="${XDG_DATA_HOME:-$HOME/.local/share}/UnleashedRecomp"
fi
mkdir -p "$data_dir"
cd "$data_dir"

exec "$APPDIR/usr/bin/UnleashedRecomp" --use-cwd "$@"
EOF
chmod +x "$APPRUN"

VERSION="$(git describe --tags --always 2>/dev/null || echo dev)"
export LINUXDEPLOY_OUTPUT_VERSION="$VERSION"
export DEPLOY_GTK_VERSION=3
[[ -n "${UPDATE_INFORMATION:-}" ]] && export LDAI_UPDATE_INFORMATION="$UPDATE_INFORMATION"

log "Running linuxdeploy (version $VERSION)"
"$TOOLS/linuxdeploy" \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/UnleashedRecomp" \
    --desktop-file "$APPDIR/usr/share/applications/${APPID}.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/128x128/apps/${APPID}.png" \
    --library /usr/lib/libasound.so.2 \
    --plugin gtk \
    --custom-apprun "$APPRUN" \
    --output appimage

mkdir -p "$OUTDIR"
mv -v ./UnleashedRecomp*-"${ARCH}".AppImage* "$OUTDIR"/ 2>/dev/null || mv -v ./*-"${ARCH}".AppImage* "$OUTDIR"/
log "Done: $(ls -1 "$OUTDIR"/*.AppImage)"
