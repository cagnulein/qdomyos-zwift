#!/bin/bash
# =============================================================================
# check-qml-deps.sh — QML Runtime Dependency Checker
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# License: GPL-3.0
#
# Checks all required QML modules against the system filesystem and reports
# anything missing, with optional apt install command to fix.
#
# Usage:
#   ./check-qml-deps.sh [--fix]
#
# Exit codes:
#   0  All QML dependencies satisfied
#   1  One or more dependencies missing
#
# Options:
#   --fix    Print the apt install command to resolve missing deps
# =============================================================================

set -uo pipefail

# ---------------------------------------------------------------------------
# Import → "qml_subdir|apt_package" lookup table
# Key   = QML module name
# Value = pipe-delimited: relative path under qt5/qml/ | apt package name
#
# To add a new dependency: one line here is all that's needed.
# ---------------------------------------------------------------------------
declare -A QML_MAP=(
    ["QtQuick"]="QtQuick.2|qml-module-qtquick2"
    ["QtQuick.Controls"]="QtQuick/Controls|qml-module-qtquick-controls"
    ["QtQuick.Controls.Material"]="QtQuick/Controls.2|qml-module-qtquick-controls2"
    ["QtQuick.Dialogs"]="QtQuick/Dialogs|qml-module-qtquick-dialogs"
    ["QtQuick.Layouts"]="QtQuick/Layouts|qml-module-qtquick-layouts"
    ["QtQuick.Window"]="QtQuick/Window.2|qml-module-qtquick-window2"
    ["QtGraphicalEffects"]="QtGraphicalEffects|qml-module-qtgraphicaleffects"
    ["QtMultimedia"]="QtMultimedia|qml-module-qtmultimedia"
    ["QtLocation"]="QtLocation|qml-module-qtlocation"
    ["QtPositioning"]="QtPositioning|qml-module-qtpositioning"
    ["QtWebView"]="QtWebView|qml-module-qtwebview"
    ["Qt.labs.platform"]="Qt/labs/platform|qml-module-qt-labs-platform"
    ["Qt.labs.settings"]="Qt/labs/settings|qml-module-qt-labs-settings"
    ["Qt.labs.calendar"]="Qt/labs/calendar|qml-module-qt-labs-calendar"
    ["Qt.labs.folderlistmodel"]="Qt/labs/folderlistmodel|qml-module-qt-labs-folderlistmodel"
    ["QtCharts"]="QtCharts|qml-module-qtcharts"
)

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
FIX_MODE=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --binary) shift 2 ;;   # accepted but ignored (no longer used)
        --fix)    FIX_MODE=true; shift ;;
        *)        shift ;;
    esac
done

# ---------------------------------------------------------------------------
# Detect architecture QML base path (mirrors setup-dashboard.sh logic)
# ---------------------------------------------------------------------------
ARCH_LIB_PATH=""
case "$(uname -m)" in
    x86_64|amd64)   ARCH_LIB_PATH="x86_64-linux-gnu" ;;
    aarch64|arm64)  ARCH_LIB_PATH="aarch64-linux-gnu" ;;
    *)              ARCH_LIB_PATH="" ;;
esac

qml_dir_exists() {
    local subdir="$1"
    # Check arch-specific path first, then generic fallback
    [[ -n "$ARCH_LIB_PATH" && -d "/usr/lib/$ARCH_LIB_PATH/qt5/qml/$subdir" ]] && return 0
    [[ -d "/usr/lib/qt5/qml/$subdir" ]] && return 0
    return 1
}

# ---------------------------------------------------------------------------
# Modules actually used by this build — populated at build time by the
# Dockerfile scanning the QML source files. When non-empty, only these
# modules are checked; when empty (script not yet patched), every entry
# in QML_MAP is checked as a safe fallback.
#
# Dockerfile inject command (run after make, before artifact copy):
#   mods=$(grep -rh "^import Qt" src/*.qml | awk '{print $2}' | sort -u | tr '\n' ' ')
#   sed -i "s/REQUIRED_MODULES=()/REQUIRED_MODULES=($mods)/" src/devices/antlinux/check-qml-deps.sh
# ---------------------------------------------------------------------------
REQUIRED_MODULES=()

# ---------------------------------------------------------------------------
# Check each required module. Falls back to all QML_MAP entries when
# REQUIRED_MODULES is empty (i.e. on an unpatched / dev copy of the script).
# ---------------------------------------------------------------------------
MISSING_PKGS=()
MISSING_DIRS=()

if [[ ${#REQUIRED_MODULES[@]} -gt 0 ]]; then
    modules_to_check=("${REQUIRED_MODULES[@]}")
else
    modules_to_check=("${!QML_MAP[@]}")
fi

for module in "${modules_to_check[@]}"; do
    if [[ ! -v "QML_MAP[$module]" ]]; then
        continue   # module used in QML but not yet in the translation map — skip silently
    fi
    qml_dir="${QML_MAP[$module]%%|*}"
    apt_pkg="${QML_MAP[$module]##*|}"
    if ! qml_dir_exists "$qml_dir"; then
        MISSING_PKGS+=("$apt_pkg")
        MISSING_DIRS+=("$qml_dir")
    fi
done

# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------
if [[ ${#MISSING_PKGS[@]} -eq 0 ]]; then
    exit 0
fi

echo "Missing QML modules:" >&2
for i in "${!MISSING_PKGS[@]}"; do
    echo "  ✗ ${MISSING_DIRS[$i]}  (${MISSING_PKGS[$i]})" >&2
done

if [[ "$FIX_MODE" == "true" ]]; then
    echo "" >&2
    echo "Run to fix:" >&2
    echo "  sudo apt-get install -y ${MISSING_PKGS[*]}" >&2
fi

exit 1