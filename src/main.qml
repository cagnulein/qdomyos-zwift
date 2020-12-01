import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Dialogs 1.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
	 objectName: "stack"
    title: qsTr("Stack")

    signal trainprogram_open_clicked(url name)

    header: ToolBar {
        contentHeight: toolButton.implicitHeight

        ToolButton {
            id: toolButton
            icon.source: "icons/icons/icon.png"
            text: stackView.depth > 1 ? "\u25C0" : "\u2630"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        }

        Label {
            text: stackView.currentItem.title
            anchors.centerIn: parent
        }
    }

    Drawer {
        id: drawer
        width: window.width * 0.66
        height: window.height

        Column {
            anchors.fill: parent

            ItemDelegate {
                text: qsTr("Settings")
                width: parent.width
                onClicked: {
                    stackView.push("settings.qml")
                    drawer.close()
                }
            }
            ItemDelegate {
                id: gpx_open
                text: qsTr("Open GPX")
                width: parent.width
                onClicked: {
                    fileDialog.visible = true
                    drawer.close()
                }
            }
            ItemDelegate {
                id: trainprogram_open
                text: qsTr("Open Train Program")
                width: parent.width
                onClicked: {
                    fileDialog.visible = true
                    drawer.close()
                }
            }
            ItemDelegate {
                text: qsTr("by Roberto Viola")
                width: parent.width
/*                onClicked: {
                    stackView.push("Page2Form.ui.qml")
                    drawer.close()
                }*/
            }
            FileDialog {
                id: fileDialog
                title: "Please choose a file"
                folder: shortcuts.home
                onAccepted: {
                    console.log("You chose: " + fileDialog.fileUrl)
                    trainprogram_open_clicked(fileDialog.fileUrl)
                    fileDialog.close()
                }
                onRejected: {
                    console.log("Canceled")
                    fileDialog.close()
                }
            }
        }
    }

    StackView {
        id: stackView
        initialItem: "Home.qml"
        anchors.fill: parent
    }
}
