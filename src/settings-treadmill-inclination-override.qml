import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    contentWidth: -1
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
            TextField {
                id: treadmillOverrideGainTextField
                text: settings.treadmill_inclination_ovveride_gain
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_ovveride_gain = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_ovveride_gain = treadmillOverrideGainTextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }

        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Inclination Override Offset:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverrideOffsetTextField
                text: settings.treadmill_inclination_ovveride_offset
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_ovveride_offset = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_ovveride_offset = treadmillOverrideOffsetTextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }

        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride0TextField
                text: settings.treadmill_inclination_override_0
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_0 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_0 = treadmillOverride0TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 0.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride05TextField
                text: settings.treadmill_inclination_override_05
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_05 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_05 = treadmillOverride05TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 1.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride10TextField
                text: settings.treadmill_inclination_override_10
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_10 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_10 = treadmillOverride10TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 1.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride15TextField
                text: settings.treadmill_inclination_override_15
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_15 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_15 = treadmillOverride15TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 2.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride20TextField
                text: settings.treadmill_inclination_override_20
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_20 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_20 = treadmillOverride20TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 2.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride25TextField
                text: settings.treadmill_inclination_override_25
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_25 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_25 = treadmillOverride25TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 3.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride30TextField
                text: settings.treadmill_inclination_override_30
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_30 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_30 = treadmillOverride30TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 3.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride35TextField
                text: settings.treadmill_inclination_override_35
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_35 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_35 = treadmillOverride35TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 4.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride40TextField
                text: settings.treadmill_inclination_override_40
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_40 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_40 = treadmillOverride40TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 4.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride45TextField
                text: settings.treadmill_inclination_override_45
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_45 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_45 = treadmillOverride45TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 5.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride50TextField
                text: settings.treadmill_inclination_override_50
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_50 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_50 = treadmillOverride50TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 5.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride55TextField
                text: settings.treadmill_inclination_override_55
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_55 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_55 = treadmillOverride55TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 6.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride60TextField
                text: settings.treadmill_inclination_override_60
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_60 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_60 = treadmillOverride60TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 6.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride65TextField
                text: settings.treadmill_inclination_override_65
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_65 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_65 = treadmillOverride65TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 7.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride70TextField
                text: settings.treadmill_inclination_override_70
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_70 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_70 = treadmillOverride70TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 7.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride75TextField
                text: settings.treadmill_inclination_override_75
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_75 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_75 = treadmillOverride75TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 8.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride80TextField
                text: settings.treadmill_inclination_override_80
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_80 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_80 = treadmillOverride80TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 8.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride85TextField
                text: settings.treadmill_inclination_override_85
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_85 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_85 = treadmillOverride85TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 9.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride90TextField
                text: settings.treadmill_inclination_override_90
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_90 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_90 = treadmillOverride90TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 9.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride95TextField
                text: settings.treadmill_inclination_override_95
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_95 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_95 = treadmillOverride95TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 10.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride100TextField
                text: settings.treadmill_inclination_override_100
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_100 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_100 = treadmillOverride100TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 10.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride105TextField
                text: settings.treadmill_inclination_override_105
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_105 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_105 = treadmillOverride105TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 11.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride110TextField
                text: settings.treadmill_inclination_override_110
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_110 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_110 = treadmillOverride110TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 11.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride115TextField
                text: settings.treadmill_inclination_override_115
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_115 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_115 = treadmillOverride115TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 12.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride120TextField
                text: settings.treadmill_inclination_override_120
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_120 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_120 = treadmillOverride120TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 12.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride125TextField
                text: settings.treadmill_inclination_override_125
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_125 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_125 = treadmillOverride125TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 13.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride130TextField
                text: settings.treadmill_inclination_override_130
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_130 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_130 = treadmillOverride130TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 13.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride135TextField
                text: settings.treadmill_inclination_override_135
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_135 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_135 = treadmillOverride135TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 14.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride140TextField
                text: settings.treadmill_inclination_override_140
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_140 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_140 = treadmillOverride140TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 14.5%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride145TextField
                text: settings.treadmill_inclination_override_145
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_145 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_145 = treadmillOverride145TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
        RowLayout {
            spacing: 10
            Label {
                text: qsTr("Override 15.0%:")
                Layout.fillWidth: true
            }
            TextField {
                id: treadmillOverride150TextField
                text: settings.treadmill_inclination_override_150
                horizontalAlignment: Text.AlignRight
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                onAccepted: settings.treadmill_inclination_override_150 = text
                onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
            }
            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: {settings.treadmill_inclination_override_150 = treadmillOverride150TextField.text; toast.show(qsTr("Setting saved!")); }
            }
        }
    }
}

