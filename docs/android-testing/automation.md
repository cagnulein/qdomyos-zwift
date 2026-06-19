# Android Test Automation - adb and UI Automator

## Environment Setup

```bash
# Add adb to PATH on macOS.
export ADB=~/Library/Android/sdk/platform-tools/adb
export EMULATOR=~/Library/Android/sdk/emulator/emulator

# Available AVDs.
$EMULATOR -list-avds
# Pixel_8_API_36

# Start the emulator.
$EMULATOR -avd Pixel_8_API_36 -no-snapshot-load &

# Wait for boot.
$ADB wait-for-device shell getprop sys.boot_completed
# "1"

# List connected devices, including emulators and physical devices.
$ADB devices
```

## Screenshots with adb

```bash
ADB=~/Library/Android/sdk/platform-tools/adb

take_screenshot() {
  local out="${1:-/tmp/screen.png}"
  $ADB shell screencap -p /sdcard/screen.png
  $ADB pull /sdcard/screen.png "$out"
  # Resize for viewing on macOS.
  sips -Z 540 "$out" --out "${out%.png}_small.png" 2>/dev/null
}

take_screenshot /tmp/qz_test.png
```

## UI Automator Dump

```bash
# Dump the current UI hierarchy.
dump_ui() {
  local out="${1:-/tmp/ui.xml}"
  $ADB shell uiautomator dump /sdcard/ui.xml
  $ADB pull /sdcard/ui.xml "$out"
}

# Find all checkable elements with their state.
check_toggles() {
  dump_ui /tmp/ui.xml
  python3 -c "
import xml.etree.ElementTree as ET
tree = ET.parse('/tmp/ui.xml')
root = tree.getroot()
def find_all(node):
    desc = node.get('content-desc', '')
    if node.get('checkable') == 'true' and desc and node.get('bounds') != '[0,0][0,0]':
        print(f'  [{\"ON\" if node.get(\"checked\")==\"true\" else \"OFF\"}] {desc[:60]}  {node.get(\"bounds\")}')
    for child in node:
        find_all(child)
find_all(root)
"
}
```

## QZ Navigation

QZ uses Qt Quick (QML). The accessibility tree shows elements such as `android.view.View$VirtualChild`.

### Open the Drawer and Settings

```bash
# Open the side menu: tap the left arrow in the top-left corner.
$ADB shell input tap 78 193
sleep 1

# Tap "Settings" in the drawer.
$ADB shell input tap 175 300
sleep 1
```

### Scroll Settings and Open Experimental Features

```bash
# Scroll up 3 times to reach the bottom of the settings list.
for i in 1 2 3; do
  $ADB shell input swipe 540 1500 540 500 400
  sleep 0.3
done

# Expand "Experimental Features" by tapping the vertical arrow button on the right.
$ADB shell input tap 976 2290
sleep 1

# Scroll down 4 times to show all experimental toggles.
for i in 1 2 3 4; do
  $ADB shell input swipe 540 1800 540 800 400
  sleep 0.3
done
```

### Tap a Toggle Precisely

Experimental settings items are not always marked as `clickable=true` in the accessibility tree, but they still respond to touch. Use the coordinates from the element bounds:

```bash
# Element: "Android Notification" -> bounds=[0,1662][1078,1787]
# Center: x=(0+1078)/2=539, y=(1662+1787)/2=1724
$ADB shell input tap 539 1724

# Element: "Fake Device" -> bounds=[0,394][1078,519]
$ADB shell input tap 539 456
```

**Note:** bounds change depending on scroll position. Use `dump_ui` to verify the current bounds.

## Edit the QZ Configuration Directly

The most reliable way to set toggles is to write the Qt INI file directly while QZ is stopped:

```bash
set_qz_setting() {
  local key="$1"
  local value="$2"
  local conf_path="/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola/qDomyos-Zwift.conf"

  $ADB shell am force-stop org.cagnulen.qdomyoszwift
  sleep 1

  # Read the current configuration.
  $ADB shell "run-as org.cagnulen.qdomyoszwift sh -c 'cat \"$conf_path\" 2>/dev/null'" > /tmp/qz_cfg.conf

  if grep -q "^$key=" /tmp/qz_cfg.conf; then
    # Update an existing value.
    sed -i '' "s/^$key=.*/$key=$value/" /tmp/qz_cfg.conf
  elif grep -q "^\[General\]" /tmp/qz_cfg.conf; then
    # Add the value under [General].
    sed -i '' "/^\[General\]/a\\
$key=$value" /tmp/qz_cfg.conf
  else
    # Empty file or no [General] section.
    printf "[General]\n$key=$value\n" > /tmp/qz_cfg.conf
  fi

  # Write the updated file.
  local content=$(cat /tmp/qz_cfg.conf)
  $ADB shell "run-as org.cagnulen.qdomyoszwift sh -c '
    mkdir -p \"/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola\"
    printf \"%s\n\" \"$(echo "$content" | sed "s/\"/\\\\\"/g")\" \
      > \"$conf_path\"
  '"
}

# Examples.
set_qz_setting android_notification true
set_qz_setting applewatch_fakedevice true
```

## Verify Real-Time Data

```bash
# Show Bluetooth data sent by QZ (FTMS).
$ADB shell dumpsys bluetooth_manager | grep -i "ftms\|fitness"

# Show active notifications.
$ADB shell dumpsys notification | grep -A10 "qdomyos"

# Check whether Zwift is connected.
$ADB shell dumpsys activity | grep "com.zwift.android.prod" | head -5
```

## Read logcat

```bash
# Live QZ logs.
$ADB logcat | grep -i "qdomyos\|QZFitness\|bluetooth"

# Zwift logs.
$ADB logcat -s "ZwiftApp"

# Save logs for analysis.
$ADB logcat -d > /tmp/android_log.txt
```

## Full Automatic Test Startup (QZ + Zwift on the Same Device)

```bash
#!/bin/bash
ADB=~/Library/Android/sdk/platform-tools/adb

# 1. Configure QZ.
echo "Configuring QZ..."
$ADB shell am force-stop org.cagnulen.qdomyoszwift
$ADB shell "run-as org.cagnulen.qdomyoszwift sh -c '
  mkdir -p \"/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola\"
  printf \"[General]\nandroid_notification=true\napplewatch_fakedevice=true\n\" \
    > \"/data/data/org.cagnulen.qdomyoszwift/files/.config/Roberto Viola/qDomyos-Zwift.conf\"
'"

# 2. Start QZ.
echo "Starting QZ..."
$ADB shell am start -n org.cagnulen.qdomyoszwift/.CustomQtActivity
sleep 6

# 3. Verify the foreground notification.
POSTED=$($ADB shell dumpsys notification | grep -A5 "qdomyos" | grep "numPostedByApp" | grep -o '[0-9]*' | head -1)
if [ "$POSTED" -gt 0 ] 2>/dev/null; then
  echo "QZ foreground notification is active"
else
  echo "Notification not found; check the configuration"
  exit 1
fi

# 4. Start Zwift.
echo "Starting Zwift..."
$ADB shell monkey -p com.zwift.android.prod -c android.intent.category.LAUNCHER 1
echo "Done. Go to the sensor pairing screen in Zwift."
```
