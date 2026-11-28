import QtQuick 2.7
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

ScrollView {
    // Modern look: the rows Body Large, as on the settings page
    font: window.ui.modern ? window.ui.rowFont : window.font
    // Buttons and lists of this page take Label Large (window.ui.onTypeScalePage)
    readonly property bool typeScalePage: true
    contentWidth: -1
    // Modern look: with the column's top and bottom margins, not in its implicit height (the
    // end of the page was cut off). The classic look keeps the height ScrollView takes itself
    Binding on contentHeight { when: window.ui.modern; value: mainLayout.implicitHeight + 20; restoreMode: Binding.RestoreBindingOrValue }
    focus: true
    anchors.fill: parent
    id: settingsShortcutsPane
    objectName: "settingsShortcutsPane"

    Component.onCompleted: {
        if (typeof rootItem !== "undefined" && rootItem) {
            rootItem.setNativeShortcutCaptureSuspended(true)
        }
    }

    Component.onDestruction: {
        if (typeof rootItem !== "undefined" && rootItem) {
            rootItem.setNativeShortcutCaptureSuspended(false)
        }
    }

    // The Settings object of settings.qml, given by the page that opens this one: one object
    // for all the settings pages, so a page no longer writes back an old copy of what another
    // page changed
    property var settings

    ColumnLayout {
        id: mainLayout
        // Modern look: 8 between the groups, as Material 3
        spacing: window.ui.modern ? 8 : 10
        // Modern look: no bottom anchor - the height comes from the column itself, which the
        // contentHeight above reads (anchored to fill, the column looped on it)
        anchors.fill: window.ui.modern ? undefined : parent
        anchors.left: window.ui.modern ? parent.left : undefined
        anchors.right: window.ui.modern ? parent.right : undefined
        anchors.top: window.ui.modern ? parent.top : undefined
        anchors.margins: 10
        // Modern look: inside the frames, which run framePadH (16) further out, on the page
        // margin 16
        anchors.leftMargin: window.ui.modern ? Math.max(16, window.contentSideMargin) + 16 : window.contentSideMargin
        anchors.rightMargin: window.ui.modern ? Math.max(16, window.contentSideMargin) + 16 : window.contentSideMargin

        // Modern look: the switch and its description in a frame of their own; the title and the
        // groups (framed by themselves) are left out
        UiSettingFrames {
            content: mainLayout
            exclude: [shortcutsTitle, generalBox, metricsBox, targetBox, pelotonBox, presetBox,
                      presetSpeedBox, presetInclinationBox, presetPowerZoneBox]
            Layout.preferredHeight: 0
        }

        Label {
            id: shortcutsTitle
            text: qsTr("Keyboard Shortcuts")
            // Modern look: a page title like the TTS page instead of the red caption
            font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
            font.weight: window.ui.modern ? Font.DemiBold : Font.Bold
            color: window.ui.ink(Material.color(Material.Red))
            // The layout margin is 10: 2 more puts the title 12 from the top, as on Tiles and TTS
            topPadding: window.ui.modern ? 2 : 0
        }

        UiSwitchDelegate {
            // on iOS the title of a switch kept the size of the style while the text around it grew: with a size other than 100 % it takes the one of the window font
            Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.rowScaledPx; restoreMode: Binding.RestoreBindingOrValue }
            id: shortcutsEnabledDelegate
            // Modern look: no side padding, so the label lines up with the hint and the switch
            // with the key fields, as on the TTS page; the classic look as on master
            Binding on leftPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
            Binding on rightPadding { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
            text: qsTr("Enable Keyboard Shortcuts")
            checked: settings.shortcuts_enabled
            Layout.fillWidth: true
            onClicked: settings.shortcuts_enabled = checked
        }

        Label {
            text: qsTr("Click on a field and press a key to assign a shortcut. Press Backspace to clear. Preset fields follow the same left-to-right order as the preset buttons.")
            font.italic: !window.ui.modern
            font.pixelSize: window.fontHint
            bottomPadding: window.hintGap
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            color: window.ui.modern ? window.ui.textMuted : Material.color(Material.Lime)
        }

        component ShortcutField : UiTextField {
            id: shortcutField
            property string settingName: ""
            placeholderText: qsTr("None")
            // Modern look: a dash when the word does not fit the field (at 170 % the Ukrainian word
            // was elided); the screen reader still reads the word. The width comes from the
            // layout (Layout.preferredWidth), not from the placeholder
            shownPlaceholder: window.ui.modern && placeholderMetrics.advanceWidth > width - leftPadding - rightPadding ? "–" : placeholderText
            TextMetrics {
                id: placeholderMetrics
                font: shortcutField.font
                text: shortcutField.placeholderText
            }
            // Modern: the letter and "None" in the middle, with room for the whole word
            horizontalAlignment: window.ui.modern ? Text.AlignHCenter : Text.AlignRight
            leftPadding: window.ui.modern ? 6 : padding
            rightPadding: window.ui.modern ? 6 : padding
            // Modern: one letter needs less than 100; three of them (the AVS row) left no
            // room for the label on a phone
            Layout.preferredWidth: window.ui.modern ? 76 : 100
            // Modern look: the key in Label Large 14 - Body Large of the page did not fit "None"
            // in a field of a row of five. The page is made while the look is set, and the
            // look is switched on the settings page under it
            Binding on font.pixelSize { when: window.ui.modern; value: window.ui.fontLabelLarge; restoreMode: Binding.RestoreBindingOrValue }
            selectByMouse: false
            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData | Qt.ImhNoAutoUppercase

            MouseArea {
                anchors.fill: parent
                onClicked: parent.forceActiveFocus()
            }

            onActiveFocusChanged: {
                if (activeFocus) {
                    cursorPosition = text.length
                }
            }

            onTextEdited: {
                // Handles soft keyboard input (iOS/Android)
                if (text.length === 0) {
                    settings[settingName] = ""
                } else {
                    var lastChar = text[text.length - 1].toUpperCase()
                    settings[settingName] = lastChar
                    text = lastChar
                }
            }

            Keys.onPressed: (event) => {
                if (event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) {
                    settings[settingName] = ""
                    text = ""
                } else if (event.text !== "") {
                    settings[settingName] = event.text.toUpperCase()
                    text = event.text.toUpperCase()
                }
                event.accepted = true
            }
        }

        component PresetShortcutField : ShortcutField {
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            Layout.minimumWidth: 28
            Layout.preferredWidth: 56
            Layout.maximumWidth: 100
        }

        UiGroupBox {
            id: generalBox
            title: qsTr("General Controls")
            framed: true
            // Out to the edges of the frames above it
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Start / Pause"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_start_stop; settingName: "shortcut_start_stop" }
                }
                RowLayout {
                    Label { text: qsTr("Stop"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_stop; settingName: "shortcut_stop" }
                }
                RowLayout {
                    Label { text: qsTr("Lap"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_lap; settingName: "shortcut_lap" }
                }
            }
        }

        UiGroupBox {
            id: metricsBox
            title: qsTr("Main Metrics")
            framed: true
            // Out to the edges of the frames above it
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Speed + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_speed_plus; settingName: "shortcut_speed_plus" }
                    ShortcutField { text: settings.shortcut_speed_minus; settingName: "shortcut_speed_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Inclination + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_inclination_plus; settingName: "shortcut_inclination_plus" }
                    ShortcutField { text: settings.shortcut_inclination_minus; settingName: "shortcut_inclination_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Resistance + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_resistance_plus; settingName: "shortcut_resistance_plus" }
                    ShortcutField { text: settings.shortcut_resistance_minus; settingName: "shortcut_resistance_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Gears + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_gears_plus; settingName: "shortcut_gears_plus" }
                    ShortcutField { text: settings.shortcut_gears_minus; settingName: "shortcut_gears_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Gears Big Buttons + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_biggears_plus; settingName: "shortcut_biggears_plus" }
                    ShortcutField { text: settings.shortcut_biggears_minus; settingName: "shortcut_biggears_minus" }
                }
            }
        }

        UiGroupBox {
            id: targetBox
            title: qsTr("Target Controls")
            framed: true
            // Out to the edges of the frames above it
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Target Resistance + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_target_resistance_plus; settingName: "shortcut_target_resistance_plus" }
                    ShortcutField { text: settings.shortcut_target_resistance_minus; settingName: "shortcut_target_resistance_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Power + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_target_power_plus; settingName: "shortcut_target_power_plus" }
                    ShortcutField { text: settings.shortcut_target_power_minus; settingName: "shortcut_target_power_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Zone + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_target_zone_plus; settingName: "shortcut_target_zone_plus" }
                    ShortcutField { text: settings.shortcut_target_zone_minus; settingName: "shortcut_target_zone_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Speed + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_target_speed_plus; settingName: "shortcut_target_speed_plus" }
                    ShortcutField { text: settings.shortcut_target_speed_minus; settingName: "shortcut_target_speed_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Target Incline + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_target_incline_plus; settingName: "shortcut_target_incline_plus" }
                    ShortcutField { text: settings.shortcut_target_incline_minus; settingName: "shortcut_target_incline_minus" }
                }
            }
        }

        UiGroupBox {
            id: pelotonBox
            title: qsTr("Peloton & Others")
            framed: true
            // Out to the edges of the frames above it
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Label { text: qsTr("Peloton Resistance + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_peloton_resistance_plus; settingName: "shortcut_peloton_resistance_plus" }
                    ShortcutField { text: settings.shortcut_peloton_resistance_minus; settingName: "shortcut_peloton_resistance_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Peloton Offset + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_peloton_offset_plus; settingName: "shortcut_peloton_offset_plus" }
                    ShortcutField { text: settings.shortcut_peloton_offset_minus; settingName: "shortcut_peloton_offset_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Peloton Remaining + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_peloton_remaining_plus; settingName: "shortcut_peloton_remaining_plus" }
                    ShortcutField { text: settings.shortcut_peloton_remaining_minus; settingName: "shortcut_peloton_remaining_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Time to Next + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_remaining_time_plus; settingName: "shortcut_remaining_time_plus" }
                    ShortcutField { text: settings.shortcut_remaining_time_minus; settingName: "shortcut_remaining_time_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Fan Speed + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_fan_plus; settingName: "shortcut_fan_plus" }
                    ShortcutField { text: settings.shortcut_fan_minus; settingName: "shortcut_fan_minus" }
                }
                RowLayout {
                    Label { text: qsTr("PID Heart Rate + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_pid_hr_plus; settingName: "shortcut_pid_hr_plus" }
                    ShortcutField { text: settings.shortcut_pid_hr_minus; settingName: "shortcut_pid_hr_minus" }
                }
                RowLayout {
                    Label { text: qsTr("Ext. Inclination + / -"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_ext_incline_plus; settingName: "shortcut_ext_incline_plus" }
                    ShortcutField { text: settings.shortcut_ext_incline_minus; settingName: "shortcut_ext_incline_minus" }
                }
                RowLayout {
                    Label { text: qsTr("ERG Mode Toggle"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_erg_mode; settingName: "shortcut_erg_mode" }
                }
                RowLayout {
                    Label { text: qsTr("Power Avg Toggle"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_power_avg; settingName: "shortcut_power_avg" }
                }
                RowLayout {
                    Label { text: qsTr("Auto-Resistance Toggle"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_auto_resistance; settingName: "shortcut_auto_resistance" }
                }
                RowLayout {
                    Label { text: qsTr("AVS Cruise / Climb / Sprint"); Layout.fillWidth: true; wrapMode: window.labelWrap }
                    ShortcutField { text: settings.shortcut_avs_cruise; settingName: "shortcut_avs_cruise" }
                    ShortcutField { text: settings.shortcut_avs_climb; settingName: "shortcut_avs_climb" }
                    ShortcutField { text: settings.shortcut_avs_sprint; settingName: "shortcut_avs_sprint" }
                }
            }
        }

        UiGroupBox {
            id: presetBox
            title: qsTr("Preset Resistance")
            framed: true
            // Out to the edges of the frames above it
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                PresetShortcutField { text: settings.shortcut_preset_resistance_1; settingName: "shortcut_preset_resistance_1" }
                PresetShortcutField { text: settings.shortcut_preset_resistance_2; settingName: "shortcut_preset_resistance_2" }
                PresetShortcutField { text: settings.shortcut_preset_resistance_3; settingName: "shortcut_preset_resistance_3" }
                PresetShortcutField { text: settings.shortcut_preset_resistance_4; settingName: "shortcut_preset_resistance_4" }
                PresetShortcutField { text: settings.shortcut_preset_resistance_5; settingName: "shortcut_preset_resistance_5" }
            }
        }

        UiGroupBox {
            id: presetSpeedBox
            title: qsTr("Preset Speed")
            // Modern look: framed as the resistance presets above, not a filled card in a frame
            // of UiSettingFrames
            framed: true
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                PresetShortcutField { text: settings.shortcut_preset_speed_1; settingName: "shortcut_preset_speed_1" }
                PresetShortcutField { text: settings.shortcut_preset_speed_2; settingName: "shortcut_preset_speed_2" }
                PresetShortcutField { text: settings.shortcut_preset_speed_3; settingName: "shortcut_preset_speed_3" }
                PresetShortcutField { text: settings.shortcut_preset_speed_4; settingName: "shortcut_preset_speed_4" }
                PresetShortcutField { text: settings.shortcut_preset_speed_5; settingName: "shortcut_preset_speed_5" }
            }
        }

        UiGroupBox {
            id: presetInclinationBox
            title: qsTr("Preset Inclination")
            // Modern look: framed as the resistance presets above, not a filled card in a frame
            // of UiSettingFrames
            framed: true
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                PresetShortcutField { text: settings.shortcut_preset_inclination_1; settingName: "shortcut_preset_inclination_1" }
                PresetShortcutField { text: settings.shortcut_preset_inclination_2; settingName: "shortcut_preset_inclination_2" }
                PresetShortcutField { text: settings.shortcut_preset_inclination_3; settingName: "shortcut_preset_inclination_3" }
                PresetShortcutField { text: settings.shortcut_preset_inclination_4; settingName: "shortcut_preset_inclination_4" }
                PresetShortcutField { text: settings.shortcut_preset_inclination_5; settingName: "shortcut_preset_inclination_5" }
            }
        }

        UiGroupBox {
            id: presetPowerZoneBox
            title: qsTr("Preset Power Zone")
            // Modern look: framed as the resistance presets above, not a filled card in a frame
            // of UiSettingFrames
            framed: true
            Layout.leftMargin: window.ui.modern ? -16 : 0
            Layout.rightMargin: window.ui.modern ? -16 : 0
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                PresetShortcutField { text: settings.shortcut_preset_powerzone_1; settingName: "shortcut_preset_powerzone_1" }
                PresetShortcutField { text: settings.shortcut_preset_powerzone_2; settingName: "shortcut_preset_powerzone_2" }
                PresetShortcutField { text: settings.shortcut_preset_powerzone_3; settingName: "shortcut_preset_powerzone_3" }
                PresetShortcutField { text: settings.shortcut_preset_powerzone_4; settingName: "shortcut_preset_powerzone_4" }
                PresetShortcutField { text: settings.shortcut_preset_powerzone_5; settingName: "shortcut_preset_powerzone_5" }
                PresetShortcutField { text: settings.shortcut_preset_powerzone_6; settingName: "shortcut_preset_powerzone_6" }
                PresetShortcutField { text: settings.shortcut_preset_powerzone_7; settingName: "shortcut_preset_powerzone_7" }
            }
        }
    }
}
