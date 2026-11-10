#!/usr/bin/env bash
# Smoke-test the *packaged* macOS app on its native CPU, not the build-tree executable.
set -euo pipefail

dmg="${1:?Usage: macos_intel_dmg_smoke_test.sh <dmg> [evidence-dir] [x86_64|arm64]}"
evidence_dir="${2:-$PWD/macos-test-evidence}"
expected_arch="${3:-x86_64}"
case "$expected_arch" in
  x86_64|arm64) ;;
  *) echo "Unsupported macOS CPU architecture: $expected_arch" >&2; exit 1 ;;
esac
mkdir -p "$evidence_dir"
evidence_dir="$(cd "$evidence_dir" && pwd)"
dmg="$(cd "$(dirname "$dmg")" && pwd)/$(basename "$dmg")"
mount_point="$(mktemp -d "${RUNNER_TEMP:-/tmp}/qz-macos-dmg.XXXXXX")"
mounted=false
app_pid=""

cleanup() {
  if [[ -n "$app_pid" ]] && kill -0 "$app_pid" 2>/dev/null; then
    kill "$app_pid" 2>/dev/null || true
  fi
  if [[ "$mounted" == true ]]; then
    hdiutil detach "$mount_point" -quiet || hdiutil detach "$mount_point" -force -quiet || true
  fi
  rmdir "$mount_point" 2>/dev/null || true
}
trap cleanup EXIT

hdiutil verify "$dmg" > "$evidence_dir/dmg-verify.log" 2>&1
hdiutil attach -readonly -nobrowse -mountpoint "$mount_point" "$dmg" > "$evidence_dir/dmg-mount.log"
mounted=true

app="$(find "$mount_point" -maxdepth 1 -name '*.app' -type d -print -quit)"
if [[ -z "$app" ]]; then
  echo "No .app bundle in the DMG" >&2
  exit 1
fi
# Validate launch metadata as well as the binary: Finder relies on
# CFBundleExecutable, so simply locating Contents/MacOS is not enough.
plist="$app/Contents/Info.plist"
plutil -lint "$plist" | tee "$evidence_dir/info-plist-validation.log"
if ! executable_name="$(/usr/libexec/PlistBuddy -c 'Print CFBundleExecutable' "$plist")"; then
  echo "DMG app is not Finder-launchable: CFBundleExecutable is missing" >&2
  exit 1
fi
binary="$app/Contents/MacOS/$executable_name"
if [[ ! -x "$binary" ]]; then
  echo "CFBundleExecutable does not point to an executable: $binary" >&2
  exit 1
fi
lipo "$binary" -verify_arch "$expected_arch"

httpserver="$app/Contents/Frameworks/QtHttpServer.framework/Versions/5/QtHttpServer"
if [[ ! -f "$httpserver" ]]; then
  echo "QtHttpServer is not bundled in the app" >&2
  exit 1
fi
lipo "$httpserver" -verify_arch "$expected_arch"
file "$httpserver" | tee "$evidence_dir/httpserver-architecture.log"
file "$binary" | tee "$evidence_dir/architecture.log"
if [[ "$(uname -m)" != "$expected_arch" ]]; then
  echo "Expected native macOS runner with architecture $expected_arch" >&2
  exit 1
fi

# This exercises QApplication and dynamically deployed Qt frameworks.
"$binary" -smoke-test > "$evidence_dir/qt-smoke.log" 2>&1
grep -Fq 'SMOKE_OK' "$evidence_dir/qt-smoke.log"

# This also creates the actual QML root window on the macOS runner. The CI
# environment variable makes QZ emit a ready marker and quit cleanly after 5s.
# Print the plugin loader's *actual* dlopen failure rather than just Qt's
# generic "Could not load the Qt platform plugin cocoa" message.
cocoa_plugin="$app/Contents/PlugIns/platforms/libqcocoa.dylib"
if [[ -f "$cocoa_plugin" ]]; then
  file "$cocoa_plugin" | tee "$evidence_dir/cocoa-architecture.log"
  otool -L "$cocoa_plugin" > "$evidence_dir/cocoa-dependencies.log" 2>&1
  codesign -dv --verbose=2 "$cocoa_plugin" > "$evidence_dir/cocoa-signature.log" 2>&1 || true
fi
env QT_DEBUG_PLUGINS=1 QZ_CI_GUI_SMOKE_TEST=1 "$binary" -qml -no-virtual-device-bluetooth > "$evidence_dir/gui-smoke.log" 2>&1 &
app_pid="$!"
for _ in $(seq 1 45); do
  if ! kill -0 "$app_pid" 2>/dev/null; then
    break
  fi
  sleep 2
done

if kill -0 "$app_pid" 2>/dev/null; then
  echo "QZ did not exit after the GUI smoke test" >&2
  exit 1
fi
if ! wait "$app_pid"; then
  echo "QZ exited with an error during GUI startup" >&2
  cat "$evidence_dir/gui-smoke.log"
  exit 1
fi
app_pid=""
if ! grep -Fq 'QZ_QML_LOADED' "$evidence_dir/gui-smoke.log"; then
  echo "QZ never confirmed that main.qml loaded" >&2
  cat "$evidence_dir/gui-smoke.log"
  exit 1
fi
echo "macOS $expected_arch DMG smoke tests passed (Qt, native architecture, and QML startup)."
