import QtQuick 2.7
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0

ScrollView {
    Popup {
        id: popup
        parent: Overlay.overlay

        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)
        width: 380
        height: 60
        modal: true
        focus: true
        palette.text: "white"
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        enter: Transition
        {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }
        exit: Transition
        {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Program has been loaded correctly. Press start to begin!")
            }
        }
    }

    signal trainprogram_open_clicked(url name)
    ColumnLayout {
        ToolBar {
            RowLayout {
                anchors.fill: parent
                ToolButton {
                    text: qsTr("‹")
                    onClicked: stack.pop()
                }
                ToolButton {
                    text: qsTr("Search")
                    onClicked: {
                        fileDialogTrainProgram.visible = true
                    }
                }
            }
        }
        FileDialog {
            id: fileDialogTrainProgram
            title: "Please choose a file"
            folder: shortcuts.home
            onAccepted: {
                console.log("You chose: " + fileDialogTrainProgram.fileUrl)
                trainprogram_open_clicked(fileDialogTrainProgram.fileUrl)
                fileDialogTrainProgram.close()
                popup.open()
            }
            onRejected: {
                console.log("Canceled")
                fileDialogTrainProgram.close()
            }
        }
        ListView {
            FolderListModel {
                id: folderModel
                nameFilters: ["*.xml"]
                rootFolder: rootItem.getWritableAppDir() + '/training'
                showDotAndDotDot: false
                showDirs: false
                showOnlyReadable: true
            }
            id: list
            anchors.fill: parent
            model: forlderModel
            delegate: Component {
                Item {
                    width: parent.width
                    height: 40
                    Column {
                        Text { text: fileName.substring(0, fileName.length-4) }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: list.currentIndex = index
                    }
                }
            }
            highlight: Rectangle {
                color: 'grey'
                Text {
                    anchors.centerIn: parent
                    text: 'Selected ' + model.get(list.currentIndex).fileName
                    color: 'white'
                }
            }
            focus: true
            onCurrentItemChanged: {
                let mi = model.get(list.currentIndex);
                if (mi) {
                    console.log(mi.fileName + ' selected');
                    let url = mi.fileUrl || mi.fileURL;
                    trainprogram_open_clicked(url);
                }
            }
        }
    }
}
