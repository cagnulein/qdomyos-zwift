#!/usr/bin/env bash
set -euo pipefail

# Test for setup-dashboard.sh config generator
# Verifies that typed setters and generate_config_file() produce expected INI

SCRIPT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TEST_TEMP_DIR=$(mktemp -d /tmp/qz_test_temp.XXXXXX)
export TEMP_DIR="$TEST_TEMP_DIR"
export QZ_NO_MAIN=1

echo "Using TEMP_DIR=$TEMP_DIR"

# Source the dashboard script (guarded by QZ_NO_MAIN so it won't run UI)
source "$SCRIPT_ROOT/src/devices/antlinux/setup-dashboard.sh"

# Ensure arrays are initialized via defaults
initialize_default_config || true

# Set some deterministic test values
config_set_string "filter_device" "TEST_DEVICE"
config_set_int "age" 30
config_set_float "calib_factor" 1.2345
config_set_bool "virtual_device_enabled" false

OUT_CONF="$TEMP_DIR/qDomyos-Zwift.conf"

generate_config_file "$OUT_CONF"

echo "Generated: $OUT_CONF"

if [[ ! -f "$OUT_CONF" ]]; then
    echo "ERROR: Config not generated" >&2
    exit 2
fi

# Verify keys/values
grep -q "^filter_device=TEST_DEVICE$" "$OUT_CONF"
grep -q "^age=30$" "$OUT_CONF"
grep -q "^calib_factor=1.2345$" "$OUT_CONF"
grep -q "^virtual_device_enabled=false$" "$OUT_CONF"

echo "OK: INI contains expected keys"

# Clean up
rm -rf "$TEST_TEMP_DIR"

exit 0
