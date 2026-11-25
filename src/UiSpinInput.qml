import QtQuick 2.7

// The number of a +/- field. Modern look: centred by its digits, not by the line - the line box
// has more room above the digits than below, and the number sat higher than the drawn signs
TextInput {
    id: input
    readonly property real digitsOffset: {
        var r = digitMetrics.tightBoundingRect("0")
        return -(digitMetrics.ascent - digitMetrics.descent + 2 * r.y + r.height)
    }
    FontMetrics { id: digitMetrics; font: input.font }
    topPadding: window.ui.modern ? Math.max(0, digitsOffset) : 0
    bottomPadding: window.ui.modern ? Math.max(0, -digitsOffset) : 0
    horizontalAlignment: Qt.AlignHCenter
    verticalAlignment: Qt.AlignVCenter
}
