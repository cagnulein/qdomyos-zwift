import QtQuick 2.7
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0

ColumnLayout {
    signal loadSettings(url name)

    Connections {
        target: rootItem
        function onAndroidDocumentPicked(kind, localUrl) {
            if (kind === "settings") {
                loadSettings(localUrl)
            }
        }
    }

    Component.onCompleted: rootItem.folderPageOpened()

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: Component {
            FileDialog {
                title: qsTr("Please choose a file")
                folder: shortcuts.home
                visible: true
                onAccepted: {
                    console.log("You chose: " + fileUrl)
                    loadSettings(fileUrl)
                    close()
                    // Destroy and recreate the dialog for next use
                    fileDialogLoader.active = false
                }
                onRejected: {
                    console.log("Canceled")
                    close()
                    // Destroy the dialog
                    fileDialogLoader.active = false
                }
            }
        }
    }

    StaticAccordionElement {
        title: qsTr("Settings folder")
        indicatRectColor: Material.color(Material.Grey)
        textColor: Material.color(Material.Grey)
        color: Material.backgroundColor
        accordionContent: ColumnLayout {
            ListView {
                id: list
                anchors.fill: parent
                FolderListModel {
                    id: folderModel
                    nameFilters: ["*.qzs"]
                    folder: "file://" + rootItem.getWritableAppDir() + 'settings'
                    showDotAndDotDot: false
                    showDirs: true
                    sortReversed: true
                }
                // files of a previous install were copied back into the QZ folder; here, as
                // the accordion loads this list as a component, out of reach of the page
                Connections {
                    target: rootItem
                    function onFolderFilesRestored() {
                        var currentFolder = folderModel.folder
                        folderModel.folder = ""
                        folderModel.folder = currentFolder
                    }
                }
                model: folderModel
                delegate: Component {
                    Rectangle {
                        property alias textColor: fileTextBox.color
                        // not parent.width: while the folder is set again (onFolderFilesRestored)
                        // the rows go and have no parent for a moment
                        width: ListView.view.width
                        height: window.textScale !== 1 ? Math.max(40, fileTextBox.implicitHeight) : 40
                        color: Material.backgroundColor
                        z: 1
                        Text {
                            id: fileTextBox
                            color: Material.color(Material.Grey)
                            font.pixelSize: window.textScale === 1 ? Qt.application.font.pixelSize * 1.6 : (Qt.application.font.pixelSize > 0 ? window.fontPx(window.appFontPx * 1.6) : window.plainFontPx)
                            text: fileName.substring(0, fileName.length-4)
                            leftPadding: window.contentSideMargin
                        }
                        MouseArea {
                            anchors.fill: parent
                            z: 100
                            onClicked: {
                                console.log('onclicked ' + index+ " count "+list.count);
                                if (index == list.currentIndex) {
                                    let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                                    if (fileUrl) {
                                        loadSettings(fileUrl);
                                    }
                                }
                                else {
                                    if (list.currentItem)
                                        list.currentItem.textColor = Material.color(Material.Grey)
                                    list.currentIndex = index
                                }
                            }
                        }
                    }
                }
                highlight: Rectangle {
                    color: Material.color(Material.Green)
                    z:3
                    radius: 5
                    opacity: 0.4
                    focus: true
                    /*Text {
                        anchors.centerIn: parent
                        text: 'Selected ' + folderModel.get(list.currentIndex, "fileName")
                        color: "white"
                    }*/
                }
                focus: true
                onCurrentItemChanged: {
                    let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                    if (fileUrl) {
                        list.currentItem.textColor = Material.color(Material.Yellow)
                        console.log(fileUrl + ' selected');
                    }
                }
            }
        }
    }
    spacing: 10

    RowLayout {
        Layout.alignment: Qt.AlignCenter | Qt.AlignVCenter
        anchors {
            bottom: parent.bottom
        }

        Button {
            id: searchButton
            height: 50
            text: qsTr("Other folders")
            onClicked: {
                console.log("folder is " + rootItem.getWritableAppDir() + 'settings')
                if (Qt.platform.os === "android") {
                    rootItem.openAndroidDocumentPicker("settings")
                } else {
                    fileDialogLoader.active = true
                }
            }
        }

        // files of a previous install are found only on request; with a saved grant on the
        // QZ folder the page restores them by itself
        Button {
            height: 50
            visible: rootItem.qzFolderRestorable && !rootItem.qzFolderGrantSaved
            text: qsTr("Restore files")
            onClicked: rootItem.offerFolderRestore()
        }
    }
}
