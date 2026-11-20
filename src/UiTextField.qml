import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtQuick.Controls 2.12
import QtQuick.Controls.impl 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

// TextField of the settings pages and the lists: the Qt 5.15 Material TextField, copied as
// is (qtquickcontrols2 5.15, src/imports/controls/material/TextField.qml), so nothing
// changes with the modern look off. Modern look: a filled field with rounded corners, like
// the ones of the workout editor, instead of the underline; the accent ring shows focus.
T.TextField {
    id: control

    readonly property bool modern: window.ui.modern
    readonly property int modernInset: modern ? 6 : 0

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                   || Math.max(contentWidth, placeholder.implicitWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    // Modern: 48 tall with the 6 inset of Material buttons and combo boxes, so the visible
    // field is 36 like the OK button and the combo box next to it
    topInset: modernInset
    bottomInset: modernInset
    topPadding: modern ? 12 : 8
    bottomPadding: modern ? 12 : 16
    leftPadding: modern ? 14 : padding
    rightPadding: modern ? 14 : padding

    color: enabled ? Material.foreground : Material.hintTextColor
    selectionColor: Material.accentColor
    selectedTextColor: Material.primaryHighlightedTextColor
    placeholderTextColor: modern ? window.ui.textMuted : Material.hintTextColor
    verticalAlignment: TextInput.AlignVCenter

    // What a value may be – a number, a time, an address – is checked by the field that uses
    // this one (SettingsNumberField, SettingsFormatField); here only how the modern look
    // shows them.
    // A number field (set by the caller): modern look – the narrow width, a number
    // fits in 88, the 120 of the text fields left the long labels next to "75" or "35" wrapping
    property bool numberField: false
    readonly property bool narrow: modern && numberField && !timeShaped
    // The value is not valid (set by the caller): modern look –
    // a red ring around the field; the classic look draws its red line itself
    property bool invalid: false
    // A time field ("04:12:00"); set only when the text shows it: a page may bind it itself (the
    // search results field is created with no text), an assignment of false would break that
    // binding
    property bool timeShaped: false
    // "h:mm" or "h:mm:ss": digits only, the hours any length, minutes and seconds two digits
    function isTime(s) {
        var parts = String(s).split(":")
        if (parts.length < 2 || parts.length > 3)
            return false
        for (var i = 0; i < parts.length; i++) {
            var p = parts[i]
            if (p.length === 0 || (i > 0 && p.length !== 2))
                return false
            for (var j = 0; j < p.length; j++) {
                if (p[j] < "0" || p[j] > "9")
                    return false
            }
        }
        return true
    }
    Component.onCompleted: {
        if (!timeShaped && isTime(text))
            timeShaped = true
    }

    // Modern look: a time field ("00:32:00", the paces) is picked on wheels instead of typed:
    // the text keyboard had no digits row and the digit one no colon, and a bare "320" was
    // saved as garbage. The picker writes the text; the OK button of the row saves it as before
    readOnly: modern && timeShaped
    TapHandler {
        enabled: control.modern && control.timeShaped && control.enabled
        onTapped: pickerLoader.item.openFor(control.text)
    }
    // Made only for the time fields: a settings page has hundreds of text fields
    Loader {
        id: pickerLoader
        active: control.modern && control.timeShaped
        sourceComponent: pickerComponent
    }
    Component {
        id: pickerComponent
    UiPopup {
        id: timePicker
        parent: Overlay.overlay
        modal: true
        // Placed by x/y, not anchors.centerIn: centred by anchors, the popup grew out of the
        // corner while it opened. The wheels have a fixed size, so the height is known before
        // the first open and still follows the text of the labels and buttons; the width
        // never runs past a narrow screen
        x: Math.round((parent.width - width) / 2)
        y: Math.max(0, Math.round((parent.height - height) / 2))
        width: Math.min(292, parent.width - 16)
        height: Math.min(topPadding + contentItem.implicitHeight + bottomPadding, parent.height)
        padding: 16
        property bool withSeconds: true

        function openFor(value) {
            var parts = String(value).split(":").map(function (v) { return parseInt(v, 10) || 0 })
            withSeconds = parts.length > 2
            hours.currentIndex = Math.min(parts[0], 99)
            minutes.currentIndex = Math.min(parts[1] || 0, 59)
            seconds.currentIndex = withSeconds ? Math.min(parts[2] || 0, 59) : 0
            open()
        }
        function pad(n) { return n < 10 ? "0" + n : "" + n }

        contentItem: Column {
            spacing: 12
            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 4
                // Which wheel is what: "00" alone does not say hours or minutes
                Row {
                    spacing: 4
                    Label { text: qsTr("hours"); width: 64; horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: window.ui.fontCaption; color: window.ui.textMuted }
                    Item { width: timeColon.width; height: 1 }
                    Label { text: qsTr("min"); width: 64; horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: window.ui.fontCaption; color: window.ui.textMuted }
                    Item { width: timeColon.width; height: 1; visible: timePicker.withSeconds }
                    Label { text: qsTr("sec"); width: 64; horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: window.ui.fontCaption; color: window.ui.textMuted; visible: timePicker.withSeconds }
                }
                Row {
                    spacing: 4
                    height: 200
                    Tumbler { id: hours; model: 100; visibleItemCount: 5; width: 64; height: 200
                              delegate: timeDigit; Accessible.name: qsTr("hours") + ": " + currentIndex }
                    Label { id: timeColon; text: ":"; font.pixelSize: window.ui.fontPx(24); anchors.verticalCenter: parent.verticalCenter }
                    Tumbler { id: minutes; model: 60; visibleItemCount: 5; width: 64; height: 200
                              delegate: timeDigit; Accessible.name: qsTr("min") + ": " + currentIndex }
                    Label { text: ":"; font.pixelSize: window.ui.fontPx(24); visible: timePicker.withSeconds
                            anchors.verticalCenter: parent.verticalCenter }
                    Tumbler { id: seconds; model: 60; visibleItemCount: 5; width: 64; height: 200
                              visible: timePicker.withSeconds; delegate: timeDigit; Accessible.name: qsTr("sec") + ": " + currentIndex }
                }
            }
            Row {
                anchors.right: parent.right
                spacing: 8
                UiButton { text: qsTranslate("settings", "Cancel"); flat: true; onClicked: timePicker.close() }
                UiButton {
                    text: qsTranslate("settings", "OK")
                    highlighted: true
                    onClicked: {
                        control.text = timePicker.pad(hours.currentIndex) + ":" + timePicker.pad(minutes.currentIndex)
                                       + (timePicker.withSeconds ? ":" + timePicker.pad(seconds.currentIndex) : "")
                        timePicker.close()
                    }
                }
            }
        }
    }
    }
    Component {
        id: timeDigit
        Label {
            text: index < 10 ? "0" + index : index
            font.pixelSize: window.ui.fontHeadline
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: Tumbler.tumbler && Tumbler.tumbler.currentIndex === index ? window.ui.accent : window.ui.textMain
            opacity: 1.0 - Math.abs(Tumbler.displacement) / (Tumbler.tumbler ? Tumbler.tumbler.visibleItemCount / 2 : 3)
        }
    }
    cursorDelegate: CursorDelegate { }

    PlaceholderText {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)
        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        verticalAlignment: control.verticalAlignment
        elide: Text.ElideRight
        renderType: control.renderType
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
    }

    background: Item {
        implicitWidth: control.narrow ? 88 : 120
        implicitHeight: control.modern ? 48 - 2 * control.modernInset : 0

        // Classic: the Material underline
        Rectangle {
            visible: !control.modern
            y: control.height - height - control.bottomPadding + 8
            width: parent.width
            height: control.activeFocus || control.hovered ? 2 : 1
            color: control.activeFocus ? control.Material.accentColor
                                       : (control.hovered ? control.Material.primaryTextColor : control.Material.hintTextColor)
        }

        // Modern: a filled rounded field, the accent ring while it has the focus, a red one
        // while the value is not valid
        UiFrame {
            visible: control.modern
            anchors.fill: parent
            radius: 12
            fill: window.ui.surfaceHighest
            stroke: control.invalid ? window.ui.danger : window.ui.accent
            strokeWidth: control.activeFocus || control.invalid ? 2 : 0
            opacity: control.enabled ? 1 : 0.5
        }
    }
}
