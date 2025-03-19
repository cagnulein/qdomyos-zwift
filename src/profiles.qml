import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import Qt.labs.platform 1.1
import Qt.labs.folderlistmodel 2.15
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0 as FileDialogClass

ColumnLayout {

    anchors.top: parent.top
    anchors.fill: parent

    signal profile_open_clicked(url name)

    Settings {
        id: settings
        property string profile_name: "default"
    }

    FileDialogClass.FileDialog {
        id: fileDialogTrainProgram
        title: "Please choose a file"
        folder: shortcuts.home
        onAccepted: {
            console.log("You chose: " + fileDialogTrainProgram.fileUrl)
            profile_open_clicked(fileDialogTrainProgram.fileUrl)
            fileDialogTrainProgram.close()
        }
        onRejected: {
            console.log("Canceled")
            fileDialogTrainProgram.close()
        }
    }

    MessageDialog {
        id: quitDialog
        title: "Profile loaded"
        text: "Would you like to quit?"
        informativeText: "You must quit and restart for changes to take effect."
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            restart()
        }
        onNoClicked: {
            quitDialog.close()
        }
    }

    MessageDialog {
        id: deleteDialog
        property string fileUrl
        title: "Delete profile"
        text: "Would you like to delete this profile?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            deleteSettings(fileUrl)
        }
        onNoClicked: {
            deleteDialog.close()
        }
    }

    MessageDialog {
        id: saveDialog
        title: "Profile Saved"
        text: "Profile saved correctly!"
        buttons: (MessageDialog.Ok)
        onOkClicked: {
            stackView.pop();
        }
    }

    MessageDialog {
        id: restoreSettingsDialog
        title: "New Profile"
        text: "New Profile Created with default values. Save it with a name and restart the app to apply them."
        buttons: (MessageDialog.Ok)
        onOkClicked: {
            restoreSettingsDialog.visible = false
        }
    }

    MessageDialog {
        id: newProfileDialog
        title: "Save Current Profile?"
        text: "You're creating a new profile with the default values, would you like to save the current one before?"
        buttons: (MessageDialog.Yes | MessageDialog.No | MessageDialog.Abort)
        onYesClicked: {
            if(profileNameTextField.text.length == 0)
                profileNameTextField.text = "OldProfile"

            saveProfile(profileNameTextField.text);
            restoreSettings()

            newProfileDialog.visible = false;
            restoreSettingsDialog.visible = true
        }
        onNoClicked: {
            restoreSettings()
            newProfileDialog.visible = false;
            restoreSettingsDialog.visible = true
        }
        onAbortClicked: {
            newProfileDialog.visible = false;
        }
    }

    RowLayout {
        spacing: 10
        Label {
            id: labelProfileName
            text: qsTr("Profile name")
            Layout.fillWidth: true
        }
        TextField {
            id: profileNameTextField
            text: settings.profile_name
            horizontalAlignment: Text.AlignRight
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onAccepted: settings.profile_name = text
            onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
        }
        Button {
            id: addProfileButton
            text: "+"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: {
                console.log("folder is " + rootItem.getWritableAppDir() + 'profiles')
                newProfileDialog.visible = true;
            }
        }
        Button {
            id: saveProfileNameButton
            text: "Save"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: {
                console.log("folder is " + rootItem.getWritableAppDir() + 'profiles')
                saveProfile(profileNameTextField.text);
                saveDialog.visible = true;
            }
        }
    }

    StaticAccordionElement {
        title: qsTr("Profiles")
        indicatRectColor: Material.color(Material.Grey)
        textColor: Material.color(Material.Grey)
        color: Material.backgroundColor
        isOpen: true        
        accordionContent: ColumnLayout {
            ListView {
                id: list
                property bool clicked: false
                anchors.fill: parent
                FolderListModel {
                    id: folderModel
                    nameFilters: ["*.qzs"]
                    folder: "file://" + rootItem.getProfileDir()
                    showDotAndDotDot: false
                    showDirs: false
                    sortReversed: true
                    onStatusChanged: {
                        if(folderModel.status ==
                                FolderListModel.Ready && list.clicked == false) {
                            for(var i=0; i<folderModel.count; i++) {
                                if(folderModel.get(i,
                                                   "fileBaseName") === settings.profile_name) {
                                    list.currentIndex = i;
                                    return;
                                }
                            }
                        }
                    }
                    Component.onCompleted: {
                        // on Windows it doesn't update the folder
                        folderModel.folder = "file://" + rootItem.getProfileDir();
                    }
                }
                model: folderModel
                delegate: Component {
                    Rectangle {
                        property alias textColor: fileTextBox.color
                        width: parent.width
                        height: 40
                        color: Material.backgroundColor
                        z: 1
                        Text {
                            id: fileTextBox
                            color: Material.color(Material.Grey)
                            font.pixelSize: Qt.application.font.pixelSize * 1.6
                            text: fileName.substring(0, fileName.length-4)
                        }
                        MouseArea {
                            anchors.fill: parent
                            z: 100
                            onClicked: {
                                list.clicked = true;
                                console.log('onclicked ' + index+ " count "+list.count);
                                if (index == list.currentIndex) {
                                    let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                                    if (fileUrl) {
                                        loadSettings(fileUrl);
                                        quitDialog.visible = true
                                    }
                                }
                                else {
                                    if (list.currentItem)
                                        list.currentItem.textColor = Material.color(Material.Grey)
                                    list.currentIndex = index
                                }
                            }
                            onPressAndHold: {
                                list.clicked = true;
                                console.log('onPressAndHold ' + index+ " count "+list.count);
                                deleteDialog.informativeText = folderModel.get(index, 'fileName').substring(0, fileName.length-4)
                                deleteDialog.fileUrl = folderModel.get(index, 'fileUrl') || folderModel.get(index, 'fileURL')
                                deleteDialog.visible = true
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
                        for(var i=0; i<folderModel.count; i++) {
                            list.itemAtIndex(i).textColor = Material.color(Material.Grey)
                        }
                        list.currentItem.textColor = Material.color(Material.Yellow)
                        console.log(fileUrl + ' selected');
                    }
                }
            }
        }
    }

    Button {
        id: searchButton
        height: 50
        width: parent.width
        text: "Other folders"
        Layout.alignment: Qt.AlignCenter | Qt.AlignVCenter
        onClicked: {
            console.log("folder is " + rootItem.getWritableAppDir() + 'training')
            fileDialogTrainProgram.visible = true
        }
        anchors {
            bottom: parent.bottom
        }
    }
}
