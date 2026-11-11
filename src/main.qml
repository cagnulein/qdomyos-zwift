import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.12
import Qt.labs.settings 1.0
import QtMultimedia 5.15
import org.cagnulein.qdomyoszwift 1.0
import QtQuick.Window 2.12
import Qt.labs.platform 1.1
import AndroidStatusBar 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visibility: Qt.WindowFullScreen
    visible: true
	 objectName: "stack"
    title: Qt.platform.os === "ios" ? "" : qsTr("qDomyos-Zwift")

    // Force update on orientation change
    property int currentOrientation: Screen.orientation
    onCurrentOrientationChanged: {
        if (Qt.platform.os === "android") {
            console.log("Orientation changed to:", currentOrientation)
            // Force property binding updates by accessing the properties
            var temp = AndroidStatusBar.height + AndroidStatusBar.navigationBarHeight + AndroidStatusBar.leftInset + AndroidStatusBar.rightInset
        }
    }
    
    // Helper functions for cleaner padding calculations
    function getTopPadding() {
        // Add padding for iPadOS multi-window mode (Stage Manager, Split View, Slide Over)
        // to avoid overlap with window control buttons (red/yellow/green)
        // Check both the native detection and window size comparison for reactivity
        if (Qt.platform.os === "ios") {
            var isMultiWindow = (typeof rootItem !== "undefined" && rootItem && rootItem.iPadMultiWindowMode) ||
                                (window.width < Screen.width - 10);  // Window smaller than screen = multi-window
            if (isMultiWindow) {
                return 15;  // Space for window control buttons
            }
        }
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        // AndroidStatusBar.height is always the top inset in the current orientation
        // (getSystemWindowInsets() returns orientation-aware values)
        return AndroidStatusBar.height;
    }

    function getBottomPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        // navigationBarHeight is always the bottom inset in the current orientation
        return AndroidStatusBar.navigationBarHeight;
    }

    function getLeftPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.LandscapeOrientation || Screen.orientation === Qt.InvertedLandscapeOrientation) ?
               (settings.android_landscape_cutout_margin ? AndroidStatusBar.leftInset : AndroidStatusBar.systemBarLeftInset) : 0;
    }

    function getRightPadding() {
        if (Qt.platform.os !== "android" || AndroidStatusBar.apiLevel < 31) return 0;
        return (Screen.orientation === Qt.LandscapeOrientation || Screen.orientation === Qt.InvertedLandscapeOrientation) ?
               (settings.android_landscape_cutout_margin ? AndroidStatusBar.rightInset : AndroidStatusBar.systemBarRightInset) : 0;
    }

    // Side margin for text, after the Material 3 window margins (16 on compact windows
    // under 600 wide, 24 on wider ones) but tighter: 12 and 16. Keeps text clear of
    // rounded screen corners and of the edges covered by protective glass.
    // A property rather than a function, so .ui.qml forms can bind to it.
    readonly property int contentSideMargin: width < 600 ? 12 : 16

    function isConfiguringShortcuts() {
        // Check if a TextField in the shortcuts settings has active focus
        // This prevents global shortcuts from intercepting key presses when configuring them
        var focusItem = window.activeFocusItem;
        if (!focusItem) return false;

        // Walk up the parent hierarchy to check if we're inside settingsShortcutsPane
        var current = focusItem;
        while (current) {
            if (current.objectName === "settingsShortcutsPane") {
                return true;
            }
            current = current.parent;
        }
        return false;
    }

    function shortcutReady(sequence) {
        return Qt.platform.os !== "ios" && settings.shortcuts_enabled && !isConfiguringShortcuts() && String(sequence).length > 0;
    }
    function stripBluetoothDeviceName(deviceName) {
        return deviceName.replace(/ \(\d+%\)$/, "")
    }

    // A Settings object that loads a store without one of its keys snapshots its properties for
    // the write in the middle of the load (Qt.labs.settings): the properties declared after that
    // key still have their defaults, and 500 ms later the defaults are written over their values.
    // The change signal of that key, emitted once the load is over, takes the snapshot again
    // without changing a value
    function retakeSettingsSnapshot(copy) {
        for (var key in copy) {
            var type = typeof copy[key]
            if (type !== "boolean" && type !== "number" && type !== "string")
                continue
            if (key === "objectName" || key === "category" || key === "fileName" || copy.value(key) !== undefined)
                continue
            copy[key + "Changed"]()
            return
        }
    }

    function maybeOpenGymModePopup() {
        if (typeof rootItem === "undefined" || !rootItem) {
            return
        }
        if (settings.gym_mode && !rootItem.hasConnectedDevice() && !gymModePopupDismissed && !popupGymMode.visible) {
            popupGymMode.open()
        }
    }

    signal gpx_open_clicked(url name)
    signal gpxpreview_open_clicked(url name)
    signal profile_open_clicked(url name)
    signal trainprogram_open_clicked(url name)
    signal fitfile_preview_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal gpx_open_other_folder(url name)
    signal trainprogram_preview(url name)
    signal trainprogram_zwo_loaded(string s)
    signal trainprogram_autostart_requested()
    signal fitfile_preview(string s)
    signal gpx_save_clicked()
    signal fit_save_clicked()
    signal refresh_bluetooth_devices_clicked()
    signal strava_connect_clicked()
    signal peloton_connect_clicked()
    signal intervalsicu_connect_clicked()
    signal intervalsicu_download_todays_workout_clicked()
    signal loadSettings(url name)
    signal saveSettings(url name)
    signal deleteSettings(url name)
    signal restoreSettings()
    signal saveProfile(string profilename)
    signal restart()
    // Sent by the settings page when a key of the home page copy changes (Home.qml)
    signal tileSettingChanged(string key, var value)
    signal volumeUp()
    signal volumeDown()
    signal keyMediaPrevious()
    signal keyMediaNext()
    signal floatingOpen()
    signal openFloatingWindowBrowser();
    signal strava_upload_file_prepare();

    property bool lockTiles: false
    property bool settings_restart_to_apply: false
    property bool gymModePopupDismissed: false

    // rootItem comes from homeform, created only after this file is loaded (main.cpp): for about
    // a second the bindings on it fail and keep their defaults - every header button shown in a
    // squashed bar. The header and the page wait for it
    readonly property bool rootReady: typeof rootItem !== "undefined" && rootItem !== null

    Settings {
        id: settings
        Component.onCompleted: retakeSettingsSnapshot(settings)
        property string profile_name: "default"        
        property string theme_status_bar_background_color: "#800080"
        property bool volume_change_gears: false
        property string peloton_username: "username"
        property string peloton_password: "password"

        property bool gym_mode: false

        property bool shortcuts_enabled: false
        property string shortcut_speed_plus: ""
        property string shortcut_speed_minus: ""
        property string shortcut_inclination_plus: ""
        property string shortcut_inclination_minus: ""
        property string shortcut_resistance_plus: ""
        property string shortcut_resistance_minus: ""
        property string shortcut_peloton_resistance_plus: ""
        property string shortcut_peloton_resistance_minus: ""
        property string shortcut_target_resistance_plus: ""
        property string shortcut_target_resistance_minus: ""
        property string shortcut_target_power_plus: ""
        property string shortcut_target_power_minus: ""
        property string shortcut_target_zone_plus: ""
        property string shortcut_target_zone_minus: ""
        property string shortcut_target_speed_plus: ""
        property string shortcut_target_speed_minus: ""
        property string shortcut_target_incline_plus: ""
        property string shortcut_target_incline_minus: ""
        property string shortcut_fan_plus: ""
        property string shortcut_fan_minus: ""
        property string shortcut_peloton_offset_plus: ""
        property string shortcut_peloton_offset_minus: ""
        property string shortcut_peloton_remaining_plus: ""
        property string shortcut_peloton_remaining_minus: ""
        property string shortcut_remaining_time_plus: ""
        property string shortcut_remaining_time_minus: ""
        property string shortcut_gears_plus: ""
        property string shortcut_gears_minus: ""
        property string shortcut_pid_hr_plus: ""
        property string shortcut_pid_hr_minus: ""
        property string shortcut_ext_incline_plus: ""
        property string shortcut_ext_incline_minus: ""
        property string shortcut_biggears_plus: ""
        property string shortcut_biggears_minus: ""
        property string shortcut_avs_cruise: ""
        property string shortcut_avs_climb: ""
        property string shortcut_avs_sprint: ""
        property string shortcut_power_avg: ""
        property string shortcut_erg_mode: ""
        property string shortcut_preset_resistance_1: ""
        property string shortcut_preset_resistance_2: ""
        property string shortcut_preset_resistance_3: ""
        property string shortcut_preset_resistance_4: ""
        property string shortcut_preset_resistance_5: ""
        property string shortcut_preset_speed_1: ""
        property string shortcut_preset_speed_2: ""
        property string shortcut_preset_speed_3: ""
        property string shortcut_preset_speed_4: ""
        property string shortcut_preset_speed_5: ""
        property string shortcut_preset_inclination_1: ""
        property string shortcut_preset_inclination_2: ""
        property string shortcut_preset_inclination_3: ""
        property string shortcut_preset_inclination_4: ""
        property string shortcut_preset_inclination_5: ""
        property string shortcut_preset_powerzone_1: ""
        property string shortcut_preset_powerzone_2: ""
        property string shortcut_preset_powerzone_3: ""
        property string shortcut_preset_powerzone_4: ""
        property string shortcut_preset_powerzone_5: ""
        property string shortcut_preset_powerzone_6: ""
        property string shortcut_preset_powerzone_7: ""
        property string shortcut_auto_resistance: ""
        property string shortcut_lap: ""
        property string shortcut_start_stop: ""
        property string shortcut_stop: ""
        property bool android_landscape_cutout_margin: true
        property bool android_landscape_cutout_prompt_shown: false
        property int ui_text_scale: 100
        property bool ui_text_scale_system: true
        property bool ui_modern: false
        property string ui_theme: "graphite"
        property string ui_accent: "violet"
        property string ui_theme_mode: "auto"
    }

    // Modern look (settings.ui_modern, off by default). Pages and Ui* components read the palette
    // and the text sizes through window.ui; with ui.modern off every page keeps its classic look,
    // and nothing here asks the system for its theme
    readonly property QtObject ui: QtObject {
        readonly property bool modern: settings.ui_modern

        // Appearance: "auto" follows the night mode of the phone (dark on Android before 10 and
        // iOS before 13, which have no dark theme), "dark" and "light" are fixed. The classic look
        // is always dark.
        readonly property string themeMode: settings.ui_theme_mode
        property bool systemDark: true
        readonly property bool dark: !modern || themeMode === "dark" || (themeMode !== "light" && systemDark)

        readonly property var themes: ({
            "graphite": { bg: "#111318", surface: "#1B1E24", surfaceHigh: "#23272E", surfaceHighest: "#2D323A",
                          outline: "#3B414B", textMain: "#E4E6EB", textMuted: "#A2A8B3" },
            "oled":     { bg: "#000000", surface: "#101215", surfaceHigh: "#181B1F", surfaceHighest: "#22262B",
                          outline: "#30353C", textMain: "#E6E7EA", textMuted: "#9CA2AC" },
            "midnight": { bg: "#0B1220", surface: "#131C2D", surfaceHigh: "#1B2639", surfaceHighest: "#243148",
                          outline: "#35445F", textMain: "#E3E9F4", textMuted: "#99A6BE" }
        })
        readonly property var accents: ({
            "violet": "#B69DF8", "blue": "#7AB8FF", "teal": "#4FD8C4",
            "green": "#7EDC8A", "orange": "#FFB36B", "pink": "#F7A1C4"
        })
        readonly property var lightThemes: ({
            "graphite": { bg: "#F4F5F7", surface: "#FFFFFF", surfaceHigh: "#ECEEF1", surfaceHighest: "#E1E4E8",
                          outline: "#C4C8CF", textMain: "#1A1C20", textMuted: "#5B616B" },
            "oled":     { bg: "#FFFFFF", surface: "#F3F4F6", surfaceHigh: "#E9EBEE", surfaceHighest: "#DDE0E4",
                          outline: "#C8CCD2", textMain: "#111214", textMuted: "#5A5F68" },
            "midnight": { bg: "#EEF2F9", surface: "#FFFFFF", surfaceHigh: "#E3E9F4", surfaceHighest: "#D6DFEE",
                          outline: "#B8C4D9", textMain: "#162033", textMuted: "#55627A" }
        })
        // At least 5:1 as text on every light surface (WCAG AA asks 4.5), and white on them as fill
        readonly property var lightAccents: ({
            "violet": "#6444BE", "blue": "#1B59B3", "teal": "#00675B",
            "green": "#26682A", "orange": "#984200", "pink": "#A32C62"
        })
        readonly property var t: dark ? (themes[settings.ui_theme] || themes["graphite"])
                                      : (lightThemes[settings.ui_theme] || lightThemes["graphite"])
        readonly property var a: dark ? accents : lightAccents

        // The system is asked only while the modern look is on
        function refreshSystemTheme() {
            if (modern)
                systemDark = AndroidStatusBar.systemDarkMode()
        }
        onModernChanged: refreshSystemTheme()
        Component.onCompleted: refreshSystemTheme()

        readonly property color bg: t.bg
        readonly property color surface: t.surface
        readonly property color surfaceHigh: t.surfaceHigh
        readonly property color surfaceHighest: t.surfaceHighest
        readonly property color outline: t.outline
        readonly property color textMain: t.textMain
        readonly property color textMuted: t.textMuted
        readonly property color accent: a[settings.ui_accent] || a["violet"]
        readonly property color accentInk: dark ? "#12101A" : "#FFFFFF"
        readonly property color danger: dark ? "#FF8A80" : "#C62828"
        readonly property color ok: dark ? "#7EDC8A" : "#2E7D32"
        // Dimming behind a modal popup
        readonly property color scrim: Qt.rgba(0, 0, 0, dark ? 0.6 : 0.4)

        // Type scale: the role sizes at 100 %, times the text size of the app (window.textScale).
        // Pages and Ui* components take a role, not a number, so the text of the modern look grows as one
        function fontPx(base) { return window.fontPx(base) }
        // The font f with another size (and weight): Qt.font() alone keeps nothing but what it is given
        function fontOf(f, px, weight) {
            return Qt.font({ family: f.family, pixelSize: px, weight: weight === undefined ? f.weight : weight,
                             italic: f.italic, capitalization: f.capitalization, letterSpacing: f.letterSpacing,
                             hintingPreference: f.hintingPreference })
        }
        readonly property int fontHeadline: fontPx(22) // page titles
        readonly property int fontTitle: fontPx(20)    // dialog titles, empty states
        readonly property int fontTitleSmall: fontPx(18) // toolbar title
        readonly property int fontBody: fontPx(16)     // dialog text, list titles, fields
        readonly property int fontLabel: fontPx(15)    // drawer entries, group titles
        readonly property int fontCaption: fontPx(13)  // secondary lines, chips, captions
        // The app font (12 on the emulator, not the Material 14): labels, fields, buttons, lists
        readonly property int fontBase: fontPx(window.appFontPx)

        function alpha(c, a) { return Qt.rgba(c.r, c.g, c.b, a) }
        // A title without its emoji ("Maps 🗺️", "Video 🎥"): Qt 5 on Android draws no colour emoji
        // with the app font, they showed as boxes. For the headers of the modern look and the
        // toast and search results of the settings page (its plainTitle() calls this). Drops a
        // pictograph (a surrogate pair in U+1F000-1FBFF, or U+2300-23FF, U+2600-27BF) with the
        // spaces before it and the emoji style mark (U+FE0F) after it
        function plainTitle(text) {
            var s = String(text)
            var out = ""
            var gap = ""
            for (var i = 0; i < s.length; i++) {
                var c = s.charCodeAt(i)
                var n = 0
                if (c >= 0xD83C && c <= 0xD83E && i + 1 < s.length
                        && s.charCodeAt(i + 1) >= 0xDC00 && s.charCodeAt(i + 1) <= 0xDFFF)
                    n = 2
                else if ((c >= 0x2300 && c <= 0x23FF) || (c >= 0x2600 && c <= 0x27BF))
                    n = 1
                if (n > 0) {
                    gap = ""
                    i += n - 1
                    if (i + 1 < s.length && s.charCodeAt(i + 1) === 0xFE0F)
                        i++
                } else if (s[i].trim() === "") {
                    gap += s[i]
                } else {
                    out += gap + s[i]
                    gap = ""
                }
            }
            return (out + gap).trim()
        }
        // Text that the classic look paints in a fixed colour (mostly white) on the page
        // background: the theme text colour in the modern look, the old colour otherwise
        function ink(classic) { return modern ? textMain : classic }
    }

    // Classic look: undefined resets them to the style (qtquickcontrols2.conf), as without these lines
    Material.theme: ui.modern ? (ui.dark ? Material.Dark : Material.Light) : undefined
    Material.accent: ui.modern ? ui.accent : undefined
    Material.background: ui.modern ? ui.bg : undefined

    // The phone can switch its night mode while the app runs (by schedule or from the quick
    // settings): Android reports it as a configuration change, iOS through the return to the
    // foreground (onStateChanged below)
    Connections {
        target: AndroidStatusBar
        function onSystemThemeChanged() { window.ui.refreshSystemTheme() }
    }

    // Text size of the app (not the tiles, they follow ui_zoom). On Android and iOS it follows the
    // system text size by default, capped so the fixed-height rows still fit; otherwise ui_text_scale.
    // Qt 5.15 applies neither by itself, and Qt.application.font is constant, so the size goes
    // through the window font, which the labels, fields, buttons, lists and menus inherit
    readonly property bool systemTextScaleAvailable: Qt.platform.os === "android" || Qt.platform.os === "ios"
    readonly property real systemTextScaleCap: 1.5
    property real systemTextScale: AndroidStatusBar.systemFontScale()
    readonly property real textScale: systemTextScaleAvailable && settings.ui_text_scale_system
        ? Math.max(0.8, Math.min(systemTextScaleCap, systemTextScale))
        : Math.max(0.8, Math.min(1.7, settings.ui_text_scale / 100))
    // At 100 % every size stays exactly as before
    function fontPx(base) { return textScale === 1 ? base : Math.round(base * textScale) }
    // On iOS the app font is not the one of the controls: Qt gives it the size of the body text,
    // 17 px at the default system text size and more with a bigger one (Dynamic Type), while the
    // controls keep 14 px. So the controls are scaled from 14 and a plain Text from 17
    readonly property bool iosTextSize: Qt.platform.os === "ios"
    readonly property int iosBodyPx: 17
    readonly property int appFontPx: iosTextSize ? 14 : Qt.application.font.pixelSize > 0 ? Qt.application.font.pixelSize : 14
    // Size of the hints under the settings. Where the app font has no pixel size (desktop)
    // "pixelSize - 2" is ignored and the hints have the size of the text: kept so when scaled
    readonly property int fontHint: textScale === 1 ? Qt.application.font.pixelSize - 2
        : iosTextSize ? Math.round((iosBodyPx - 2) * textScale)
        : fontPx(Qt.application.font.pixelSize > 0 ? Qt.application.font.pixelSize - 2 : appFontPx)
    // The window font is set only for a size other than 100 % and reset otherwise
    readonly property int scaledFontPx: textScale !== 1 ? fontPx(appFontPx) : 0
    // Size of a plain Text, which takes the app font, not the window font (0 at 100 %). On Android
    // the app font has a pixel size, the one of the controls. On iOS a Text is scaled from the body
    // size (above); on desktop the app font has a point size (in QML "pointSize": there is no
    // "pointSizeF"), so a Text is scaled from that, in pixels
    readonly property int plainFontPx: textScale === 1 ? 0
        : iosTextSize ? Math.round(iosBodyPx * textScale)
        : Qt.application.font.pixelSize > 0 || !(Qt.application.font.pointSize > 0) ? scaledFontPx
        : Math.round(Qt.application.font.pointSize * Screen.logicalPixelDensity * 25.4 / 72 * textScale)
    // Base of a plain Text that is a multiple of the text size (the file names in profiles)
    readonly property int plainBasePx: iosTextSize ? iosBodyPx : appFontPx
    // Font of the section headers of the settings: a plain Text, which does not inherit the window
    // font. At 100 % the app font, the one a Text takes by itself
    readonly property font headerFont: textScale === 1 ? Qt.application.font
        : Qt.font({ family: Qt.application.font.family, pixelSize: plainFontPx })
    // Wrap mode of a label next to a field: with a bigger text it no longer fits in one line
    // and ran under the field and the OK button. The fixed-size popups below wrap their text too,
    // and then take the width of the screen and the height of the text
    readonly property int labelWrap: textScale === 1 ? Text.NoWrap : Text.WordWrap
    // Space under the hint of a setting: a wrapped label of the next one fills its row and
    // touched the hint above it, the two read as one block
    readonly property int hintGap: textScale === 1 ? 0 : Math.round(6 * textScale)
    function applyTextScale() {
        if (scaledFontPx > 0)
            font.pixelSize = scaledFontPx
        else
            font = undefined
    }
    onScaledFontPxChanged: applyTextScale()
    Component.onCompleted: applyTextScale()
    // The settings page has its own Settings object, which this one does not hear about until a
    // restart: the text size options are written through here, so the size changes at once
    function setTextScaleOption(key, value) {
        settings[key] = value
    }
    // The system text size is changed in the phone settings, while QZ is in the background
    Connections {
        target: Qt.application
        function onStateChanged() {
            if (Qt.application.state === Qt.ApplicationActive) {
                window.systemTextScale = AndroidStatusBar.systemFontScale()
                window.ui.refreshSystemTheme()
            }
        }
    }


    Store {
        id: iapStore
    }

    Loader {
      id: googleMapUI
      source:"GoogleMap.qml";
      active: false
      onLoaded: { console.log("googleMapUI loaded"); stackView.push(googleMapUI.item); }
    }

    // here in order to cache everything for the SwagBagView
    Product {
        id: productUnlockVowels
        type: Product.Unlockable
        store: iapStore
        identifier: "org.cagnulein.qdomyoszwift.swagbag"

        onPurchaseSucceeded: {
            console.log(identifier + " purchase successful");
            applicationData.vowelsUnlocked = true;
            transaction.finalize();
            pageStack.pop();
        }

        onPurchaseFailed: {
            console.log(identifier + " purchase failed");
            console.log("reason: "
                        + transaction.failureReason === Transaction.CanceledByUser ? "Canceled" : transaction.errorString);
            transaction.finalize();
        }

        onPurchaseRestored: {
            console.log(identifier + " purchase restored");
            applicationData.vowelsUnlocked = true;
            console.log("timestamp: " + transaction.timestamp);
            transaction.finalize();
            pageStack.pop();
        }
    }

    ToastManager {
        id: toast
    }

    property bool lapPromptVisible: false
    property string lapPromptText: ""

    function isLapPromptMessage(message) {
        var lowerMessage = message.toLowerCase()
        return (lowerMessage.indexOf("press") >= 0 && lowerMessage.indexOf("lap") >= 0) ||
               (lowerMessage.indexOf("lap") >= 0 && lowerMessage.indexOf("continue") >= 0 &&
                lowerMessage.indexOf("received") < 0)
    }

    Rectangle {
        id: lapPromptOverlay
        z: Infinity
        visible: window.lapPromptVisible
        anchors.centerIn: parent
        width: Math.min(parent.width - 32, 520)
        height: Math.max(96, lapPromptLabel.implicitHeight + 44)
        radius: 8
        color: "#9C27B0"
        border.color: "white"
        border.width: 3
        opacity: visible ? 1 : 0

        Label {
            id: lapPromptLabel
            anchors.fill: parent
            anchors.margins: 16
            text: window.lapPromptText
            color: "white"
            font.bold: true
            font.pixelSize: window.fontPx(26)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        SequentialAnimation on scale {
            running: lapPromptOverlay.visible
            loops: Animation.Infinite
            NumberAnimation { from: 1.0; to: 1.05; duration: 450; easing.type: Easing.InOutQuad }
            NumberAnimation { from: 1.05; to: 1.0; duration: 450; easing.type: Easing.InOutQuad }
        }
    }

    Timer {
        id: lapPromptAutoClose
        interval: 15000
        repeat: false
        onTriggered: window.lapPromptVisible = false
    }

    Timer {
        interval: 1
        repeat: false
        running: (rootItem.toastRequested !== "")
        onTriggered: {
            if (window.isLapPromptMessage(rootItem.toastRequested)) {
                window.lapPromptText = rootItem.toastRequested;
                window.lapPromptVisible = true;
                lapPromptAutoClose.restart();
            } else {
                toast.show(rootItem.toastRequested);
            }
            rootItem.toastRequested = "";
        }
    }

    Timer {
       id: gymModeStartupTimer
       interval: 1500
       running: true
       repeat: true
       onTriggered: {
            if (typeof rootItem === "undefined" || !rootItem) {
                return
            }
            maybeOpenGymModePopup()
            if (popupGymMode.visible || rootItem.hasConnectedDevice() || gymModePopupDismissed || !settings.gym_mode) {
                stop()
            }
        }
    }

    /*
    Timer {
        interval: 1000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            toast.show("This timer has triggered " + (++i) + " times!");
        }
    }

    Timer {
        interval: 3000
        repeat: true
        running: true
        property int i: 0
        onTriggered: {
            toast.show("This important message has been shown " + (++i) + " times.", 5000);
        }
    }*/

    // Shared by the toolbar "◄" button and the Android back button.
    // stepInsidePage: pages with their own inner navigation (Wizard, training programs
    // list) first go back one step there, via their handleBack().
    // Returns false when there is nothing to go back to (home page).
    function navigateBack(stepInsidePage) {
        if (stepInsidePage && stackView.currentItem && typeof stackView.currentItem.handleBack === "function" &&
                stackView.currentItem.handleBack()) {
            return true
        }
        if (stackView.depth <= 1) {
            return false
        }

        var remindToSaveProfile = headerToolbar.settingsPageActive &&
                stackView.currentItem &&
                typeof stackView.currentItem.profileSaveReminderNeeded === "function" &&
                stackView.currentItem.profileSaveReminderNeeded()
        var activeProfileName = settings.profile_name

        if(window.settings_restart_to_apply === true) {
            window.settings_restart_to_apply = false;
            popupRestartApp.visible = true;
        }

        stackView.pop()
        // Back from a page opened out of the settings (Tiles Options...) the settings keep their
        // load and save buttons, and with them the tile lock, map, video and chart stay hidden
        var backOnSettings = stackView.depth > 1 && headerToolbar.settingsPageActive
        toolButtonLoadSettings.visible = backOnSettings;
        toolButtonSaveSettings.visible = backOnSettings;
        rootItem.sortTiles()
        if (remindToSaveProfile) {
            toast.show(qsTr("Remember to save profile \"%1\" if you want to keep these changes in this profile.").arg(activeProfileName))
        }
        return true
    }

    // On Android an unhandled back key closes the window, which quits the app.
    // (Keys.onBackPressed cannot be attached to ApplicationWindow: it is not an Item.)
    // Popups and the drawer close themselves on back before this is reached.
    // Android 16+ with targetSdk 36 no longer sends the back key to the app unless
    // AndroidManifest.xml sets android:enableOnBackInvokedCallback="false".
    onClosing: {
        if (OS_VERSION !== "Android") {
            return
        }
        if (navigateBack(true)) {
            close.accepted = false
            return
        }
        if (backToExitTimer.running) {
            return  // second press within the interval: let the app close
        }
        close.accepted = false
        backToExitTimer.start()
        toast.show(qsTr("Press back again to exit"), backToExitTimer.interval)
    }

    Timer {
        id: backToExitTimer
        repeat: false
        interval: 2000 // ms
    }

    UiNotice {
        id: popup
        text: qsTr("Program has been loaded correctly. Press start to begin!")
    }

    UiMessageDialog {
           id: popupPelotonAuth
           text: qsTr("Peloton Authentication Change")
           informativeText: qsTr("Peloton has moved to a new authentication system. Username and password are no longer required.\n\nWould you like to switch to the new authentication method now?")
           buttons: (MessageDialog.Yes | MessageDialog.No)
           onYesClicked: {
               settings.peloton_username = "username"
               settings.peloton_password = "password"
               stackView.push("WebPelotonAuth.qml")
               peloton_connect_clicked()
           }
           onNoClicked: this.visible = false
           visible: false
       }

    Timer {
       id: pelotonAuthCheck
       interval: 1000  // 1 second delay after startup
       running: true
       repeat: false
       onTriggered: {
           if (settings.peloton_password !== "password") {
               popupPelotonAuth.visible = true
           }
       }
    }

    UiInfoPopup {
        id: popupClassificaHelper
        onClosed: stackView.push("Classifica.qml");
        text: qsTr("QZ Classifica is a realtime viewer about the actual\neffort of every QZ users! If you want to join in,\nchoose a nickname in the general settings\nand enable the QZ Classifica setting in the\nexperimental settings section and\nrestart the app.")
    }

    Popup {
        id: popupGymMode
        parent: Overlay.overlay
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        width: Math.min(parent.width - 30, 720)
        // with a bigger text the content is taller: Skip fell out
        height: window.textScale !== 1
            ? Math.min(parent.height - 40, Math.max(260, gymModeColumn.implicitHeight + 36 + topPadding + bottomPadding))
            : Math.min(parent.height - 40, 260)
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        onOpened: refresh_bluetooth_devices_clicked()

        Column {
            id: gymModeColumn
            anchors.fill: parent
            anchors.margins: 18
            spacing: 14

            Label {
                width: parent.width
                text: qsTr("Select Your Gym Device")
                font.pixelSize: window.fontPx(Qt.application.font.pixelSize + 10)
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            Label {
                width: parent.width
                text: qsTr("QZ found the nearby Bluetooth trainers. Choose the machine you want to use for this session.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            ValueComboBox {
                id: gymModeDeviceComboBox
                width: parent.width
                model: rootItem.bluetoothDevices
                labels: ({ "Disabled": qsTr("Disabled") })
                displayText: currentIndex >= 0 ? labelFor(currentValue) : qsTr("Select a device")
                currentIndex: -1
                font.pixelSize: window.fontPx(Qt.application.font.pixelSize + 8)

                onActivated: {
                    var selectedDevice = stripBluetoothDeviceName(currentValue)
                    if (selectedDevice === "Disabled" || selectedDevice === "Wifi" || selectedDevice.length === 0) {
                        return
                    }
                    popupGymMode.close()
                    rootItem.selectGymModeDevice(selectedDevice)
                }
            }

            Label {
                width: parent.width
                text: qsTr("The list refreshes automatically every 10 seconds.")
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: Material.color(Material.Grey)
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Skip")
                onClicked: {
                    gymModePopupDismissed = true
                    popupGymMode.close()
                }
            }
        }
    }

    Timer {
        id: gymModeRefreshTimer
        interval: 10000
        repeat: true
        running: popupGymMode.visible
        onTriggered: refresh_bluetooth_devices_clicked()
    }

    UiInfoPopup {
        id: popupWhatsOnZwiftHelper
        // the address is only named: "not affiliated with ... website"
        links: false
         onClosed: {
             stackView.push("WebEngineTest.qml")
             drawer.close()
             stackView.currentItem.trainprogram_zwo_loaded.connect(trainprogram_zwo_loaded)
             stackView.currentItem.trainprogram_zwo_loaded.connect(function(s) {
                 stackView.pop();
              });
         }
        text: qsTr("Browse the What's on Zwift workout library<br>and choose your workout. It will<br> be automatically loaded on QZ when you will<br>press the load button on the top!<br><br>QZ is not affiliated with Zwift<br>or https://whatsonzwift.com/ website.")
    }

    UiNotice {
        id: popupLoadSettings
        text: qsTr("Settings has been loaded correctly. Restart the app!")
    }

    UiNotice {
        id: popupSaveFile
        // Modern look: only the place of this phone's system, one line that wraps by itself
        text: !window.ui.modern ? qsTr("Saved! Check your private folder (Android)<br>or Files App (iOS)")
              : Qt.platform.os === "ios" ? qsTr("Saved! Look in the Files app.")
              : qsTr("Saved! Look in the app's private folder.")
    }

    UiInfoPopup {
        id: popupStravaConnected
         enabled: rootItem.generalPopupVisible
         onEnabledChanged: { if(rootItem.generalPopupVisible) popupStravaConnected.open() }
         onClosed: { rootItem.generalPopupVisible = false; }
        classicHeight: 120
        classicLabelWidth: 370
        classicLabelHeight: 120
        title: qsTr("Strava")
        text: qsTr("Your Strava account is now connected!<br><br>When you will save a FIT file it will<br>automatically uploaded to Strava!")
    }

    UiInfoPopup {
        id: popupPelotonConnected
         enabled: rootItem.pelotonPopupVisible
         onEnabledChanged: { if(rootItem.pelotonPopupVisible) popupPelotonConnected.open() }
         onClosed: { rootItem.pelotonPopupVisible = false; }
        classicHeight: 120
        classicLabelWidth: 370
        classicLabelHeight: 120
        title: qsTr("Peloton")
        text: qsTr("Your Peloton account is now connected!<br><br>Restart the app to apply this change!")
    }

    Timer {
        id: popupLicenseAutoClose
        interval: 120000; running: rootItem.licensePopupVisible; repeat: false
        onTriggered: popupLicense.close();
    }

    UiInfoPopup {
        id: popupLicense
         enabled: rootItem.licensePopupVisible
         onEnabledChanged: { if(rootItem.licensePopupVisible) popupLicense.open() }
         onClosed: { Qt.openUrlExternally("https://www.patreon.com/bePatron?u=45290147"); Qt.callLater(Qt.quit); }
        classicWidth: 580
        classicHeight: 230
        classicLabelWidth: 570
        classicLabelHeight: 220
        text: qsTr("Trial time expired!<br><br>Please join the QZ Patreon Membership to unlock the full license!<br>https://www.patreon.com/bePatron?u=45290147<br><br>Then add your patreon email in the email field in the general settings.<br>The App will now close.")
    }

    UiMessageDialog {
        id: popupRestartApp
        text: qsTr("Settings changed")
        informativeText: qsTr("In order to apply the changes you need to restart the app.\nDo you want to do it now?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: Qt.callLater(Qt.quit)
        onNoClicked: this.visible = false;
        visible: false
    }

    // In landscape the camera side keeps an empty margin as wide as the cutout. The first time
    // it happens (the cutout inset is wider than the system bar one), ask once whether to let
    // the content go under the cutout instead. The delay lets the rotation settle first.
    readonly property bool landscapeCutoutPromptDue: Qt.platform.os === "android" && AndroidStatusBar.apiLevel >= 31 &&
        (Screen.orientation === Qt.LandscapeOrientation || Screen.orientation === Qt.InvertedLandscapeOrientation) &&
        settings.android_landscape_cutout_margin && !settings.android_landscape_cutout_prompt_shown &&
        // the cutout safe inset also covers curved (waterfall) edges, which are not a camera
        (AndroidStatusBar.leftInset > Math.max(AndroidStatusBar.systemBarLeftInset, AndroidStatusBar.waterfallLeftInset) ||
         AndroidStatusBar.rightInset > Math.max(AndroidStatusBar.systemBarRightInset, AndroidStatusBar.waterfallRightInset))

    Timer {
        interval: 5000
        running: window.landscapeCutoutPromptDue
        onTriggered: {
            settings.android_landscape_cutout_prompt_shown = true
            popupLandscapeCutout.visible = true
        }
    }

    UiMessageDialog {
        id: popupLandscapeCutout
        text: qsTr("Camera Cutout")
        informativeText: qsTr("In landscape, QZ keeps a margin on the camera side so the camera hole does not cover the content.\nDo you want to use the full screen width instead?\n\nYou can change it later in Settings > General UI Options > Keep Content Clear of the Camera Cutout.")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: settings.android_landscape_cutout_margin = false
        onNoClicked: this.visible = false
        visible: false
    }

    // virtual gears on a Wahoo trainer change a value stored in the trainer: at the first connection
    // the trainer gets nothing until this is answered (homeform::wahooWheelQuestionPending)
    UiMessageDialog {
        id: popupWahooWheel
        text: qsTr("Wahoo Virtual Gears")
        informativeText: qsTr("To simulate virtual gears, QZ changes the wheel circumference stored in your trainer. This value may stay in the trainer after you close QZ and affect it in other apps.\n\nAllow QZ to change it?\n\nNo: QZ turns on Without Wheel Diameter Protocol (Settings > Bike Options > Wahoo Options), and virtual gears work through the grade. With Yes you can undo it later there: turn on that option and, with the trainer connected, tap Restore Default Setting to the Trainer.")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        // the modern card: the text names these buttons, so they are translated as in main.qml
        yesText: qsTr("Yes")
        noText: qsTr("No")
        onYesClicked: rootItem.wahooWheelAnswer(true)
        onNoClicked: rootItem.wahooWheelAnswer(false)
        // the back key closes it without an answer, and the trainer would wait until the next start
        onClosed: if (!answered) Qt.callLater(popupWahooWheel.open)
        visible: false
    }

    Connections {
        target: rootItem
        ignoreUnknownSignals: true
        function onWahooWheelQuestionRequested() {
            popupWahooWheel.open()
        }
    }

    // a new workout history database was started (first start, reinstall, database removed):
    // the workouts of a previous install may still be in the QZ folder, hidden from the app
    UiMessageDialog {
        id: popupHistoryRecovery
        text: qsTr("Workout History")
        informativeText: qsTr("Workouts of a previous installation may still be in the QZ folder.\nDo you want to look for them?\n\nIn the next window, allow access to the QZ folder.")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: rootItem.importFitFolder()
        onNoClicked: this.visible = false
        visible: false
    }

    Connections {
        target: rootItem
        ignoreUnknownSignals: true
        function onHistoryRecoveryOfferRequested() {
            popupHistoryRecovery.visible = true
        }
        function onTrainingRestoreOfferRequested() {
            popupTrainingRestore.visible = true
        }
        function onChangeOfdevice() {
            window.equipmentConnected = window.ui.modern && rootItem.supportDeviceConnected()
        }
    }

    // Modern look: equipment connected, for the header and the drawer, set by onChangeOfdevice
    // above. Not rootItem.device: with no equipment its getter flips on every read to blink the
    // Bluetooth icon of the home page, and one more binding reading it would stop the blinking
    property bool equipmentConnected: false

    // after a reinstall the training programs of the previous install are hidden in the
    // training folder: with access to the QZ folder QZ copies them back as its own
    UiMessageDialog {
        id: popupTrainingRestore
        text: qsTr("Training Programs")
        informativeText: qsTr("Training programs of a previous installation may still be in the QZ folder.\nDo you want to look for them?\n\nIn the next window, allow access to the QZ folder.")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: rootItem.restoreTrainingPrograms()
        onNoClicked: this.visible = false
        visible: false
    }

    // a device changed a setting on its own (auto-detection): the message says what QZ found and why it must restart
    UiMessageDialog {
        id: popupRestartAppDetected
        text: ""
        informativeText: qsTr("Restart now?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: Qt.callLater(Qt.quit)
        onNoClicked: this.visible = false;
        visible: false
    }

    Connections {
        target: rootItem
        ignoreUnknownSignals: true
        function onRestartToApplyRequested(message) {
            popupRestartAppDetected.text = message;
            popupRestartAppDetected.visible = true;
        }
    }

    UiMessageDialog {
        text: qsTr("Strava")
        informativeText: qsTr("Do you want to upload the workout to Strava?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {strava_upload_file_prepare(); rootItem.stravaUploadRequested = false;}
        onNoClicked: {rootItem.stravaUploadRequested = false;}
        backAnswersNo: window.ui.modern
        visible: rootItem.stravaUploadRequested
    }

    UiMessageDialog {
        text: qsTr("Garmin Workout Planned")
        informativeText: qsTr("Workout found:\n") + rootItem.garminWorkoutPromptName +
                         (rootItem.garminWorkoutPromptDate.length > 0 ? qsTr("\nDate: ") + rootItem.garminWorkoutPromptDate : "") +
                         qsTr("\n\nDo you want to start it now?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.garmin_start_downloaded_workout(); }
        onNoClicked: { rootItem.garmin_dismiss_downloaded_workout_prompt(); }
        backAnswersNo: window.ui.modern
        visible: rootItem.garminWorkoutPromptRequested
    }

    UiMessageDialog {
        text: qsTr("Garmin FTP Update")
        informativeText: rootItem.garminFtpPromptMessage
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.garmin_accept_ftp_update(); }
        onNoClicked: { rootItem.garmin_dismiss_ftp_update(); }
        backAnswersNo: window.ui.modern
        visible: rootItem.garminFtpPromptRequested
    }

    UiMessageDialog {
        text: qsTr("Clipboard Workout")
        informativeText: qsTr("Workout found in clipboard:\n%1\n\nDo you want to open the workout preview?").arg(rootItem.clipboardWorkoutPromptName)
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            var workoutUrl = rootItem.clipboard_workout_url()
            rootItem.clipboard_accept_workout_prompt()
            var page = CHARTJS
                    ? stackView.push("TrainingProgramsListJS.qml", { initialWorkoutUrl: workoutUrl })
                    : stackView.push("TrainingProgramsList.qml", { initialWorkoutUrl: workoutUrl })
            page.trainprogram_open_clicked.connect(trainprogram_open_clicked)
            page.trainprogram_open_other_folder.connect(trainprogram_open_other_folder)
            page.trainprogram_preview.connect(trainprogram_preview)
            if (page.trainprogram_autostart_requested) {
                page.trainprogram_autostart_requested.connect(trainprogram_autostart_requested)
            }
            page.trainprogram_open_clicked.connect(function(url) {
                stackView.pop();
            });
        }
        onNoClicked: { rootItem.clipboard_dismiss_workout_prompt(); }
        backAnswersNo: window.ui.modern
        visible: rootItem.clipboardWorkoutPromptRequested
    }

    UiMessageDialog {
        text: qsTr("Clipboard Workout")
        informativeText: qsTr("The clipboard workout has ended.\n\nDo you want to delete the file?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: rootItem.clipboard_delete_finished_workout()
        onNoClicked: rootItem.clipboard_keep_finished_workout()
        backAnswersNo: window.ui.modern
        visible: rootItem.clipboardWorkoutDeletePromptRequested
    }

    UiMessageDialog {
        text: qsTr("Echelon Unlock")
        informativeText: qsTr("The bike has been unlocked and cadence is flowing.\n\nDo you want to switch to the classic Bluetooth bridge for this session?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.echelon_switch_to_classic_bridge(); }
        onNoClicked: { rootItem.echelon_dismiss_bridge_switch_prompt(); }
        backAnswersNo: window.ui.modern
        visible: rootItem.echelonBridgeSwitchPromptRequested
    }

    Popup {
        id: echelonEnablePopup
        parent: Overlay.overlay
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        width: Math.min(window.width - 40, 460)
        height: Math.min(window.height - 60, 420)
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        // Modern look: the card below (echelonEnableDialog)
        visible: rootItem.echelonEnablePromptRequested && !window.ui.modern

        background: Rectangle {
            radius: 8
            color: Material.background
            border.color: Material.accent
            border.width: 1
        }

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Label {
                id: echelonEnableTitle
                width: parent.width
                text: qsTr("Echelon Locked Bike")
                font.bold: true
                font.pixelSize: window.fontPx(20)
                wrapMode: Text.WordWrap
            }

            ScrollView {
                width: parent.width
                // 52 is a one-line title: a bigger one pushed the buttons out of the popup
                height: window.textScale !== 1
                    ? parent.height - buttonsRow.height - echelonEnableTitle.height - 2 * parent.spacing
                    : parent.height - buttonsRow.height - 52
                clip: true

                TextArea {
                    width: echelonEnablePopup.width - 56
                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    selectByMouse: true
                    text:
                        qsTr("Your bike is locked by Echelon, but QZ can unlock it.\n\n") +
                        qsTr("Enable Virtual Echelon in the experimental settings and restart qz, then open the official Echelon app on a separate device and connect to the bike once.\n\n") +
                        qsTr("After initialization, return to QZ and everything will work normally.\n\n") +
                        qsTr("You have to repeat this for each session, would you like to enable the Virtual Echelon setting now for this?")
                }
            }

            Row {
                id: buttonsRow
                width: parent.width
                spacing: 12
                layoutDirection: Qt.RightToLeft

                Button {
                    text: qsTr("Yes")
                    onClicked: rootItem.echelon_enable_virtual_bridge()
                }

                Button {
                    text: qsTr("No")
                    onClicked: rootItem.echelon_dismiss_enable_prompt()
                }
            }
        }
    }

    Popup {
        id: androidNotificationPopup
        parent: Overlay.overlay
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        width: Math.min(window.width - 40, 460)
        height: Math.min(window.height - 60, 380)
        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        visible: rootItem.androidNotificationPromptRequested && !window.ui.modern

        background: Rectangle {
            radius: 8
            color: Material.background
            border.color: Material.accent
            border.width: 1
        }

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Label {
                id: androidNotificationTitle
                width: parent.width
                text: qsTr("Connection lost in the background")
                font.bold: true
                font.pixelSize: window.fontPx(20)
                wrapMode: Text.WordWrap
            }

            ScrollView {
                width: parent.width
                // 52 is a one-line title: a bigger one pushed the buttons out of the popup
                height: window.textScale !== 1
                    ? parent.height - androidNotificationButtons.height - androidNotificationTitle.height - 2 * parent.spacing
                    : parent.height - androidNotificationButtons.height - 52
                clip: true

                TextArea {
                    width: androidNotificationPopup.width - 56
                    readOnly: true
                    wrapMode: TextEdit.Wrap
                    text:
                        qsTr("Android cut QZ's network while it was in the background, so the app connected to QZ over Wi-Fi (Zwift, for example) lost it.\n\n") +
                        qsTr("The Android notification keeps QZ running in the background. Enable it now? The workout keeps going, no restart needed.")
                }
            }

            // Flow, not Row: three buttons don't fit one row on a phone once translated
            Flow {
                id: androidNotificationButtons
                width: parent.width
                spacing: 12
                layoutDirection: Qt.RightToLeft

                Button {
                    text: qsTr("Enable")
                    onClicked: rootItem.android_notification_prompt_enable()
                }

                Button {
                    text: qsTr("Not now")
                    onClicked: rootItem.android_notification_prompt_dismiss(false)
                }

                Button {
                    text: qsTr("Don't ask again")
                    onClicked: rootItem.android_notification_prompt_dismiss(true)
                }
            }
        }
    }

    // Modern look of androidNotificationPopup. Yes = enable, No = never ask again, Cancel = not now;
    // back answers nothing, so it counts as "not now" too
    UiMessageDialog {
        id: androidNotificationDialog
        text: qsTr("Connection lost in the background")
        informativeText:
            qsTr("Android cut QZ's network while it was in the background, so the app connected to QZ over Wi-Fi (Zwift, for example) lost it.\n\n") +
            qsTr("The Android notification keeps QZ running in the background. Enable it now? The workout keeps going, no restart needed.")
        buttons: (MessageDialog.Yes | MessageDialog.No | MessageDialog.Cancel)
        yesText: qsTr("Enable")
        noText: qsTr("Don't ask again")
        cancelText: qsTr("Not now")
        onYesClicked: rootItem.android_notification_prompt_enable()
        onNoClicked: rootItem.android_notification_prompt_dismiss(true)
        onCancelClicked: rootItem.android_notification_prompt_dismiss(false)
        onClosed: if (!answered) rootItem.android_notification_prompt_dismiss(false)
        visible: window.ui.modern && rootItem.androidNotificationPromptRequested
    }

    // Modern look of echelonEnablePopup: the card of the other questions, the same texts.
    // Classic look keeps the popup above (here the native dialog would replace it).
    UiMessageDialog {
        id: echelonEnableDialog
        text: qsTr("Echelon Locked Bike")
        informativeText:
            qsTr("Your bike is locked by Echelon, but QZ can unlock it.\n\n") +
            qsTr("Enable Virtual Echelon in the experimental settings and restart qz, then open the official Echelon app on a separate device and connect to the bike once.\n\n") +
            qsTr("After initialization, return to QZ and everything will work normally.\n\n") +
            qsTr("You have to repeat this for each session, would you like to enable the Virtual Echelon setting now for this?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: rootItem.echelon_enable_virtual_bridge()
        onNoClicked: rootItem.echelon_dismiss_enable_prompt()
        backAnswersNo: true
        visible: window.ui.modern && rootItem.echelonEnablePromptRequested
    }

    UiMessageDialog {
        id: stravaLogoutConfirm
        text: qsTr("Strava")
        informativeText: qsTr("You are already connected to Strava. Do you want to log out?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.strava_logout(); }
        onNoClicked: this.visible = false
        visible: false
    }

    UiMessageDialog {
        id: pelotonLogoutConfirm
        text: qsTr("Peloton")
        informativeText: qsTr("You are already connected to Peloton. Do you want to log out?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.peloton_logout(); }
        onNoClicked: this.visible = false
        visible: false
    }

    UiMessageDialog {
        id: intervalsICULogoutConfirm
        text: qsTr("Intervals.icu")
        informativeText: qsTr("You are already connected to Intervals.icu. Do you want to log out?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: { rootItem.intervalsicu_logout(); }
        onNoClicked: this.visible = false
        visible: false
    }

    header: ToolBar {
        contentHeight: toolButton.implicitHeight
        Material.primary: window.ui.modern ? window.ui.bg : settings.theme_status_bar_background_color
        Material.elevation: window.ui.modern ? 0 : 4
        id: headerToolbar
        // Opacity, not visible: some pages set headerToolbar.visible themselves
        opacity: window.rootReady ? 1 : 0
        property bool settingsPageActive: stackView.currentItem && typeof stackView.currentItem.showSettingsSearch === "function"
        // The map, video and chart buttons: everywhere in the classic look, on the home page only
        // in the modern one
        readonly property bool workoutButtonsShown: !window.ui.modern || stackView.depth === 1
        // Modern look: load and save belong to the settings page itself. The pages opened from
        // it (the settings files of the load button among them) kept both, and load could open
        // the list again and again. The profiles page has its own buttons and none of these.
        // The classic look sets them where it always did
        onSettingsPageActiveChanged: {
            if (window.ui.modern) {
                var keep = settingsPageActive
                toolButtonLoadSettings.visible = keep
                toolButtonSaveSettings.visible = keep
            }
        }
        // The toolbar hangs over the pages: in the window layout it is only as tall as the status
        // bar inset, and the page stack below leaves room for the rest of it (stackView
        // anchors.topMargin), so every page sits where it did. The tile grid in Home.qml reaches
        // up under it and slides it away with the scroll (Material "scroll|enterAlways"): barShift
        // is how much of it has gone up under the status bar. The page never changes size, so the
        // tiles do not move under the finger
        property real barShift: 0
        // Moving the tiles keeps it shown: the lock button is on the toolbar. So does any other page
        // (the tile grid can still be gliding or snapping under it when one is opened); back on the
        // home page it is where the scroll left it, not over the Start/Stop row
        readonly property real barHidden: window.lockTiles || stackView.depth > 1 ? 0 : Math.max(0, Math.min(barShift, contentHeight))
        // Set, not derived from the style, which counts the height of the background (48) as well
        implicitHeight: getTopPadding()
        // The content keeps its height and slides up with the bar
        topPadding: getTopPadding() - barHidden
        bottomPadding: barHidden - contentHeight
        // The background, as tall as the status bar inset and the bar, slides up with it. Negative
        // insets let it reach past the header: a background.height binding loses to the control's
        // own resizing at an inset of 0
        topInset: -barHidden
        bottomInset: barHidden - contentHeight
        // The buttons fade out as the bar goes and are hidden once it is gone, from TalkBack as
        // well; the background goes then too
        contentItem.opacity: 1 - barHidden / contentHeight
        contentItem.visible: barHidden < contentHeight
        background.visible: barHidden < contentHeight

        // The bar takes the touches on it, as a toolbar the height of its content did: drawn
        // past the height of the header, it would let a tap on the title reach the tiles below
        MouseArea {
            anchors.fill: parent
            z: -1
        }
        // Clear of the side system bar and the cutout held sideways, as the page stack: with
        // button navigation the bar on the right covered the rightmost button
        leftPadding: getLeftPadding()
        rightPadding: getRightPadding()

        ToolButton {
            id: toolButton
            // Modern look: the buttons of the bar are icons without a text, each named for the
            // screen reader; the classic look keeps the name it had (the text, if any)
            Accessible.name: window.ui.modern ? (stackView.depth > 1 ? qsTr("Back") : qsTr("Main menu")) : text
            icon.source: window.ui.modern ? "" : "icons/icons/icon.png"
            text: window.ui.modern ? "" : (stackView.depth > 1 ? "◄" : "◄")
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: stackView.depth > 1 ? "arrow_back" : "menu"; color: window.ui.textMain; visible: window.ui.modern }
            // The header is as tall as this button. On iOS the app font already carries the system
            // text size (53 px at the largest one), so the arrow is scaled from the body size, as
            // the rest of the text, and keeps the same cap
            font.pixelSize: window.iosTextSize ? window.fontPx(Math.round(window.iosBodyPx * 1.6)) : Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    // Modern look: the arrow goes back one step inside the page as the back
                    // gesture does (settings search results, wizard, workout preview), so
                    // those pages need no back button of their own. Classic look: only back to
                    // the settings search results, the other pages keep their own back buttons
                    navigateBack(window.ui.modern || !!(stackView.currentItem && stackView.currentItem.searchReturn))
                } else {
                    drawer.open()
                }
            }
        }

        ToolButton {
            id: toolButtonFloating
            Accessible.name: window.ui.modern ? qsTr("Floating window") : text
            icon.source: window.ui.modern ? "" : "icons/icons/mini-display.png"
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: "picture_in_picture_alt"; color: window.ui.textMain; visible: window.ui.modern }
            onClicked: { console.log("floating!"); floatingOpen(); }
            anchors.left: toolButton.right
            // The header is as tall as the back arrow, which grows with the text size (iOS):
            // the icon buttons stay in its middle, in line with the arrow and the title
            anchors.verticalCenter: parent.verticalCenter
            // Modern look: on the home page only, the other pages have nothing to float
            visible: OS_VERSION === "Android" ? (!window.ui.modern || stackView.depth === 1) : false
        }

        UiNotice {
            id: popupAutoResistance
            text: rootItem.autoResistance ? qsTr("Auto Resistance enabled") : qsTr("Auto Resistance disabled")
        }

        Timer {
            id: popupAutoResistanceAutoClose
            interval: 2000; running: false; repeat: false
            onTriggered: popupAutoResistance.close();
        }

        UiNotice {
            id: popuplockTiles
            text: window.lockTiles ? qsTr("You can move the tiles!") : qsTr("The tiles are locked now")
        }

        Timer {
            id: popuplockTilesAutoClose
            interval: 2000; running: false; repeat: false
            onTriggered: popuplockTiles.close();
        }

        ToolButton {
            id: toolButtonLoadSettings
            Accessible.name: window.ui.modern ? qsTr("Load settings") : text
            icon.source: window.ui.modern ? "" : "icons/icons/tray-arrow-up.png"
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: "upload_file"; color: window.ui.textMain; visible: window.ui.modern }
            onClicked: {
                stackView.push("SettingsList.qml")
                stackView.currentItem.loadSettings.connect(loadSettings)
                stackView.currentItem.loadSettings.connect(function(url) {
                    stackView.pop();
                    if (stackView.depth > 1) {
                        stackView.pop()
                    }
                    // Modern: the restart asked right away (Yes/No) rather than a notice
                    // at the bottom that only says to do it
                    if (window.ui.modern)
                        popupRestartApp.visible = true;
                    else
                        popupLoadSettings.open();
                 });
                drawer.close()
            }
            anchors.right: toolButtonSaveSettings.left
            anchors.verticalCenter: parent.verticalCenter
            visible: false
        }

        ToolButton {
            id: toolButtonSettingsSearch
            Accessible.name: qsTr("Search settings")
            // A drawn magnifier instead of the emoji: the emoji is blank on devices without a
            // glyph for it. Painted once, again only when the colour changes
            Canvas {
                id: settingsSearchIcon
                property color color: toolButtonSettingsSearch.Material.foreground
                visible: !window.ui.modern
                anchors.centerIn: parent
                // Drawn at the pixel density of the screen and scaled down, so it stays sharp
                width: 24 * Screen.devicePixelRatio
                height: 24 * Screen.devicePixelRatio
                scale: 1 / Screen.devicePixelRatio
                onColorChanged: requestPaint()
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    // Material Symbols "search", viewBox 0 -960 960 960
                    ctx.scale(width / 960, height / 960)
                    ctx.translate(0, 960)
                    ctx.path = "M380-320q-109 0-184.5-75.5T120-580q0-109 75.5-184.5T380-840q109 0 184.5 75.5T640-580q0 44-14 83t-38 69l224 224q11 11 11 28t-11 28q-11 11-28 11t-28-11L532-372q-30 24-69 38t-83 14Zm0-80q75 0 127.5-52.5T560-580q0-75-52.5-127.5T380-760q-75 0-127.5 52.5T200-580q0 75 52.5 127.5T380-400Z"
                    ctx.fillStyle = color
                    ctx.fill()
                }
            }
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: "search"; color: window.ui.textMain; visible: window.ui.modern }
            onClicked: {
                if (headerToolbar.settingsPageActive)
                    stackView.currentItem.showSettingsSearch()
            }
            anchors.right: toolButtonLoadSettings.left
            anchors.verticalCenter: parent.verticalCenter
            visible: headerToolbar.settingsPageActive
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Search settings")
        }

        ToolButton {
            id: toolButtonSaveSettings
            Accessible.name: window.ui.modern ? qsTr("Save settings") : text
            icon.source: window.ui.modern ? "" : "icons/icons/tray-arrow-down.png"
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: "save"; color: window.ui.textMain; visible: window.ui.modern }
            onClicked: {
                saveSettings("settings");
                popupSaveFile.open()
            }
            anchors.right: toolButtonAutoResistance.left/*toolClassifica.left*/
            anchors.verticalCenter: parent.verticalCenter
            visible: false
        }

        /*ToolButton {
            id: toolClassifica
            icon.source: "icons/icons/chart.png"
            onClicked: {  if(settings.classifica_enable) stackView.push("Classifica.qml"); else popupClassificaHelper.open(); }
            anchors.right: toolButtonAutoResistance.left
        }*/

        ToolButton {
            function loadMaps() {
                if(rootItem.currentCoordinateValid) {
                    console.log("coordinate is valid for map");
                    if(googleMapUI.status === Loader.Ready)
                        stackView.push(googleMapUI.item);
                    else
                        googleMapUI.active = true;

                } else {
                    console.log("coordinate is NOT valid for map");
                }
            }
            id: toolButtonMaps
            Accessible.name: window.ui.modern ? qsTr("Map") : text
            icon.source: window.ui.modern ? "" : ( "icons/icons/maps-icon-16.png" )
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: "map"; color: window.ui.textMain; visible: window.ui.modern }
            onClicked: { loadMaps(); }
            anchors.right: toolButtonChart.left
            anchors.verticalCenter: parent.verticalCenter
            // Hidden where load and save are shown (settings, profiles), as the tile lock: there the
            // map, video and chart buttons lie over search, load and the title.
            // Modern look: the map, video and chart buttons belong to the workout of the home page,
            // as the lock and the auto resistance. Elsewhere video and chart toggled what is under
            // the tiles, out of sight, and the map pushed itself again. Hidden, they leave
            // no gap
            visible: rootItem.mapsVisible && headerToolbar.workoutButtonsShown && !toolButtonSaveSettings.visible
            width: window.ui.modern && !visible ? 0 : implicitWidth
        }      

        ToolButton {
            function loadVideo() {
                if(rootItem.currentCoordinateValid || rootItem.trainProgramLoadedWithVideo) {
                    console.log("coordinate is valid for map");
                    //stackView.push("videoPlayback.qml");
                    rootItem.videoVisible = !rootItem.videoVisible
                } else {
                    console.log("coordinate is NOT valid for map");
                }
            }
            id: toolButtonVideo
            Accessible.name: window.ui.modern ? qsTr("Video") : text
            icon.source: window.ui.modern ? "" : ( "icons/icons/video.png" )
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: "videocam"; color: window.ui.textMain; visible: window.ui.modern }
            onClicked: { loadVideo(); }
            anchors.right: toolButtonMaps.left
            anchors.verticalCenter: parent.verticalCenter
            // Modern look: on the home page only, as the map (above)
            visible: rootItem.videoIconVisible && headerToolbar.workoutButtonsShown && !toolButtonSaveSettings.visible
            width: window.ui.modern && !visible ? 0 : implicitWidth
        }

        ToolButton {
            id: toolButtonChart
            Accessible.name: window.ui.modern ? qsTr("Chart") : text
            icon.source: window.ui.modern ? "" : ( "icons/icons/chart.png" )
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: "show_chart"; color: window.ui.textMain; visible: window.ui.modern }
            onClicked: { rootItem.chartFooterVisible = !rootItem.chartFooterVisible }
            anchors.right: toolButtonLockTiles.left
            anchors.verticalCenter: parent.verticalCenter
            // Modern look: on the home page only, as the map (above)
            visible: rootItem.chartIconVisible && headerToolbar.workoutButtonsShown && !toolButtonSaveSettings.visible
            width: window.ui.modern && !visible ? 0 : implicitWidth
        }

        // Share the workout file of the page (PreviewChart.qml opened from the workout history)
        ToolButton {
            id: toolButtonShareFit
            readonly property string filePath: stackView.currentItem && stackView.currentItem.fitFilePath ? stackView.currentItem.fitFilePath : ""
            Accessible.name: qsTr("Share FIT file")
            // drawn as the search button: no image file, sharp at any density. The icon font of the
            // modern look has no share glyph: the same drawing there, in the colour of its icons
            Canvas {
                property color color: window.ui.modern ? window.ui.textMain : toolButtonShareFit.Material.foreground
                anchors.centerIn: parent
                width: 24 * Screen.devicePixelRatio
                height: 24 * Screen.devicePixelRatio
                scale: 1 / Screen.devicePixelRatio
                onColorChanged: requestPaint()
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    // three dots joined by two lines, on a 24 x 24 grid
                    ctx.scale(width / 24, height / 24)
                    ctx.strokeStyle = color
                    ctx.fillStyle = color
                    ctx.lineWidth = 1.6
                    ctx.beginPath()
                    ctx.moveTo(18, 5)
                    ctx.lineTo(6, 12)
                    ctx.lineTo(18, 19)
                    ctx.stroke()
                    var dots = [[18, 5], [6, 12], [18, 19]]
                    for (var i = 0; i < dots.length; i++) {
                        ctx.beginPath()
                        ctx.arc(dots[i][0], dots[i][1], 3, 0, 2 * Math.PI)
                        ctx.fill()
                    }
                }
            }
            onClicked: rootItem.shareWorkoutFile(filePath)
            // left of the leftmost shown button of the chain video - maps - chart - lock
            anchors.right: toolButtonVideo.visible ? toolButtonVideo.left :
                           toolButtonMaps.visible ? toolButtonMaps.left :
                           toolButtonChart.visible ? toolButtonChart.left : toolButtonLockTiles.left
            anchors.verticalCenter: parent.verticalCenter
            visible: filePath !== ""
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Share FIT file")
        }

        ToolButton {
            id: toolButtonLockTiles
            // the name says what a tap does, like auto resistance: lockTiles is the move mode
            // ("You can move the tiles!"), so the fixed tiles of the default get "Unlock tiles"
            Accessible.name: window.ui.modern ? (window.lockTiles ? qsTr("Lock tiles") : qsTr("Unlock tiles")) : text
            icon.source: window.ui.modern ? "" : ( window.lockTiles ? "icons/icons/unlock.png" : "icons/icons/lock.png")
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: (window.lockTiles ? "lock_open" : "lock"); color: window.ui.textMain; visible: window.ui.modern }
            onClicked: { window.lockTiles = !window.lockTiles; console.log("lock tiles toggled " + window.lockTiles); popuplockTiles.open(); popuplockTilesAutoClose.running = true; }
            anchors.right: toolButtonAutoResistance.left
            anchors.verticalCenter: parent.verticalCenter
            // Modern look: the tiles are on the home page only, so is their lock; while the
            // equipment is being searched (rootItem.labelHelp) there are no tiles yet
            visible: window.ui.modern ? stackView.depth === 1 && typeof rootItem !== "undefined" && !rootItem.labelHelp : !toolButtonSaveSettings.visible
            // Modern look: hidden, it leaves no gap. The classic bar keeps the gap: the chart,
            // maps and video buttons are anchored to it and stay where they always were
            width: window.ui.modern && !visible ? 0 : implicitWidth
        }

        ToolButton {
            id: toolButtonAutoResistance
            Accessible.name: window.ui.modern ? (rootItem.autoResistance ? qsTr("Disable auto resistance") : qsTr("Enable auto resistance")) : text
            icon.source: window.ui.modern ? "" : ( rootItem.autoResistance ? "icons/icons/resistance.png" : "icons/icons/pause.png")
            UiIcon { anchors.centerIn: parent; width: 24; height: 24; name: (window.ui.modern && rootItem.autoResistance ? "motion_mode" : "pause_circle"); color: window.ui.textMain; visible: window.ui.modern }
            onClicked: { rootItem.autoResistance = !rootItem.autoResistance; console.log("auto resistance toggled " + rootItem.autoResistance); popupAutoResistance.open(); popupAutoResistanceAutoClose.running = true; }
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            // Modern look: a workout control, on the home page only and while the equipment is
            // connected (before that there is no resistance to follow)
            visible: window.ui.modern ? stackView.depth === 1 && typeof rootItem !== "undefined" && !rootItem.labelHelp && window.equipmentConnected
                                      : !headerToolbar.settingsPageActive
            width: visible ? implicitWidth : 0
        }

        Label {
            id: toolbarTitle
            // Modern look, home page: more buttons on the right than on the left, and the title centred
            // on the screen had room for one letter ("Q..."). It stays centred on the screen if it fits
            // there, else it goes to the middle of the free room between the buttons, in its full
            // size; the name that does not fit there either is cut to "QZ", not to "QZ Fi...".
            // "QZ" goes too when it would not keep the width of a button of free room around it
            readonly property bool homeTitle: window.ui.modern && stackView.depth === 1
            readonly property bool fitsCentred: titleMetrics.advanceWidth <= titleSpan.centred
            readonly property bool fitsFree: titleMetrics.advanceWidth <= titleSpan.to - titleSpan.from
            readonly property bool fitsShort: shortTitleMetrics.advanceWidth + toolButton.width <= titleSpan.to - titleSpan.from
            TextMetrics {
                id: titleMetrics
                font: toolbarTitle.font
                // No page or no title yet at start-up: an empty one, not a warning in the log
                text: stackView.currentItem && stackView.currentItem.title ? stackView.currentItem.title : ""
            }
            TextMetrics {
                id: shortTitleMetrics
                font: toolbarTitle.font
                text: "QZ"
            }
            text: homeTitle && !fitsCentred && !fitsFree ? shortTitleMetrics.text : stackView.currentItem.title
            visible: !homeTitle || fitsCentred || fitsFree || fitsShort
            // Modern look only: the classic title keeps the font of the bar, as it inherited it. A
            // plain binding to the bar font: not Binding (its restored value was the size at
            // start-up, the scaled one after a text size), not Qt.font({}) (a default font with a
            // size of its own, not an inherited one). Modern: its own font in mixed case - built
            // from the bar font, the title came out in capitals on iOS
            font: window.ui.modern ? Qt.font({ family: headerToolbar.font.family, pixelSize: window.ui.fontTitleSmall,
                                               weight: Font.DemiBold, capitalization: Font.MixedCase,
                                               hintingPreference: headerToolbar.font.hintingPreference })
                                   : headerToolbar.font
            color: window.ui.ink(Material.foreground)
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: homeTitle && !fitsCentred ? (titleSpan.from + titleSpan.to - parent.width) / 2 : 0
            // Modern look and a text size other than 100 %: no wider than the room between the
            // nearest shown buttons, centred; a large text size ran the title into the search button. A
            // longer title takes a smaller font down to 14 px of the text size, and only then an ellipsis. The home
            // page keeps the full size (above); a title that fits is not cut by a pixel of rounding
            // between titleMetrics and implicitWidth
            width: homeTitle ? (fitsCentred || fitsFree ? implicitWidth : Math.min(implicitWidth, titleSpan.to - titleSpan.from))
                 : window.ui.modern || window.textScale !== 1 ? Math.min(implicitWidth, titleSpan.centred) : implicitWidth
            fontSizeMode: homeTitle ? Text.FixedSize : window.ui.modern || window.textScale !== 1 ? Text.HorizontalFit : Text.FixedSize
            minimumPixelSize: window.ui.modern ? window.ui.fontPx(14) : window.appFontPx
            // The line keeps the height of the full size: a smaller font in its middle, not at its top
            verticalAlignment: window.ui.modern || window.textScale !== 1 ? Text.AlignVCenter : Text.AlignTop
            elide: window.ui.modern || window.textScale !== 1 ? Text.ElideRight : Text.ElideNone
            // centred: the width of a title centred on the bar; from, to: the free room between the
            // nearest shown buttons on the left and on the right
            readonly property var titleSpan: {
                var c = parent.width / 2, room = c
                var left = [toolButton, toolButtonFloating]
                var right = [toolButtonSettingsSearch, toolButtonLoadSettings, toolButtonSaveSettings, toolButtonMaps,
                             toolButtonVideo, toolButtonChart, toolButtonShareFit, toolButtonLockTiles, toolButtonAutoResistance]
                for (var i = 0; i < left.length; i++)
                    if (left[i].visible && left[i].opacity > 0 && left[i].width > 0)
                        room = Math.min(room, c - (left[i].x + left[i].width))
                // The search of the settings comes and goes with the scroll: its place is kept on the
                // settings page all the time, or the title grew and shrank with it
                for (var j = 0; j < right.length; j++) {
                    var held = right[j] === toolButtonSettingsSearch && headerToolbar.settingsPageActive
                    if ((held || (right[j].visible && right[j].opacity > 0)) && right[j].width > 0)
                        room = Math.min(room, right[j].x - c)
                }
                var from = 0, to = parent.width
                for (var k = 0; k < left.length; k++)
                    if (left[k].visible && left[k].opacity > 0 && left[k].width > 0)
                        from = Math.max(from, left[k].x + left[k].width)
                for (var n = 0; n < right.length; n++) {
                    var kept = right[n] === toolButtonSettingsSearch && headerToolbar.settingsPageActive
                    if ((kept || (right[n].visible && right[n].opacity > 0)) && right[n].width > 0)
                        to = Math.min(to, right[n].x)
                }
                return { centred: Math.max(0, 2 * (room - 4)), from: from + 4, to: Math.max(from + 4, to - 4) }
            }
        }
    }

    // Drawer entries, shared by the classic and the modern drawer
    function drawerAction(key) {
        switch (key) {
        case "profile":
            // Modern look: the settings file buttons belong to the settings page only
            toolButtonLoadSettings.visible = !window.ui.modern;
            toolButtonSaveSettings.visible = !window.ui.modern;
            stackView.push("profiles.qml")
            stackView.currentItem.profile_open_clicked.connect(profile_open_clicked)
            break
        case "settings":
            toolButtonLoadSettings.visible = true;
            toolButtonSaveSettings.visible = true;
            stackView.push("settings.qml")
            break
        case "history":
            stackView.push("WorkoutsHistory.qml")
            stackView.currentItem.fitfile_preview_clicked.connect(fitfile_preview_clicked)
            break
        case "swagbag":
            stackView.push("SwagBagView.qml")
            break
        case "charts":
            console.log(CHARTJS)
            if(CHARTJS)
                stackView.push("ChartJsTest.qml")
            else
                stackView.push("ChartsEndWorkout.qml")
            break
        case "opengpx":
            stackView.push("GPXList.qml")
            stackView.currentItem.trainprogram_open_clicked.connect(gpx_open_clicked)
            stackView.currentItem.trainprogram_open_other_folder.connect(gpx_open_other_folder)
            stackView.currentItem.trainprogram_preview.connect(gpxpreview_open_clicked)
            stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                stackView.pop();
                popup.open();
             });
            break
        case "trainprogram":
            if(CHARTJS)
                stackView.push("TrainingProgramsListJS.qml")
            else
                stackView.push("TrainingProgramsList.qml")
            stackView.currentItem.trainprogram_open_clicked.connect(trainprogram_open_clicked)
            stackView.currentItem.trainprogram_open_other_folder.connect(trainprogram_open_other_folder)
            stackView.currentItem.trainprogram_preview.connect(trainprogram_preview)
            stackView.currentItem.trainprogram_autostart_requested.connect(trainprogram_autostart_requested)
            stackView.currentItem.trainprogram_open_clicked.connect(function(url) {
                stackView.pop();
             });
            break
        case "editor":
            var editorPage = stackView.push("WorkoutEditor.qml")
            if (editorPage) {
                editorPage.closeRequested.connect(function() {
                    stackView.pop()
                })
                // Close editor when workout is started from Save & Start
                trainprogram_autostart_requested.connect(function() {
                    console.log("[main.qml] trainprogram_autostart_requested received, closing editor")
                    editorPage.closeRequested()
                })
            }
            break
        case "savegpx":
            gpx_save_clicked()
            drawer.close()
            popupSaveFile.open()
            return
        case "savefit":
            fit_save_clicked()
            drawer.close()
            popupSaveFile.open()
            return
        case "wizard":
            stackView.push("Wizard.qml")
            break
        case "help":
            Qt.openUrlExternally("https://robertoviola.cloud/qdomyos-zwift-guide/");
            break
        case "community":
            Qt.openUrlExternally("https://www.facebook.com/groups/149984563348738");
            break
        case "credits":
            stackView.push("Credits.qml")
            break
        case "quit":
            console.log("closing...")
            Qt.callLater(Qt.quit)
            return
        case "strava":
            if (rootItem.isStravaLoggedIn()) {
                stravaLogoutConfirm.visible = true
            } else {
                stackView.push("WebStravaAuth.qml")
                strava_connect_clicked()
            }
            break
        case "peloton":
            if (rootItem.isPelotonLoggedIn()) {
                pelotonLogoutConfirm.visible = true
            } else {
                stackView.push("WebPelotonAuth.qml")
                stackView.currentItem.goBack.connect(function() {
                    stackView.pop();
                })
                peloton_connect_clicked()
            }
            break
        case "garmin":
            toolButtonLoadSettings.visible = true;
            toolButtonSaveSettings.visible = true;
            stackView.push("settings.qml")
            if (stackView.currentItem) {
                if (stackView.currentItem.openGarminSection) {
                    stackView.currentItem.openGarminSection()
                }
            }
            break
        case "intervals":
            if (rootItem.isIntervalsICULoggedIn()) {
                intervalsICULogoutConfirm.visible = true
            } else {
                stackView.push("WebIntervalsICUAuth.qml")
                intervalsicu_connect_clicked()
            }
            break
        }
        drawer.close()
    }

    Drawer {
        id: drawer
        width: window.ui.modern ? Math.min(window.width * 0.82, 340 + getLeftPadding()) : window.width * 0.66
        height: window.height
        topPadding: getTopPadding()
        bottomPadding: getBottomPadding()
        leftPadding: getLeftPadding()
        rightPadding: window.ui.modern ? 0 : getRightPadding()
        Accessible.ignored: !drawer.opened

        // Classic look: the background of the style, untouched. Modern look: it is hidden (with
        // it its shadow) and the sheet below takes its place
        background.visible: !window.ui.modern

        // Modern: a sheet with rounded outer corners, over the whole drawer (paddings too). It
        // runs past the left edge, so only the right-hand corners show
        Rectangle {
            visible: window.ui.modern
            anchors.fill: parent
            anchors.topMargin: -drawer.topPadding
            anchors.bottomMargin: -drawer.bottomPadding
            anchors.leftMargin: -drawer.leftPadding - radius
            anchors.rightMargin: -drawer.rightPadding
            radius: 20
            color: window.ui.surface
        }

        ScrollView {
            visible: !window.ui.modern
            contentWidth: -1
            focus: true
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.fill: parent

            Column {
                anchors.fill: parent
                spacing: 3

                ItemDelegate {
                    text: qsTr("Profile: ") + settings.profile_name
                    width: parent.width
                    onClicked: drawerAction("profile")
                }

                ItemDelegate {
                    text: qsTr("Settings")
                    width: parent.width
                    onClicked: drawerAction("settings")
                }

            ItemDelegate {
                text: qsTr("Workouts History")
                width: parent.width
                onClicked: drawerAction("history")
            }
                ItemDelegate {
                    text: qsTr("Swag Bag")
                    width: parent.width
                    onClicked: drawerAction("swagbag")
                }

                ItemDelegate {
                    text: qsTr("Charts")
                    width: parent.width
                    onClicked: drawerAction("charts")
                }
                ItemDelegate {
                    id: gpx_open
                    text: qsTr("Open GPX")
                    width: parent.width
                    onClicked: drawerAction("opengpx")
                }
                ItemDelegate {
                    id: trainprogram_open
                    text: qsTr("Open Train Program")
                    width: parent.width
                    onClicked: drawerAction("trainprogram")
                }
                ItemDelegate {
                    text: qsTr("Workout Editor")
                    width: parent.width
                    onClicked: drawerAction("editor")
                }
                /*
                ItemDelegate {
                    text: qsTr("What's On Zwift™")
                    width: parent.width
                    onClicked: {
                        popupWhatsOnZwiftHelper.open()
                    }
                }*/

                ItemDelegate {
                    id: gpx_save
                    text: qsTr("Save GPX")
                    width: parent.width
                    onClicked: drawerAction("savegpx")
                }
                ItemDelegate {
                    id: fit_save
                    text: qsTr("Save FIT")
                    width: parent.width
                    onClicked: drawerAction("savefit")
                }
                ItemDelegate {
                    id: wizardItem
                    text: qsTr("Wizard")
                    width: parent.width
                    onClicked: drawerAction("wizard")
                }
                ItemDelegate {
                    id: help
                    text: qsTr("Help")
                    width: parent.width
                    onClicked: drawerAction("help")
                }
                ItemDelegate {
                    id: community
                    text: qsTr("Community")
                    width: parent.width
                    onClicked: drawerAction("community")
                }
                ItemDelegate {
                    text: qsTr("Credits")
                    width: parent.width
                    onClicked: drawerAction("credits")
                }
                ItemDelegate {
                    text: qsTr("Quit")
                    width: parent.width
                    visible: OS_VERSION === "Other" ? true : false
                    onClicked: drawerAction("quit")
                }

                ItemDelegate {
                    id: drawerVersion
                    text: "version 2.22.0"
                    width: parent.width
                }

                ItemDelegate {
                    id: strava_connect
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/btn_strava_connectwith_orange.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                    }
                    width: parent.width
                    onClicked: drawerAction("strava")
                }

                ItemDelegate {
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/Button_Connect_Rect_DarkMode.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                    }
                    width: parent.width
                    onClicked: drawerAction("peloton")
                }

                ItemDelegate {
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/garmin-connect-badge.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                        height: 48
                    }
                    width: parent.width
                    onClicked: drawerAction("garmin")
                }

				ItemDelegate {
                    Image {
                        anchors.left: parent.left;
                        anchors.verticalCenter: parent.verticalCenter
                        source: "icons/icons/intervals-logo-with-name.png"
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        width: parent.width
                    }
                    width: parent.width
                    onClicked: drawerAction("intervals")
                }

                    FileDialog {
                        id: fileDialogGPX
                         title: qsTr("Please choose a file")
                         folder: "file://" + rootItem.getWritableAppDir() + 'gpx'
                         onAccepted: {
                             console.log("You chose: " + fileDialogGPX.fileUrl)
                              gpx_open_clicked(fileDialogGPX.fileUrl)
                              fileDialogGPX.close()
                              popup.open()
                            }
                         onRejected: {
                             console.log("Canceled")
                              fileDialogGPX.close()
                            }
                        }
            }
        }

        // Modern drawer: header with the connection state and the profile, grouped entries,
        // the service logins as cards at the bottom
        Flickable {
            id: modernDrawerList
            visible: window.ui.modern
            // Screen reader names of the service cards (images only)
            function serviceName(key) {
                switch (key) {
                case "strava":
                    return rootItem.isStravaLoggedIn() ? qsTr("Disconnect from Strava") : qsTr("Connect with Strava")
                case "peloton":
                    return rootItem.isPelotonLoggedIn() ? qsTr("Disconnect from Peloton") : qsTr("Connect with Peloton")
                case "garmin":
                    return qsTr("Garmin Connect settings")
                case "intervals":
                    return rootItem.isIntervalsICULoggedIn() ? qsTr("Disconnect from Intervals.icu") : qsTr("Connect with Intervals.icu")
                }
                return ""
            }
            anchors.fill: parent
            contentHeight: modernDrawerColumn.height + 16
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            // Only vertical moves scroll: a slightly sideways one went to the drawer and the list
            // did not move
            flickableDirection: Flickable.VerticalFlick
            ScrollIndicator.vertical: ScrollIndicator { }

            Column {
                id: modernDrawerColumn
                width: modernDrawerList.width

                Item {
                    width: parent.width
                    // 132 at 100 %: the chip and the header move down as the text grows
                    height: profileChip.y + profileChip.height + 14

                    Image {
                        id: drawerLogo
                        x: 24
                        y: 22
                        width: 44
                        height: 44
                        source: "qrc:/inner_templates/chartjs/qzlogo.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                    }
                    Label {
                        id: drawerAppName
                        anchors.left: drawerLogo.right
                        anchors.leftMargin: 14
                        anchors.right: parent.right
                        anchors.rightMargin: 16
                        y: drawerLogo.y + 1
                        text: "QZ Fitness"
                        font.pixelSize: window.ui.fontTitle
                        font.weight: Font.DemiBold
                        color: window.ui.textMain
                    }
                    Row {
                        anchors.left: drawerAppName.left
                        anchors.right: drawerAppName.right
                        anchors.top: drawerAppName.bottom
                        anchors.topMargin: 2
                        spacing: 6
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 8
                            height: 8
                            radius: 4
                            color: window.equipmentConnected ? window.ui.ok : window.ui.textMuted
                        }
                        Label {
                            width: parent.width - 14
                            // "Connecting..." - the untranslated start value of homeform::m_info, kept until the
                            // equipment writes its own status: shown as the search, or as connected next to the green dot
                            text: (typeof rootItem !== "undefined" && rootItem) ? (rootItem.info !== "Connecting..." ? rootItem.info
                                  : window.equipmentConnected ? qsTranslate("HomeForm.ui", "Device connected")
                                  : qsTranslate("homeform", "Searching for the device...")) : ""
                            font.pixelSize: window.ui.fontCaption
                            color: window.ui.textMuted
                            elide: Text.ElideRight
                        }
                    }

                    // Profile chip
                    Rectangle {
                        id: profileChip
                        x: 24
                        // 82 and 36 at 100 %, lower and taller as the text grows: from the sizes, not
                        // from the labels, which are taller in a CJK fallback font
                        y: 82 + Math.round(1.2 * (window.ui.fontTitle - 20 + window.ui.fontCaption - 13))
                        height: Math.max(36, Math.round(window.ui.fontLabel * 1.2) + 16)
                        width: Math.min(parent.width - 48, profileChipRow.implicitWidth + 28)
                        radius: height / 2
                        color: profileChipArea.pressed ? window.ui.alpha(window.ui.accent, 0.28) : window.ui.alpha(window.ui.accent, 0.16)
                        Row {
                            id: profileChipRow
                            anchors.verticalCenter: parent.verticalCenter
                            x: 12
                            spacing: 8
                            UiIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 18
                                height: 18
                                name: "person"
                                color: window.ui.accent
                            }
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                width: Math.min(implicitWidth, drawer.width - 110)
                                text: qsTr("Profile: ") + settings.profile_name
                                font.pixelSize: window.ui.fontLabel
                                font.weight: Font.Medium
                                color: window.ui.textMain
                                elide: Text.ElideRight
                            }
                        }
                        MouseArea {
                            id: profileChipArea
                            anchors.fill: parent
                            onClicked: drawerAction("profile")
                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("Profile: ") + settings.profile_name
                            Accessible.onPressAction: drawerAction("profile")
                        }
                    }
                }

                Component {
                    id: drawerSectionHeader
                    Label {
                        leftPadding: 28
                        topPadding: 14
                        bottomPadding: 6
                        font.pixelSize: window.ui.fontCaption
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.4
                        color: window.ui.accent
                        // a binding, not set once: it follows a change of the app language
                        text: parent ? parent.title : ""
                    }
                }

                Loader { property string title: qsTr("Workout"); sourceComponent: drawerSectionHeader }
                UiDrawerItem { text: qsTr("Open Train Program"); iconName: "list_alt"; onClicked: drawerAction("trainprogram") }
                UiDrawerItem { text: qsTr("Workout Editor"); iconName: "edit_note"; onClicked: drawerAction("editor") }
                UiDrawerItem { text: qsTr("Open GPX"); iconName: "route"; onClicked: drawerAction("opengpx") }
                UiDrawerItem { text: qsTr("Charts"); iconName: "bar_chart"; onClicked: drawerAction("charts") }
                UiDrawerItem { text: qsTr("Workouts History"); iconName: "history"; onClicked: drawerAction("history") }
                UiDrawerItem { text: qsTr("Save FIT"); iconName: "download"; onClicked: drawerAction("savefit") }
                UiDrawerItem { text: qsTr("Save GPX"); iconName: "download"; onClicked: drawerAction("savegpx") }

                Rectangle { x: 28; width: parent.width - 56; height: 1; color: window.ui.outline; opacity: 0.6 }

                Loader { property string title: qsTr("App"); sourceComponent: drawerSectionHeader }
                UiDrawerItem { text: qsTr("Settings"); iconName: "settings"; onClicked: drawerAction("settings") }
                UiDrawerItem { text: qsTr("Wizard"); iconName: "rocket_launch"; onClicked: drawerAction("wizard") }
                UiDrawerItem { text: qsTr("Swag Bag"); iconName: "redeem"; onClicked: drawerAction("swagbag") }
                UiDrawerItem { text: qsTr("Help"); iconName: "help"; external: true; onClicked: drawerAction("help") }
                UiDrawerItem { text: qsTr("Community"); iconName: "groups"; external: true; onClicked: drawerAction("community") }
                UiDrawerItem { text: qsTr("Credits"); iconName: "info"; onClicked: drawerAction("credits") }
                UiDrawerItem { text: qsTr("Quit"); iconName: "power_settings_new"; visible: OS_VERSION === "Other"; onClicked: drawerAction("quit") }

                Rectangle { x: 28; width: parent.width - 56; height: 1; color: window.ui.outline; opacity: 0.6 }

                Loader { property string title: qsTr("Services"); sourceComponent: drawerSectionHeader }

                Grid {
                    x: 16
                    width: parent.width - 32
                    columns: 2
                    spacing: 8
                    Repeater {
                        model: [
                            { key: "strava", image: "icons/icons/btn_strava_connectwith_orange.png" },
                            { key: "peloton", image: "icons/icons/Button_Connect_Rect_DarkMode.png" },
                            { key: "garmin", image: "icons/icons/garmin-connect-badge.png" },
                            { key: "intervals", image: "icons/icons/intervals-logo-with-name.png" }
                        ]
                        delegate: Rectangle {
                            width: (parent.width - 8) / 2
                            height: 52
                            radius: 14
                            color: serviceArea.pressed ? window.ui.surfaceHighest : window.ui.surfaceHigh
                            Image {
                                anchors.fill: parent
                                anchors.margins: 8
                                source: modelData.image
                                fillMode: Image.PreserveAspectFit
                                smooth: true
                            }
                            MouseArea {
                                id: serviceArea
                                anchors.fill: parent
                                onClicked: drawerAction(modelData.key)
                                Accessible.role: Accessible.Button
                                // The login state has no change signal: read again on every
                                // opening of the drawer
                                Accessible.name: window.ui.modern && drawer.opened ? modernDrawerList.serviceName(modelData.key) : ""
                                Accessible.onPressAction: drawerAction(modelData.key)
                            }
                        }
                    }
                }

                Label {
                    width: parent.width
                    leftPadding: 28
                    topPadding: 16
                    text: drawerVersion.text
                    font.pixelSize: window.ui.fontCaption
                    color: window.ui.textMuted
                }
            }
        }
    }

    // Wrapper Item to prevent ApplicationWindow from capturing all VoiceOver focus
    Item {
        anchors.fill: parent
        Accessible.ignored: true
        opacity: window.rootReady ? 1 : 0

        StackView {
            id: stackView
            initialItem: "Home.qml"
            // The home page has no load and save buttons, whichever way it is reached (a profile
            // saved, a settings file loaded): with them shown the lock, map, video and chart hide
            onDepthChanged: {
                if (depth === 1) {
                    toolButtonLoadSettings.visible = false;
                    toolButtonSaveSettings.visible = false;
                }
            }
            anchors.fill: parent
            // Below the toolbar, which hangs over the pages (headerToolbar)
            anchors.topMargin: headerToolbar.contentHeight
            // Clear of the bottom system bar in any orientation: held sideways with gesture
            // navigation the bar stays at the bottom (with buttons it moves to the side, and the
            // bottom inset is 0 then)
            anchors.bottomMargin: getBottomPadding()
            anchors.rightMargin: getRightPadding()
            anchors.leftMargin: getLeftPadding()
            focus: true
            Connections {
                target: stackView.currentItem
                ignoreUnknownSignals: true
                function onPeloton_connect_clicked() {
                    if (rootItem.isPelotonLoggedIn()) {
                        pelotonLogoutConfirm.visible = true
                    } else {
                        stackView.push("WebPelotonAuth.qml")
                        stackView.currentItem.goBack.connect(function() {
                            stackView.pop();
                        })
                        peloton_connect_clicked()
                    }
                }
            }
            Keys.onVolumeUpPressed: (event)=> { console.log("onVolumeUpPressed"); volumeUp(); event.accepted = settings.volume_change_gears; }
            Keys.onVolumeDownPressed: (event)=> { console.log("onVolumeDownPressed"); volumeDown(); event.accepted = settings.volume_change_gears; }
            Keys.onPressed: (event)=> {
                if (event.key === Qt.Key_MediaPrevious) {
                    keyMediaPrevious();
                    event.accepted = true;
                } else if (event.key === Qt.Key_MediaNext) {
                    keyMediaNext();
                    event.accepted = true;
                } else {
                    event.accepted = false;
                }
            }

            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_speed_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_speed_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_inclination_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_inclination_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_resistance_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_resistance_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_resistance_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("peloton_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_resistance_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("peloton_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_resistance_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_resistance_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_resistance") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_power_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_power") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_power_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_power") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_zone_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_zone") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_zone_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_zone") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_speed_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_speed_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_speed") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_incline_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("target_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_target_incline_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("target_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_fan_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("fan") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_fan_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("fan") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_offset_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("peloton_offset") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_offset_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("peloton_offset") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_remaining_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("peloton_remaining") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_peloton_remaining_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("peloton_remaining") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_remaining_time_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("remainingtimetrainprogramrow") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_remaining_time_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("remainingtimetrainprogramrow") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_gears_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("gears") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_gears_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("gears") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_pid_hr_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("pid_hr") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_pid_hr_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("pid_hr") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_ext_incline_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardPlus("external_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_ext_incline_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardMinus("external_inclination") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_biggears_plus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("biggearsplus") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_biggears_minus; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("biggearsminus") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_avs_cruise; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("autoVirtualShiftingCruise") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_avs_climb; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("autoVirtualShiftingClimb") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_avs_sprint; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("autoVirtualShiftingSprint") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_power_avg; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("powerAvg") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_erg_mode; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("erg_mode") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_resistance_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_resistance_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_speed_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_speed_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_inclination_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_inclination_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_1; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_1") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_2; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_2") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_3; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_3") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_4; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_4") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_5; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_5") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_6; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_6") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_preset_powerzone_7; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLargeButton("preset_powerzone_7") }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_auto_resistance; enabled: shortcutReady(sequence); onActivated: rootItem.setAutoResistance(!rootItem.autoResistance) }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_lap; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardLap() }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_start_stop; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardStartStop() }
            Shortcut { context: Qt.WindowShortcut; sequence: settings.shortcut_stop; enabled: shortcutReady(sequence); onActivated: rootItem.keyboardStop() }
        }

        SupportHelpButton {
            activeOnHome: window.rootReady && stackView.depth === 1 && !window.lockTiles
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 16 + window.getRightPadding()
            anchors.bottomMargin: 16 + window.getBottomPadding()
        }
    }

    // After the pages, not before them: TalkBack finds the item under a finger among the window's
    // items from the last declared to the first, whatever their z (Qt 5.15; Qt 6.10.3 and 6.11
    // take the drawing order, QTBUG-137823, qtdeclarative 292896c799). Declared before the pages, a
    // touch on the bar reached the tile or the support button under it. Anything else with buttons drawn
    // over the pages and not in a Popup goes here too (the toasts and the lap prompt above have none).
    HeartRateSuggestion {}
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
