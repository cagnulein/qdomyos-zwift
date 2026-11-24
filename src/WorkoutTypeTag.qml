import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    
    property string workoutSource: "QZ"
    property alias text: tagText.text
    // new look: a tonal pill in the source colour (notes/modern-ui-style-guide.md)
    readonly property bool modern: window.ui.modern
    readonly property string sourceKey: workoutSource.toUpperCase()
    // the text colour of the tag: the action buttons of a card of the new look take its pill and ink
    readonly property color inkColor: tagText.color
    
    // Auto-size based on text
    width: tagText.implicitWidth + 16
    height: modern || window.textScale !== 1 ? Math.max(24, tagText.implicitHeight + 8) : 24
    radius: height / 2
    
    // Color scheme based on workout source
    color: {
        if (modern) {
            // the source colour at 16 % (#29 = 0.16 * 255)
            switch (sourceKey) {
                case "PELOTON": return "#29ff6b35"
                case "ZWIFT": return "#29ff6900"
                case "ERG": return "#298bc34a"
                case "QZ": return "#292196f3"
                case "MANUAL": return "#29757575"
                default: return "#299e9e9e"
            }
        }
        switch(workoutSource.toUpperCase()) {
            case "PELOTON": return "#ff6b35"
            case "ZWIFT": return "#ff6900" 
            case "ERG": return "#8bc34a"
            case "QZ": return "#2196f3"
            case "MANUAL": return "#757575"
            default: return "#9e9e9e"
        }
    }
    
    // Subtle border for better definition
    border.color: Qt.darker(color, 1.2)
    border.width: modern ? 0 : 1
    
    Text {
        id: tagText
        anchors.centerIn: parent
        text: workoutSource.toUpperCase()
        // new look: the source colour, darkened in the light themes, as is or lighter in the dark ones:
        // contrast >= 4.6:1 on the pill over the card (surface) of every theme; the worst case is PELOTON
        // on #F3F4F6, the darkest card of the light themes
        color: {
            if (!root.modern)
                return "white"
            var d = window.ui.dark
            switch (root.sourceKey) {
                case "PELOTON": return d ? "#FF6B35" : "#A74623"
                case "ZWIFT": return d ? "#FF6900" : "#A64500"
                case "ERG": return d ? "#8BC34A" : "#50712B"
                case "QZ": return d ? "#339EF4" : "#1766A6"
                default: return d ? "#9C9C9C" : "#616161"
            }
        }
        font.pixelSize: root.modern ? window.ui.fontCaption : window.fontPx(10)
        font.weight: root.modern ? Font.DemiBold : Font.Bold
        font.family: root.modern ? Qt.application.font.family : "Arial"
    }
    
    // Subtle shadow effect
    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 1
        anchors.leftMargin: 1
        radius: parent.radius
        color: "#20000000"
        z: -1
        visible: !root.modern
    }
    
    // Hover effect for interactivity feedback
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        
        onEntered: {
            parent.scale = 1.05
        }
        
        onExited: {
            parent.scale = 1.0
        }
        
        Behavior on scale {
            NumberAnimation {
                duration: 150
                easing.type: Easing.OutQuad
            }
        }
    }
}