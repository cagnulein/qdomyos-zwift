#!/usr/bin/env bash
set -euo pipefail

if [[ "${OSTYPE:-}" != darwin* ]]; then
    echo "This script must run on macOS" >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

QT_IOS_DIR="${QT_IOS_DIR:-/Users/cagnulein/Qt/5.15.2/ios}"
QTBASE_SOURCE_DIR="${QTBASE_SOURCE_DIR:-/tmp/qtbase-source-5.15.2-qz-backport}"
APPLY_PATCHES="${APPLY_PATCHES:-0}"
BUILD_CONFIG="${BUILD_CONFIG:-release}"
OUTPUT_DIR="${OUTPUT_DIR:-$PROJECT_ROOT/qt-patches/ios/5.15.2/binary}"
BUILD_DIR="$(mktemp -d /tmp/qz-qios-build.XXXXXX)"
SOURCE_OVERLAY_DIR=""

if [[ ! -x "$QT_IOS_DIR/bin/qmake" ]]; then
    echo "qmake not found in $QT_IOS_DIR" >&2
    exit 1
fi
if [[ ! -f "$QTBASE_SOURCE_DIR/src/plugins/platforms/ios/qiosapplicationdelegate.mm" ]]; then
    echo "Qt base source not found in $QTBASE_SOURCE_DIR" >&2
    exit 1
fi

if [[ "$APPLY_PATCHES" == "1" ]]; then
    SOURCE_OVERLAY_DIR="$(mktemp -d /tmp/qz-qtbase-overlay.XXXXXX)"
    cp -R "$QTBASE_SOURCE_DIR"/. "$SOURCE_OVERLAY_DIR"/
    patch --batch --forward --silent -p1 -d "$SOURCE_OVERLAY_DIR" \
        < "$PROJECT_ROOT/qt-patches/ios/5.15.2/qios-uiscene.patch"
    patch --batch --forward --silent -p1 -d "$SOURCE_OVERLAY_DIR" \
        < "$PROJECT_ROOT/qt-patches/ios/5.15.2/qios-uiscene-layout.patch"
    QTBASE_SOURCE_DIR="$SOURCE_OVERLAY_DIR"
    echo "Building from temporary Qt source overlay: $SOURCE_OVERLAY_DIR"
fi

case "$BUILD_CONFIG" in
    release)
        CONFIG_ARGS=(CONFIG+=release CONFIG-=debug)
        OUTPUT_NAME="libqios.a"
        ;;
    debug)
        CONFIG_ARGS=(CONFIG+=debug CONFIG-=release)
        OUTPUT_NAME="libqios_debug.a"
        ;;
    *)
        echo "BUILD_CONFIG must be release or debug" >&2
        exit 1
        ;;
esac

"$QT_IOS_DIR/bin/qmake" \
    -spec macx-ios-clang \
    "$PROJECT_ROOT/qt-patches/ios/5.15.2/qios.pro" \
    -o "$BUILD_DIR/Makefile" \
    "QIOS_SOURCE_DIR=$QTBASE_SOURCE_DIR/src/plugins/platforms/ios" \
    "QMAKE_XCODE_DEVELOPER_PATH=/Applications/Xcode.app/Contents/Developer" \
    "QMAKE_IOS_DEPLOYMENT_TARGET=12.0" \
    "QMAKE_APPLE_DEVICE_ARCHS=arm64 x86_64" \
    "${CONFIG_ARGS[@]}"

make -C "$BUILD_DIR" -j"$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"

mkdir -p "$OUTPUT_DIR"
cp "$BUILD_DIR/libqios.a" "$OUTPUT_DIR/$OUTPUT_NAME"
echo "Created $OUTPUT_DIR/$OUTPUT_NAME"
