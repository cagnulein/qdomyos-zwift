import QtQuick 2.12

// Rounded rectangle with an outline. Rectangle.border with a radius comes out broken on
// Android at a fractional scale: the thin ring is drawn in pieces, with gaps along the top
// edge and flat cuts at the sides. The modern look draws the outline as a filled ring
// instead - the frame in the stroke colour and the inner rectangle over it - and fills have
// clean edges. The fill has to be opaque, or the stroke colour shows through it: a
// translucent fill is laid over "base" (the colour under the frame) with Qt.tint.
// Classic look: a plain Rectangle with its border, as before.
Rectangle {
    id: frame

    property color fill: "transparent"
    property color stroke: "transparent"
    property real strokeWidth: 0
    // What is under the frame: a translucent fill is mixed over it
    property color base: window.ui.bg

    readonly property bool ring: window.ui.modern && strokeWidth > 0 && stroke.a > 0
    readonly property color solidFill: fill.a >= 1 ? fill : Qt.tint(base, fill)

    color: ring ? stroke : fill
    border.width: window.ui.modern ? 0 : strokeWidth
    border.color: stroke

    Rectangle {
        visible: frame.ring
        anchors.fill: parent
        anchors.margins: frame.strokeWidth
        radius: Math.max(0, frame.radius - frame.strokeWidth)
        color: frame.solidFill
    }
}
