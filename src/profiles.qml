import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import Qt.labs.platform 1.1
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0 as FileDialogClass

ColumnLayout {

    anchors.top: parent.top
    anchors.fill: parent

    signal profile_open_clicked(url name)

    Connections {
        target: rootItem
        function onAndroidDocumentPicked(kind, localUrl) {
            if (kind === "profile") {
                profile_open_clicked(localUrl)
            }
        }
    }

    Settings {
        id: settings
        property string profile_name: "default"
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: Component {
            FileDialogClass.FileDialog {
                title: qsTr("Please choose a file")
                folder: shortcuts.home
                visible: true
                onAccepted: {
                    console.log("You chose: " + fileUrl)
                    profile_open_clicked(fileUrl)
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

    MessageDialog {
        id: quitDialog
        title: qsTr("Profile loaded")
        text: qsTr("Would you like to quit?")
        informativeText: qsTr("You must quit and restart for changes to take effect.")
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
        title: qsTr("Delete profile")
        text: qsTr("Would you like to delete this profile?")
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            deleteSettings(fileUrl)
            list.refreshProfiles()
            deleteDialog.close()
        }
        onNoClicked: {
            deleteDialog.close()
        }
    }

    MessageDialog {
        id: saveDialog
        title: qsTr("Profile Saved")
        text: qsTr("Profile saved correctly!")
        buttons: (MessageDialog.Ok)
        onOkClicked: {
            list.refreshProfiles()
            stackView.pop();
        }
    }

    MessageDialog {
        id: restoreSettingsDialog
        title: qsTr("New Profile")
        text: qsTr("New Profile Created with default values. Save it with a name and restart the app to apply them.")
        buttons: (MessageDialog.Ok)
        onOkClicked: {
            restoreSettingsDialog.visible = false
        }
    }

    MessageDialog {
        id: newProfileDialog
        title: qsTr("Save Current Profile?")
        text: qsTr("You're creating a new profile with the default values, would you like to save the current one before?")
        buttons: (MessageDialog.Yes | MessageDialog.No | MessageDialog.Abort)
        onYesClicked: {
                if(profileNameTextField.text.length == 0)
                    profileNameTextField.text = qsTr("OldProfile")

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
        ComboBox {
            id: profileSelector
            Layout.preferredWidth: 150
            model: profileModel
            textRole: "fileBaseName"
            currentIndex: list.currentIndex
            onActivated: list.selectProfile(index)
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
                list.refreshProfiles()
                saveDialog.visible = true;
            }
        }
    }

    StaticAccordionElement {
        z: 10
        title: qsTr("Profiles")
        indicatRectColor: Material.color(Material.Grey)
        textColor: Material.color(Material.Grey)
        color: Material.backgroundColor
        isOpen: true        
        accordionContent: ColumnLayout {
            Column {
                id: list
                property bool clicked: false
                property int currentIndex: -1
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(profileModel.count * 40, 40)
                height: Math.max(profileModel.count * 40, 40)
                width: parent.width
                ListModel {
                    id: profileModel
                }
                function refreshProfiles() {
                    profileModel.clear()
                    const profiles = rootItem.getProfileFiles()
                    for (var i = 0; i < profiles.length; ++i) {
                        profileModel.append(profiles[i])
                    }
                    list.clicked = false
                    list.currentIndex = -1
                    for (var j = 0; j < profileModel.count; ++j) {
                        if (profileModel.get(j, "fileBaseName") === settings.profile_name) {
                            list.currentIndex = j
                            return
                        }
                    }
                }
                function selectProfile(profileIndex) {
                    console.log("PROFILE_SELECT", profileIndex)
                    list.clicked = true
                    const selectedFileName = profileModel.get(profileIndex, "fileName")
                    if (selectedFileName)
                        profileNameTextField.text = selectedFileName.substring(0, selectedFileName.length - 4)
                    if (profileIndex == list.currentIndex) {
                        const fileUrl = profileModel.get(profileIndex, "fileUrl")
                        if (fileUrl) {
                            loadSettings(fileUrl)
                            quitDialog.visible = true
                        }
                    } else {
                        list.currentIndex = profileIndex
                    }
                }
                function deleteProfile(profileIndex) {
                    list.clicked = true
                    const selectedFileName = profileModel.get(profileIndex, "fileName")
                    deleteDialog.informativeText = selectedFileName.substring(0, selectedFileName.length - 4)
                    deleteDialog.fileUrl = profileModel.get(profileIndex, "fileUrl")
                    deleteDialog.visible = true
                }
                Component.onCompleted: refreshProfiles()
                Repeater {
                    model: profileModel
                    delegate: Button {
                        z: 1000
                        width: Math.max(list.width, 320)
                        height: 40
                        y: index * 40
                        flat: true
                        highlighted: index === list.currentIndex
                        text: fileName.substring(0, fileName.length - 4)
                        onClicked: list.selectProfile(index)
                        onPressAndHold: list.deleteProfile(index)
                        MouseArea {
                            anchors.fill: parent
                            z: 100
                            onClicked: list.selectProfile(index)
                            onPressAndHold: list.deleteProfile(index)
                        }
                    }
                }
                onCurrentIndexChanged: {
                    if (list.currentIndex < 0 || list.currentIndex >= profileModel.count)
                        return
                    let fileUrl = profileModel.get(list.currentIndex, 'fileUrl');
                    if (fileUrl) {
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
            if (Qt.platform.os === "android") {
                rootItem.openAndroidDocumentPicker("profile")
            } else {
                fileDialogLoader.active = true
            }
        }
        anchors {
            bottom: parent.bottom
        }
    }

    MouseArea {
        id: profileTapArea
        anchors.fill: parent
        z: 10000
        propagateComposedEvents: true
        function profileIndexAt(x, y) {
            const point = profileTapArea.mapToItem(list, x, y)
            const profileIndex = Math.floor(point.y / 40)
            console.log("PROFILE_TAP", x, y, point.x, point.y, profileIndex)
            return point.x >= 0 && point.x <= list.width &&
                   profileIndex >= 0 && profileIndex < profileModel.count ? profileIndex : -1
        }
        onPressed: {
            mouse.accepted = profileIndexAt(mouse.x, mouse.y) >= 0
        }
        onClicked: {
            const profileIndex = profileIndexAt(mouse.x, mouse.y)
            if (profileIndex >= 0)
                list.selectProfile(profileIndex)
            else
                mouse.accepted = false
        }
        onPressAndHold: {
            const profileIndex = profileIndexAt(mouse.x, mouse.y)
            if (profileIndex >= 0)
                list.deleteProfile(profileIndex)
            else
                mouse.accepted = false
        }
    }
}
