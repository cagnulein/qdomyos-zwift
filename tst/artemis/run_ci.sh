#!/usr/bin/env bash
set -euo pipefail

QZ_PACKAGE="${QZ_ANDROID_PACKAGE:-org.cagnulen.qdomyoszwift}"
ARTEMIS_DIR="${ARTEMIS_DIR:-${GITHUB_WORKSPACE:-$PWD}/.artemis}"
ARTEMIS_TRACES_PATH="${ARTEMIS_TRACES_PATH:-${GITHUB_WORKSPACE:-$PWD}/artemis-traces}"

if [[ -z "${GEMINI_API_KEY:-${GOOGLE_API_KEY:-}}" ]]; then
    echo "::notice::Artemis UI tests skipped because ARTEMIS_GEMINI_API_KEY/GEMINI_API_KEY is not available."
    exit 0
fi

if [[ ! -d "$ARTEMIS_DIR" ]]; then
    echo "::error::Artemis checkout not found at $ARTEMIS_DIR"
    exit 1
fi

if ! command -v adb >/dev/null 2>&1; then
    echo "::error::adb is not available"
    exit 1
fi

DEVICE_SERIAL="${ARTEMIS_DEVICE_SERIAL:-${ADB_DEVICE_SERIAL:-}}"
if [[ -z "$DEVICE_SERIAL" ]]; then
    DEVICE_SERIAL="$(adb devices | awk 'NR > 1 && $2 == "device" { print $1; exit }')"
fi
if [[ -z "$DEVICE_SERIAL" ]]; then
    echo "::error::No online Android device/emulator found"
    adb devices -l || true
    exit 1
fi

mkdir -p "$ARTEMIS_TRACES_PATH"

echo "Running Artemis QZ tests on $DEVICE_SERIAL"
adb -s "$DEVICE_SERIAL" shell am start -n "$QZ_PACKAGE/$QZ_PACKAGE.CustomQtActivity" >/dev/null
sleep 3

run_artemis() {
    local name="$1"
    local profile="$2"
    local goal="$3"
    local expected="$4"

    echo "::group::Artemis: $name"
    (
        cd "$ARTEMIS_DIR"
        uv run artemis run "$goal" \
            --standalone \
            --device-serial "$DEVICE_SERIAL" \
            --locked-app "$QZ_PACKAGE" \
            --profile "$profile" \
            --test-name "$name" \
            --traces-path "$ARTEMIS_TRACES_PATH" \
            --output-description "$expected" \
            --disable-committee
    )
    echo "::endgroup::"
}

run_artemis \
    "qz-ftms-search-consistency" \
    "pro" \
    'Open QZ Fitness and navigate to Settings. Find "FTMS Bike" in the normal settings hierarchy and remember the exact displayed value without changing it. Open the Settings search, search for "ftms", find "FTMS Bike" in the filtered results, and verify that the displayed value is exactly the same as in the normal Settings view. Fail if the two values differ, if the search result is missing, or if QZ crashes or becomes unresponsive. If the normal value is Disabled, the values must still match; report that this run validates consistency but cannot exercise the saved-non-default-device variant.' \
    'PASS only when FTMS Bike has exactly the same displayed value before and after filtering Settings with "ftms", and QZ stays responsive.'

run_artemis \
    "qz-experimental-persistence" \
    "pro" \
    'Open QZ Fitness and navigate to Settings > Experimental Features. Locate "Android Notification" and "Fake Device". Enable each setting if it is disabled. Return to the QZ main screen, reopen Settings > Experimental Features, and verify that both settings are still enabled. Fail if either setting does not persist, cannot be found, or QZ crashes or becomes unresponsive.' \
    'PASS only when Android Notification and Fake Device are both enabled after leaving Settings and reopening it, with no crash or ANR.'

run_artemis \
    "qz-settings-smoke" \
    "flash" \
    'Run a QZ Fitness Settings smoke test without changing any additional settings: start from the main screen, open Settings, open Settings search, search for "ftms" and verify that at least one relevant result appears, clear the search, open Experimental Features, scroll through that section, then return to the QZ main screen. Fail on any Android crash dialog, ANR, blank page, unrecoverable navigation error, or loss of responsiveness.' \
    'PASS only when the Settings/search/Experimental Features navigation completes and QZ remains responsive with no crash or ANR.'

echo "All Artemis QZ UI tasks completed."
