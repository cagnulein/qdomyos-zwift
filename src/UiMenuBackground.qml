import QtQuick 2.12
import QtGraphicalEffects 1.12

// Modern look: the container of a menu - the list of a UiComboBox and the menus that open
// under a button (Material 3 menu): the surfaceHigh tone (a menu over a popup of that tone sets
// another colour), corner 16, a soft shadow and no outline. The page colour (what Material
// gives a popup) made the list one colour with the page, a dark hole on a light card and lost
// on a dark one; an outline does not lift it off the page as the shadow does
Rectangle {
    radius: 16
    color: window.ui.surfaceHigh

    // every UiComboBox makes its popup when it loads, the classic look keeps this one hidden
    layer.enabled: visible
    layer.effect: DropShadow {
        transparentBorder: true
        horizontalOffset: 0
        verticalOffset: 2
        radius: 6
        samples: 13
        color: Qt.rgba(0, 0, 0, window.ui.dark ? 0.5 : 0.22)
    }
}
