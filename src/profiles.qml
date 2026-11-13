import QtQuick 2.12
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import Qt.labs.platform 1.1
import Qt.labs.folderlistmodel 2.15
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0 as FileDialogClass

ColumnLayout {
    id: profilesPage
    // Modern look: side margin of the cards and the buttons
    readonly property int modernMargin: Math.max(16, window.contentSideMargin)
    // Modern look: no spacing, each item gives its own gap. The default spacing of a layout is
    // per platform (8 dp scaled by the dpi: 6 on Android and iOS, 5 on the desktop), so a margin
    // written on top of it would differ by platform; the classic look keeps it, as before
    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }

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

    Component.onCompleted: rootItem.folderPageOpened()

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

    UiMessageDialog {
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

    UiMessageDialog {
        id: deleteDialog
        property string fileUrl
        title: qsTr("Delete profile")
        text: qsTr("Would you like to delete this profile?")
        // modern look: Cancel / Delete in red, as the other questions that cannot be undone;
        // the native dialog of the classic look keeps its Yes / No
        buttons: modern ? (MessageDialog.Yes | MessageDialog.Cancel) : (MessageDialog.Yes | MessageDialog.No)
        yesText: qsTr("Delete")
        destructive: true
        onYesClicked: {
            deleteSettings(fileUrl)
        }
        onNoClicked: {
            deleteDialog.close()
        }
    }

    UiMessageDialog {
        id: saveDialog
        title: qsTr("Profile Saved")
        text: qsTr("Profile saved correctly!")
        buttons: (MessageDialog.Ok)
        onOkClicked: {
            stackView.pop();
        }
    }

    UiMessageDialog {
        id: restoreSettingsDialog
        title: qsTr("New Profile")
        text: qsTr("New Profile Created with default values. Save it with a name and restart the app to apply them.")
        buttons: (MessageDialog.Ok)
        onOkClicked: {
            restoreSettingsDialog.visible = false
        }
    }

    UiMessageDialog {
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

    function loadProfileAt(i) {
        let fileUrl = folderModel.get(i, 'fileUrl') || folderModel.get(i, 'fileURL');
        if (fileUrl) {
            loadSettings(fileUrl);
            quitDialog.visible = true
        }
    }

    function askDeleteProfileAt(i) {
        let name = folderModel.get(i, 'fileName')
        deleteDialog.informativeText = name.substring(0, name.length-4)
        deleteDialog.fileUrl = folderModel.get(i, 'fileUrl') || folderModel.get(i, 'fileURL')
        deleteDialog.visible = true
    }

    // Modern look: a card with the name and the save buttons, the profiles as a list of
    // cards (tap selects, Load or a second tap loads, long press deletes). Same model and
    // dialogs as the classic layout below, which is hidden.

    Rectangle {
        visible: window.ui.modern
        Layout.fillWidth: true
        Layout.leftMargin: profilesPage.modernMargin
        Layout.rightMargin: profilesPage.modernMargin
        Layout.topMargin: 12
        implicitHeight: modernNameColumn.implicitHeight + 32
        radius: 16
        color: window.ui.surface

        ColumnLayout {
            id: modernNameColumn
            anchors.fill: parent
            anchors.margins: 16
            spacing: 4

            Label {
                text: qsTr("Profile name")
                color: window.ui.textMuted
                font.pixelSize: window.ui.fontCaption
            }
            UiTextField {
                id: modernNameField
                Layout.fillWidth: true
                Accessible.name: qsTr("Profile name") + ": " + text
                text: settings.profile_name
                font.pixelSize: window.ui.fontTitleSmall
                onAccepted: settings.profile_name = text
                onActiveFocusChanged: if (activeFocus) cursorPosition = text.length
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Item { Layout.fillWidth: true }
                UiButton {
                    text: qsTr("New profile")
                    onClicked: {
                        profileNameTextField.text = modernNameField.text
                        newProfileDialog.visible = true;
                    }
                }
                UiButton {
                    text: qsTr("Save")
                    highlighted: true
                    onClicked: {
                        profileNameTextField.text = modernNameField.text
                        saveProfile(modernNameField.text);
                        saveDialog.visible = true;
                    }
                }
            }
        }
    }

    Label {
        visible: window.ui.modern
        Layout.fillWidth: true
        // on the page margin, as the card above and the headers of the other list pages
        Layout.leftMargin: profilesPage.modernMargin
        Layout.rightMargin: profilesPage.modernMargin
        // a new section: 24 below the name card
        Layout.topMargin: 24
        text: qsTr("Saved profiles")
        color: window.ui.textMain
        font.pixelSize: window.ui.fontBody
        font.weight: Font.DemiBold
        wrapMode: Text.WordWrap
    }
    Label {
        visible: window.ui.modern
        Layout.fillWidth: true
        Layout.leftMargin: profilesPage.modernMargin
        Layout.rightMargin: profilesPage.modernMargin
        // the hint of the header above: one group, 4 below it
        Layout.topMargin: 4
        text: qsTr("Tap a profile to select it, then Load. Long press to delete it.")
        color: window.ui.textMuted
        font.pixelSize: window.ui.fontCaption
        wrapMode: Text.WordWrap
    }

    ListView {
        id: modernList
        visible: window.ui.modern
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.leftMargin: profilesPage.modernMargin
        Layout.rightMargin: profilesPage.modernMargin
        // 12 from the hint, as on the settings file page; 6 above the pill below (12 seen with
        // its inset), the gap it had with the page spacing
        Layout.topMargin: 12
        Layout.bottomMargin: 6
        clip: true
        spacing: 8
        currentIndex: -1
        model: folderModel
        boundsBehavior: Flickable.StopAtBounds

        // UiFrame, not Rectangle.border: a thin border breaks up on Android
        delegate: UiFrame {
            id: profileCard
            readonly property bool selected: ListView.isCurrentItem
            readonly property string profileName: fileName.substring(0, fileName.length-4)
            readonly property bool active: profileName === settings.profile_name
            width: ListView.view.width
            // Material 3 list item: two lines (name, "Active") at least 72, the icon and the button 16
            // from the edges, the text 16 after the icon; a bigger text grows it, 8 above and below
            height: Math.max(72, Math.max(40, cardText.implicitHeight) + 16)
            radius: 16
            fill: selected ? window.ui.alpha(window.ui.accent, 0.14)
                           : (cardArea.pressed ? window.ui.surfaceHigh : window.ui.surface)
            strokeWidth: selected ? 1 : 0
            stroke: window.ui.alpha(window.ui.accent, 0.6)

            // a tap selects a profile, a second tap loads it
            function activate() {
                // as in the classic list: the user picked, a rescan must not reset it
                list.clicked = true
                if (index === modernList.currentIndex)
                    loadProfileAt(index)
                else
                    modernList.currentIndex = index
            }

            MouseArea {
                id: cardArea
                anchors.fill: parent
                onClicked: profileCard.activate()
                onPressAndHold: askDeleteProfileAt(index)
                Accessible.role: Accessible.Button
                // the labels of the card are ignored (below), so "Active" is read with the name
                Accessible.name: profileCard.active ? profileCard.profileName + ", " + qsTr("Active")
                                                    : profileCard.profileName
                Accessible.onPressAction: profileCard.activate()
            }

            Rectangle {
                id: avatar
                anchors.left: parent.left
                anchors.leftMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                width: 40
                height: 40
                radius: 20
                color: profileCard.active ? window.ui.accent : window.ui.surfaceHighest
                UiIcon {
                    anchors.centerIn: parent
                    width: 22
                    height: 22
                    name: "person"
                    color: profileCard.active ? window.ui.accentInk : window.ui.textMuted
                }
            }

            Column {
                id: cardText
                anchors.left: avatar.right
                anchors.leftMargin: 16
                anchors.right: loadButton.visible ? loadButton.left : parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 2
                // the card is read through cardArea: a touch of the screen reader goes to the deepest
                // accessible item under the finger, and these labels, drawn above cardArea, would take
                // it from the card (the focus went to the whole window)
                Label {
                    Accessible.ignored: true
                    width: parent.width
                    text: profileCard.profileName
                    color: profileCard.selected ? window.ui.accent : window.ui.textMain
                    font.pixelSize: window.ui.fontBody
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }
                Label {
                    Accessible.ignored: true
                    visible: profileCard.active
                    text: qsTr("Active")
                    color: window.ui.textMuted
                    font.pixelSize: window.ui.fontCaption
                }
            }

            UiButton {
                id: loadButton
                visible: profileCard.selected
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Load")
                highlighted: true
                onClicked: loadProfileAt(index)
            }
        }
    }

    // Other folders and, until QZ has a grant on the QZ folder, Restore files in one row
    // (as on the training programs page); a long label wraps instead of eliding
    RowLayout {
        visible: window.ui.modern
        Layout.fillWidth: true
        Layout.leftMargin: profilesPage.modernMargin
        Layout.rightMargin: profilesPage.modernMargin
        // 16 to the pill: UiButton has a 6 inset below it
        Layout.bottomMargin: 10
        // a layout in a layout fills the height by default, and the fillHeight of the pills lifts its
        // maximum: the row would take the height of the page; the pills fill the height of the taller one
        Layout.fillHeight: false
        spacing: 8

        UiButton {
            Layout.fillWidth: true
            // one height for both pills when only one label wraps
            Layout.fillHeight: true
            wrapsText: true
            text: qsTr("Other folders")
            onClicked: {
                if (Qt.platform.os === "android") {
                    rootItem.openAndroidDocumentPicker("profile")
                } else {
                    fileDialogLoader.active = true
                }
            }
        }

        UiButton {
            Layout.fillWidth: true
            // one height for both pills when only one label wraps
            Layout.fillHeight: true
            wrapsText: true
            visible: rootItem.qzFolderRestorable && !rootItem.qzFolderGrantSaved
            text: qsTr("Restore files")
            onClicked: rootItem.offerFolderRestore()
        }
    }

    RowLayout {
        visible: !window.ui.modern
        spacing: 10
        Layout.leftMargin: window.contentSideMargin
        Layout.rightMargin: window.contentSideMargin
        Label {
            id: labelProfileName
            text: qsTr("Profile name")
            Layout.fillWidth: true
            wrapMode: window.labelWrap
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
            text: qsTr("Save")
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            onClicked: {
                console.log("folder is " + rootItem.getWritableAppDir() + 'profiles')
                saveProfile(profileNameTextField.text);
                saveDialog.visible = true;
            }
        }
    }

    StaticAccordionElement {
        visible: !window.ui.modern
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
                                    modernList.currentIndex = i;
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
                // files of a previous install were copied back into the QZ folder; reload the
                // model next to it
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
                            font.pixelSize: window.textScale === 1 ? Qt.application.font.pixelSize * 1.6 : (Qt.application.font.pixelSize > 0 ? window.fontPx(window.plainBasePx * 1.6) : window.plainFontPx)
                            text: fileName.substring(0, fileName.length-4)
                            leftPadding: window.contentSideMargin
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
                        // rows not made yet, or no current row for a moment while a restore renames
                        // files under the open page
                        for(var i=0; i<folderModel.count; i++) {
                            if (list.itemAtIndex(i))
                                list.itemAtIndex(i).textColor = Material.color(Material.Grey)
                        }
                        if (list.currentItem)
                            list.currentItem.textColor = Material.color(Material.Yellow)
                        console.log(fileUrl + ' selected');
                    }
                }
            }
        }
    }

    RowLayout {
        // the classic look; the modern one has its own Other folders under its list
        visible: !window.ui.modern
        Layout.alignment: Qt.AlignCenter | Qt.AlignVCenter
        anchors {
            bottom: parent.bottom
        }

        Button {
            id: searchButton
            height: 50
            text: qsTr("Other folders")
            onClicked: {
                console.log("folder is " + rootItem.getWritableAppDir() + 'training')
                if (Qt.platform.os === "android") {
                    rootItem.openAndroidDocumentPicker("profile")
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
