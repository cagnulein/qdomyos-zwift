#!/bin/bash
# =============================================================================
# check-qml-deps.sh — QML Runtime Dependency Checker
#
# Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
# License: GPL-3.0
#
# Extracts QML import statements from the binary, maps each to its system
# directory and apt package, and reports anything missing.
#
# Usage:
#   ./check-qml-deps.sh [--binary <path>] [--fix]
#
# Exit codes:
#   0  All QML dependencies satisfied
#   1  One or more dependencies missing
#   2  Binary not found or strings(1) unavailable
#
# Options:
#   --binary <path>   Path to qdomyos-zwift-bin (default: ./qdomyos-zwift-bin)
#   --fix             Print the apt install command to resolve missing deps
# =============================================================================

set -uo pipefail

# ---------------------------------------------------------------------------
# Import → "qml_subdir|apt_package" lookup table
# Key   = module name as it appears in "import X" statements
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
)

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
BINARY="${1:-}"
FIX_MODE=false

while [[ $# -gt 0 ]]; do
    case "$1" in
        --binary) BINARY="$2"; shift 2 ;;
        --fix)    FIX_MODE=true; shift ;;
        *)        shift ;;
    esac
done

# Default binary location: same directory as this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BINARY="${BINARY:-$SCRIPT_DIR/qdomyos-zwift-bin}"

# ---------------------------------------------------------------------------
# Pre-flight
# ---------------------------------------------------------------------------
if [[ ! -f "$BINARY" ]]; then
    echo "ERROR: Binary not found: $BINARY" >&2
    exit 2
fi

if ! command -v strings >/dev/null 2>&1; then
    echo "ERROR: strings(1) not available - install binutils" >&2
    exit 2
fi

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
# Extract imports from binary and check each one
# ---------------------------------------------------------------------------
MISSING_PKGS=()
MISSING_DIRS=()

# Pull unique module names from the binary
# "import QtFoo.Bar 1.0" → "QtFoo.Bar"
while IFS= read -r module; do
    [[ -z "$module" ]] && continue
    # Skip bundled C++ plugins — not system packages
    [[ "$module" == "AndroidStatusBar" ]] && continue
    [[ "$module" == org.cagnulein.* ]] && continue

    if [[ -v "QML_MAP[$module]" ]]; then
        qml_dir="${QML_MAP[$module]%%|*}"
        apt_pkg="${QML_MAP[$module]##*|}"
        if ! qml_dir_exists "$qml_dir"; then
            MISSING_PKGS+=("$apt_pkg")
            MISSING_DIRS+=("$qml_dir")
        fi
    fi
done < <(strings "$BINARY" | grep -E "^import Qt" | awk '{print $2}' | sort -u)

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