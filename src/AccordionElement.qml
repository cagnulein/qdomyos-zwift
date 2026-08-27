import QtQuick 2.7
import QtQuick.Layouts 1.3

ColumnLayout {
    id: rootElement
    property bool isOpen: false
    property string title: ""
    property alias color: accordionHeader.color
    property alias textColor: accordionText.color
    property alias textFont: accordionText.font.family
    property alias textFontSize: accordionText.font.pixelSize
    property alias indicatRectColor: indicatRect.color
    default property alias accordionContent: contentLoader.sourceComponent
    property bool settingsVisualTestMode: Qt.application.arguments.indexOf("--settings-visual-test") >= 0

    // Signal emitted when content becomes visible
    signal contentBecameVisible()

    spacing: 0
    Layout.fillWidth: true

    function openVisualTestDescendants(item) {
        if (!settingsVisualTestMode || !item || item.children === undefined)
            return
        var children = item.children
        for (var i = 0; i < children.length; ++i) {
            var child = children[i]
            if (!child)
                continue
            if (child.isOpen !== undefined && child.title !== undefined)
                child.isOpen = true
            openVisualTestDescendants(child)
        }
    }

    Rectangle {
        id: accordionHeader
        color: "red"
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true
        height: 48

        Accessible.role: Accessible.Button         
        Accessible.name: title 
        Accessible.description: isOpen ? "Expanded" : "Collapsed"
        Accessible.onPressAction: toggle()

        Rectangle {
            id: indicatRect
            x: 16; y: 20
            width: 8; height: 8
            radius: 8
            color: "white"
        }

        Text {
            id: accordionText
            x: 34; y: 13
            color: "#FFFFFF"
            text: rootElement.title
        }

        Image {
            y: 13
            anchors.right: parent.right
            anchors.rightMargin: 20
            width: 30; height: 30
            id: indicatImg
            source: "qrc:/icons/arrow-collapse-vertical.png"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                rootElement.isOpen = !rootElement.isOpen
                if(rootElement.isOpen) {
                    indicatImg.source = "qrc:/icons/arrow-expand-vertical.png"
                } else {
                    indicatImg.source = "qrc:/icons/arrow-collapse-vertical.png"
                }
            }
        }
    }

    // Loader with enhanced visibility handling
    Loader {
        id: contentLoader
        active: rootElement.isOpen
        visible: false // Start invisible
        Layout.fillWidth: true
        asynchronous: false

        onLoaded: {
            if (item) {
                item.Layout.fillWidth = true
                visible = true
                rootElement.contentBecameVisible()
                if (rootElement.settingsVisualTestMode)
                    Qt.callLater(function() { rootElement.openVisualTestDescendants(item) })
            }
        }

        // Handle visibility changes
        onVisibleChanged: {
            if (visible && status === Loader.Ready) {
                rootElement.contentBecameVisible()
            }
        }
    }

    // Handle accordion closing
    onIsOpenChanged: {
        if (!isOpen) {
            contentLoader.visible = false
        }
    }
}
