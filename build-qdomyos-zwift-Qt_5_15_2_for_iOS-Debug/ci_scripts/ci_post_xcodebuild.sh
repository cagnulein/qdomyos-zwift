#!/bin/bash
set -euo pipefail

# Requires secret App Store Connect workflow variables:
#   ASC_ISSUER_ID, ASC_KEY_ID, ASC_KEY_P8_BASE64
# The helper also accepts APP_STORE_CONNECT_API_* aliases.
# Keep the .p8 value base64-encoded and mark all three variables as Secret.

# Xcode Cloud runs this after every xcodebuild action. The helper exits safely
# for non-archive actions and failed archives.
if [[ "${CI_XCODE_CLOUD:-${CI:-}}" != "TRUE" && "${CI_XCODE_CLOUD:-${CI:-}}" != "true" ]]; then
    echo "[beta-review] Not running in Xcode Cloud; skipping"
    exit 0
fi

if [[ "${CI_XCODEBUILD_ACTION:-}" != "archive" ]]; then
    echo "[beta-review] Not an archive action; skipping"
    exit 0
fi

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec python3 "$SCRIPT_DIR/submit_beta_review.py"
