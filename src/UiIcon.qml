import QtQuick 2.12
import "UiIcons.js" as UiIcons

// Monochrome Material Symbols icon: a glyph of the bundled icon font, so it takes any colour
// and needs neither QtSvg nor QtQuick.Shapes. Name is a key of UiIcons.codes.
Item {
    id: icon
    property string name: ""
    property color color: "white"

    implicitWidth: 24
    implicitHeight: 24

    // Loaded once: FontLoader shares the font between instances with the same source
    FontLoader {
        id: iconFont
        source: "qrc:/fonts/MaterialSymbolsQZ.ttf"
    }

    Text {
        anchors.fill: parent
        // The font's line is exactly its em square, as the 24 px box of the symbol
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: iconFont.name
        // pixelSize is whole: a fractional box (a zoomed tile) is rounded, not cut down
        font.pixelSize: Math.max(1, Math.round(Math.min(icon.width, icon.height)))
        color: icon.color
        text: UiIcons.codes[icon.name] ? String.fromCharCode(UiIcons.codes[icon.name]) : ""
        // A private-use character means nothing to a screen reader; the control around
        // the icon carries the name
        Accessible.ignored: true
    }
}
