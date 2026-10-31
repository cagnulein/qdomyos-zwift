#!/bin/bash
set -euo pipefail

LOG_PATH="${1:-ios-test/xcodebuild.log}"
mkdir -p "$(dirname "$LOG_PATH")"

# Remove the WatchKit dependency that this CI setup cannot resolve for iOS builds.
python3 .github/scripts/ios_simulator_project.py \
  build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/qdomyoszwift.xcodeproj/project.pbxproj

# TestFlight needs a signed device archive, which Fastlane builds separately.
if xcodebuild -project src/qdomyoszwift.xcodeproj -scheme qdomyoszwift -configuration Release \
  -sdk iphonesimulator -destination 'generic/platform=iOS Simulator' -derivedDataPath dd \
  ARCHS=x86_64 ONLY_ACTIVE_ARCH=NO CODE_SIGNING_ALLOWED=NO CODE_SIGNING_REQUIRED=NO \
  build > "$LOG_PATH" 2>&1; then
  grep -F "** BUILD" "$LOG_PATH"
else
  grep -F -e "error:" -e "** BUILD" "$LOG_PATH" | tail -n 50 || true
  exit 1
fi
