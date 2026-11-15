import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

// Progress of a long process with a known count, for the top of a list: what runs on the left,
// "12 / 40" on the right, a flat 4 dp bar under them (the filled part over a pale track of the
// same colour). It opens and closes by its height (200 ms), so the list under it moves smoothly
// instead of jumping under a finger, and holds the last count for a second after the process
// ends (total back to 0) before it closes.
// In a layout: put it right above the list, with the top margin the list had, and give the list
// a top margin of minus the layout spacing while this is visible - the gap above the list and the
// spacing then do not count twice, and the list does not move when this appears at height 0.
Item {
    id: control

    property int done: 0
    property int total: 0
    // false hides it for good (the other look of the page)
    property bool shown: true
    // above the text; the 12 below the bar to the list are part of the height
    property int topPadding: 0
    property int bottomPadding: 12
    property string text: qsTr("Restoring files of the previous installation")

    // the count on the screen: the last one of the running pass, kept while the block closes
    property int shownDone: 0
    property int shownTotal: 0
    // true from the first count to the end of the hold after the pass
    property bool active: false
    readonly property bool open: shown && active
    readonly property color barColor: window.ui.modern ? window.ui.accent : Material.accent

    // done and total change with one signal of homeform, and the two bindings are updated one after
    // the other: in between the pair is half new (done 0 with the old total at the end of the pass).
    // Within a pass the count only grows, so the shown count never goes back - the half new pair
    // changes nothing on the screen, whichever of the two comes first. A pass starts from 0 again.
    function sync() {
        if (total > 0) {
            if (!active || holdTimer.running)
                shownDone = 0
            holdTimer.stop()
            shownDone = Math.max(shownDone, Math.min(done, total))
            shownTotal = total
            active = true
        } else if (active && !holdTimer.running) {
            holdTimer.start()
        }
    }
    onDoneChanged: sync()
    onTotalChanged: sync()
    // a page opened in the middle of a pass: the handlers above do not run for the first values
    Component.onCompleted: sync()

    // not a sync of state: only how long the last count stays on the screen after the pass
    Timer {
        id: holdTimer
        interval: 1000
        onTriggered: control.active = false
    }

    implicitHeight: open ? topPadding + content.implicitHeight + bottomPadding : 0
    Behavior on implicitHeight { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
    opacity: open ? 1 : 0
    Behavior on opacity { NumberAnimation { duration: 200 } }
    visible: shown && implicitHeight > 0
    clip: true

    Accessible.role: Accessible.ProgressBar
    Accessible.name: qsTr("Restoring files of the previous installation: %1 of %2").arg(shownDone).arg(shownTotal)

    ColumnLayout {
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.topMargin: control.topPadding
        spacing: 4

        RowLayout {
            id: row
            Layout.fillWidth: true
            spacing: 16

            Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                text: control.text
                wrapMode: Text.WordWrap
                font.pixelSize: window.ui.modern ? window.ui.fontCaption : window.fontPx(12)
                color: window.ui.modern ? window.ui.textMuted : Material.foreground
                // the bar reads text and count as its name
                Accessible.ignored: true
            }

            Label {
                Layout.alignment: Qt.AlignTop
                text: control.shownDone + " / " + control.shownTotal
                font.pixelSize: window.ui.modern ? window.ui.fontCaption : window.fontPx(12)
                font.weight: Font.Medium
                color: window.ui.modern ? window.ui.textMain : Material.foreground
                Accessible.ignored: true
            }
        }

        Item {
            id: bar
            Layout.fillWidth: true
            Layout.preferredHeight: 4
            height: 4

            Rectangle {
                anchors.fill: parent
                radius: 2
                color: control.barColor
                opacity: 0.24
            }

            Rectangle {
                // at least a round dot as soon as the first file is under way
                width: control.shownTotal > 0 ? Math.max(parent.height, parent.width * control.shownDone / control.shownTotal) : 0
                height: parent.height
                radius: 2
                color: control.barColor
                Behavior on width { NumberAnimation { duration: 200 } }
            }
        }
    }
}
