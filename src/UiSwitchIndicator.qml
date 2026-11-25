import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

// Switch indicator of UiSwitch and UiSwitchDelegate. Classic look: the Material
// SwitchIndicator of Qt 5.15 as is (a thin track under a bigger handle). Modern look: the
// Material 3 switch - a 52x32 track with the handle inside it: outlined with a small grey
// handle when off, filled with the accent and a bigger handle when on.
Item {
    id: indicator

    property Item control
    readonly property bool modern: window.ui.modern
    // For the ripple of the classic Switch, which is centred on the handle
    readonly property Item handle: classic.handle

    implicitWidth: modern ? 52 : classic.implicitWidth
    implicitHeight: modern ? 32 : classic.implicitHeight

    SwitchIndicator {
        id: classic
        anchors.fill: parent
        visible: !indicator.modern
        control: indicator.control
    }

    UiFrame {
        id: track
        visible: indicator.modern
        anchors.fill: parent
        radius: height / 2
        opacity: indicator.control && indicator.control.enabled ? 1 : 0.38
        readonly property bool on: indicator.control ? indicator.control.checked : false
        base: window.ui.bg
        fill: on ? window.ui.accent : window.ui.surfaceHighest
        stroke: window.ui.outline
        strokeWidth: on ? 0 : 2

        Rectangle {
            id: thumb
            readonly property real size: indicator.control && indicator.control.pressed ? 28 : (track.on ? 24 : 16)
            width: size
            height: size
            radius: size / 2
            anchors.verticalCenter: parent.verticalCenter
            // visualPosition follows the finger while the switch is dragged
            x: {
                var pos = indicator.control ? indicator.control.visualPosition : 0
                var from = (track.height - size) / 2
                var to = track.width - track.height + from
                return from + (to - from) * pos
            }
            color: track.on ? window.ui.accentInk : window.ui.textMuted

            Behavior on x {
                enabled: indicator.control && !indicator.control.pressed
                NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
            }
            Behavior on width { NumberAnimation { duration: 100 } }
            Behavior on height { NumberAnimation { duration: 100 } }

            UiIcon {
                anchors.centerIn: parent
                width: 16
                height: 16
                name: "check"
                color: window.ui.accent
                visible: track.on && thumb.width >= 22
            }
        }
    }
}
