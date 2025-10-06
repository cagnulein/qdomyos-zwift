import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    
    property string workoutSource: "QZ"
    property alias text: tagText.text
    
    // Auto-size based on text
    width: tagText.implicitWidth + 16
    height: 24
    radius: 12
    
    // Color scheme based on workout source
    color: {
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
    border.width: 1
    
    Text {
        id: tagText
        anchors.centerIn: parent
        text: workoutSource.toUpperCase()
        color: "white"
        font.pixelSize: 10
        font.bold: true
        font.family: "Arial"
    }
    
    // Subtle shadow effect
    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 1
        anchors.leftMargin: 1
        radius: parent.radius
        color: "#20000000"
        z: -1
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