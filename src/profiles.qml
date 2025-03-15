import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import Qt.labs.platform 1.1
import Qt.labs.folderlistmodel 2.15
import Qt.labs.settings 1.0
import QtQuick.Dialogs

ColumnLayout {
    anchors.top: parent.top
    anchors.fill: parent

    signal profile_open_clicked(url name)

    Component.onCompleted: {
      console.log("Profile directory path: " + rootItem.getProfileDir());
      console.log("Folder model count: " + folderModel.count);
      // Debug the model contents to see what files are actually in the model
      for (var i = 0; i < folderModel.count; i++) {
          console.log("File at index " + i + ": " + folderModel.get(i, "fileName"));
      }
    }

    Settings {
        id: settings
        property string profile_name: "default"
    }

    FileDialog {
        id: fileDialogTrainProgram
        title: "Please choose a file"
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        onAccepted: {
            console.log("You chose: " + fileDialogTrainProgram.selectedFile)
            profile_open_clicked(fileDialogTrainProgram.selectedFile)
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
        onAccepted: {
            if (quitDialog.clickedButton === MessageDialog.Yes) {
                restart()
            } else {
                quitDialog.close()
            }
        }
    }

    MessageDialog {
        id: deleteDialog
        property string fileUrl
        title: "Delete profile"
        text: "Would you like to delete this profile?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onAccepted: {
            if (deleteDialog.clickedButton === MessageDialog.Yes) {
                deleteSettings(fileUrl)
            } else {
                deleteDialog.close()
            }
        }
    }

    MessageDialog {
        id: saveDialog
        title: "Profile Saved"
        text: "Profile saved correctly!"
        buttons: (MessageDialog.Ok)
        onAccepted: {
            stackView.pop();
        }
    }

    MessageDialog {
        id: restoreSettingsDialog
        title: "New Profile"
        text: "New Profile Created with default values. Save it with a name and restart the app to apply them."
        buttons: (MessageDialog.Ok)
        onAccepted: {
            restoreSettingsDialog.visible = false
        }
    }

    MessageDialog {
        id: newProfileDialog
        title: "Save Current Profile?"
        text: "You're creating a new profile with the default values, would you like to save the current one before?"
        buttons: (MessageDialog.Yes | MessageDialog.No | MessageDialog.Abort)
        onAccepted: {
            if (newProfileDialog.clickedButton === MessageDialog.Yes) {
                if (profileNameTextField.text.length == 0)
                    profileNameTextField.text = "OldProfile"

                saveProfile(profileNameTextField.text);
                restoreSettings()

                newProfileDialog.visible = false;
                restoreSettingsDialog.visible = true
            } else if (newProfileDialog.clickedButton === MessageDialog.No) {
                restoreSettings()
                newProfileDialog.visible = false;
                restoreSettingsDialog.visible = true
            } else if (newProfileDialog.clickedButton === MessageDialog.Abort) {
                newProfileDialog.visible = false;
            }
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
        // Ensure accordion element has height
        Layout.fillWidth: true
        Layout.preferredHeight: 300
        // Add content height
        accordionContent: ColumnLayout {
            anchors.fill: parent
            spacing: 5

            // Add explicit height to make sure ListView appears
            Layout.fillHeight: true
            Layout.fillWidth: true

            ListView {
                id: list
                property bool clicked: false

                // Make sure ListView gets proper space
                Layout.fillHeight: true
                Layout.fillWidth: true
                height: parent.height
                width: parent.width
                clip: true // Add this to clip content to ListView boundaries

                // Add spacing and visual debugging
                spacing: 2

                FolderListModel {
                    id: folderModel
                    nameFilters: ["*.qzs"]
                    folder: "file://" + rootItem.getProfileDir()
                    showDotAndDotDot: false
                    showDirs: false
                    sortReversed: true
                    onStatusChanged: {
                        if(folderModel.status == FolderListModel.Ready && list.clicked == false) {
                            // Debug output when folder model is ready
                            //console.log("FolderModel is ready. Count: " + folderModel.count);
                            for(var i=0; i<folderModel.count; i++) {
                                //console.log("File " + i + ": " + folderModel.get(i, "fileName"));
                                if(folderModel.get(i, "fileBaseName") === settings.profile_name) {
                                    list.currentIndex = i;
                                    //console.log("Current profile found at index " + i);
                                    return;
                                }
                            }
                        }
                    }
                    Component.onCompleted: {
                        // on Windows it doesn't update the folder
                        folderModel.folder = "file://" + rootItem.getProfileDir();
                        console.log("Folder model initialized with path: " + folderModel.folder);
                    }
                }
                model: folderModel
                delegate: Component {
                    Rectangle {
                        property alias textColor: fileTextBox.color
                        width: list.width
                        height: 40
                        color: Material.backgroundColor
                        z: 1

                        // Ensure text is properly positioned and visible
                        Text {
                            id: fileTextBox
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            color: Material.color(Material.Grey)
                            font.pixelSize: Qt.application.font.pixelSize * 1.6
                            text: fileName.substring(0, fileName.length-4)
                            // Add this to help with debugging
                            Component.onCompleted: {
                                console.log("Created delegate for: " + fileName);
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            z: 100
                            onClicked: {
                                list.clicked = true;
                                console.log('onclicked ' + index + " count " + list.count);
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
                                console.log('onPressAndHold ' + index + " count " + list.count);
                                deleteDialog.informativeText = folderModel.get(index, 'fileName').substring(0, fileName.length-4)
                                deleteDialog.fileUrl = folderModel.get(index, 'fileUrl') || folderModel.get(index, 'fileURL')
                                deleteDialog.visible = true
                            }
                        }
                    }
                }
                highlight: Rectangle {
                    color: Material.color(Material.Green)
                    z: 3
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
                    if (list.currentIndex >= 0 && list.currentIndex < folderModel.count) {
                        let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                        if (fileUrl) {
                            // Reset color for all items first
                            for(var i=0; i<folderModel.count; i++) {
                                var item = list.itemAtIndex(i);
                                if (item) {
                                    item.textColor = Material.color(Material.Grey);
                                }
                            }

                            // Set color for current item
                            if (list.currentItem) {
                                list.currentItem.textColor = Material.color(Material.Yellow);
                                console.log(fileUrl + ' selected');
                            }
                        }
                    }
                }

                // Add this section to show visual placeholder when list is empty
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    visible: folderModel.count === 0

                    Text {
                        anchors.centerIn: parent
                        text: "No profiles found"
                        color: Material.color(Material.Grey)
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
