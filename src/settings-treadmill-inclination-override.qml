import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    contentWidth: -1
    leftPadding: window.contentSideMargin
    rightPadding: window.contentSideMargin
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent
    //anchors.bottom: footerSettings.top
    //anchors.bottomMargin: footerSettings.height + 10
    id: settingsInclinationPane

    Settings {
        id: settings
        property double treadmill_inclination_override_0: 0.0
        property double treadmill_inclination_override_05: 0.5
        property double treadmill_inclination_override_10: 1.0
        property double treadmill_inclination_override_15: 1.5
        property double treadmill_inclination_override_20: 2.0
        property double treadmill_inclination_override_25: 2.5
        property double treadmill_inclination_override_30: 3.0
        property double treadmill_inclination_override_35: 3.5
        property double treadmill_inclination_override_40: 4.0
        property double treadmill_inclination_override_45: 4.5
        property double treadmill_inclination_override_50: 5.0
        property double treadmill_inclination_override_55: 5.5
        property double treadmill_inclination_override_60: 6.0
        property double treadmill_inclination_override_65: 6.5
        property double treadmill_inclination_override_70: 7.0
        property double treadmill_inclination_override_75: 7.5
        property double treadmill_inclination_override_80: 8.0
        property double treadmill_inclination_override_85: 8.5
        property double treadmill_inclination_override_90: 9.0
        property double treadmill_inclination_override_95: 9.5
        property double treadmill_inclination_override_100: 10.0
        property double treadmill_inclination_override_105: 10.5
        property double treadmill_inclination_override_110: 11.0
        property double treadmill_inclination_override_115: 11.5
        property double treadmill_inclination_override_120: 12.0
        property double treadmill_inclination_override_125: 12.5
        property double treadmill_inclination_override_130: 13.0
        property double treadmill_inclination_override_135: 13.5
        property double treadmill_inclination_override_140: 14.0
        property double treadmill_inclination_override_145: 14.5
        property double treadmill_inclination_override_150: 15.0

        property double treadmill_inclination_ovveride_gain: 1.0
        property double treadmill_inclination_ovveride_offset: 0.0
    }


    ColumnLayout {
        id: column1
        spacing: 0
        anchors.fill: parent

        Label {
            Layout.preferredWidth: parent.width
            id: ttsLabel
            //: Section title: table that replaces each incline value the treadmill receives with a user-defined value. Avoid the technical word "override".
            text: qsTr("Treadmill Inclination Overrides")
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
            color: Material.color(Material.Red)
        }

        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Inclination Override Gain:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverrideGainTextField
                signed: true
                text: settings.treadmill_inclination_ovveride_gain
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_ovveride_gain = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverrideGainTextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_ovveride_gain = treadmillOverrideGainTextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }

        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Inclination Override Offset:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverrideOffsetTextField
                signed: true
                text: settings.treadmill_inclination_ovveride_offset
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_ovveride_offset = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverrideOffsetTextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_ovveride_offset = treadmillOverrideOffsetTextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }

        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride0TextField
                signed: true
                text: settings.treadmill_inclination_override_0
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_0 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride0TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_0 = treadmillOverride0TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 0.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride05TextField
                signed: true
                text: settings.treadmill_inclination_override_05
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_05 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride05TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_05 = treadmillOverride05TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 1.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride10TextField
                signed: true
                text: settings.treadmill_inclination_override_10
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_10 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride10TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_10 = treadmillOverride10TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 1.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride15TextField
                signed: true
                text: settings.treadmill_inclination_override_15
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_15 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride15TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_15 = treadmillOverride15TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 2.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride20TextField
                signed: true
                text: settings.treadmill_inclination_override_20
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_20 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride20TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_20 = treadmillOverride20TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 2.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride25TextField
                signed: true
                text: settings.treadmill_inclination_override_25
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_25 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride25TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_25 = treadmillOverride25TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 3.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride30TextField
                signed: true
                text: settings.treadmill_inclination_override_30
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_30 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride30TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_30 = treadmillOverride30TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 3.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride35TextField
                signed: true
                text: settings.treadmill_inclination_override_35
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_35 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride35TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_35 = treadmillOverride35TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 4.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride40TextField
                signed: true
                text: settings.treadmill_inclination_override_40
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_40 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride40TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_40 = treadmillOverride40TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 4.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride45TextField
                signed: true
                text: settings.treadmill_inclination_override_45
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_45 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride45TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_45 = treadmillOverride45TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 5.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride50TextField
                signed: true
                text: settings.treadmill_inclination_override_50
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_50 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride50TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_50 = treadmillOverride50TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 5.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride55TextField
                signed: true
                text: settings.treadmill_inclination_override_55
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_55 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride55TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_55 = treadmillOverride55TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 6.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride60TextField
                signed: true
                text: settings.treadmill_inclination_override_60
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_60 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride60TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_60 = treadmillOverride60TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 6.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride65TextField
                signed: true
                text: settings.treadmill_inclination_override_65
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_65 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride65TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_65 = treadmillOverride65TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 7.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride70TextField
                signed: true
                text: settings.treadmill_inclination_override_70
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_70 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride70TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_70 = treadmillOverride70TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 7.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride75TextField
                signed: true
                text: settings.treadmill_inclination_override_75
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_75 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride75TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_75 = treadmillOverride75TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 8.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride80TextField
                signed: true
                text: settings.treadmill_inclination_override_80
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_80 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride80TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_80 = treadmillOverride80TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 8.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride85TextField
                signed: true
                text: settings.treadmill_inclination_override_85
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_85 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride85TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_85 = treadmillOverride85TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 9.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride90TextField
                signed: true
                text: settings.treadmill_inclination_override_90
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_90 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride90TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_90 = treadmillOverride90TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 9.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride95TextField
                signed: true
                text: settings.treadmill_inclination_override_95
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_95 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride95TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_95 = treadmillOverride95TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 10.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride100TextField
                signed: true
                text: settings.treadmill_inclination_override_100
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_100 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride100TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_100 = treadmillOverride100TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 10.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride105TextField
                signed: true
                text: settings.treadmill_inclination_override_105
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_105 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride105TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_105 = treadmillOverride105TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 11.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride110TextField
                signed: true
                text: settings.treadmill_inclination_override_110
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_110 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride110TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_110 = treadmillOverride110TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 11.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride115TextField
                signed: true
                text: settings.treadmill_inclination_override_115
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_115 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride115TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_115 = treadmillOverride115TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 12.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride120TextField
                signed: true
                text: settings.treadmill_inclination_override_120
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_120 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride120TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_120 = treadmillOverride120TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 12.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride125TextField
                signed: true
                text: settings.treadmill_inclination_override_125
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_125 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride125TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_125 = treadmillOverride125TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 13.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride130TextField
                signed: true
                text: settings.treadmill_inclination_override_130
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_130 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride130TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_130 = treadmillOverride130TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 13.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride135TextField
                signed: true
                text: settings.treadmill_inclination_override_135
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_135 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride135TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_135 = treadmillOverride135TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 14.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride140TextField
                signed: true
                text: settings.treadmill_inclination_override_140
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_140 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride140TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_140 = treadmillOverride140TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 14.5%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride145TextField
                signed: true
                text: settings.treadmill_inclination_override_145
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_145 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride145TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_145 = treadmillOverride145TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                //: Row label in the incline replacement table: the value entered next to it is sent to the treadmill instead of this incline.
                text: qsTr("Override 15.0%:")
                Layout.fillWidth: true
            }
            SettingsNumberField {
                id: treadmillOverride150TextField
                signed: true
                text: settings.treadmill_inclination_override_150
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_150 = value
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                enabled: treadmillOverride150TextField.valid
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_150 = treadmillOverride150TextField.value; toast.show(qsTr("Setting saved!")); }
            }
        }
    }
}

