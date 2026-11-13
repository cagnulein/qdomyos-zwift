import QtQuick 2.7
import QtQml 2.15
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import QtWebView 1.1

ColumnLayout {
    id: programsPage
    signal trainprogram_open_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal trainprogram_preview(url name)
    signal trainprogram_autostart_requested()

    property url pendingWorkoutUrl: ""
    property url initialWorkoutUrl: ""

    Settings {
        id: settings
        property real ftp: 200.0
    }

    property var selectedFileUrl: ""
    property bool isSearching: false

    // Modern look: a filled search field, the workouts as cards and pill buttons; the classic
    // look keeps its list and buttons
    readonly property int modernMargin: Math.max(16, window.contentSideMargin)

    Connections {
        target: rootItem
        function onAndroidDocumentPicked(kind, localUrl) {
            if (kind === "training") {
                trainprogram_open_clicked(localUrl)
            }
        }
    }

    // Called by main.qml on the Android back button: leave the workout preview first
    function handleBack() {
        if (stackView.depth > 1) {
            stackView.pop()
            return true
        }
        return false
    }

    function openWorkoutPreview(fileUrl) {
        if (!fileUrl || fileUrl.toString() === "") {
            return
        }
        pendingWorkoutUrl = fileUrl
        trainprogram_preview(fileUrl)
        stackView.push(detailView)
    }

    Component.onCompleted: {
        Qt.callLater(function() {
            openWorkoutPreview(initialWorkoutUrl)
        })
        rootItem.folderPageOpened()
    }

    // Model for search results
    ListModel {
        id: searchResultsModel
    }

    // Function to perform C++-based recursive search
    function searchRecursively(folderUrl, filter) {
        searchResultsModel.clear()

        if (!filter || filter.trim() === "") {
            isSearching = false
            return
        }

        isSearching = true

        // Call C++ FileSearcher for fast recursive search
        var results = fileSearcher.searchRecursively(folderUrl, filter, ["*.xml", "*.zwo", "*.mrc", "*.erg", "*.json", "*.xsr"])

        // Populate search results model
        for (var i = 0; i < results.length; i++) {
            searchResultsModel.append(results[i])
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: Component {
            FileDialog {
                id: fileDialog
                title: qsTr("Please choose a file")
                folder: shortcuts.home
                nameFilters: [qsTr("Training programs (*.xml *.zwo *.mrc *.erg *.json *.xsr)"), qsTr("All files (*)")]
                visible: true
                onAccepted: {
                    var chosenFile = fileDialog.fileUrl || fileDialog.file || (fileDialog.fileUrls && fileDialog.fileUrls.length > 0 ? fileDialog.fileUrls[0] : "")
                    console.log("You chose: " + chosenFile)
                    selectedFileUrl = chosenFile
                    if(OS_VERSION === "Android") {
                        trainprogram_open_other_folder(chosenFile)
                    } else {
                        trainprogram_open_clicked(chosenFile)
                    }
                    close()
                    fileDialogLoader.active = false
                }
                onRejected: {
                    console.log("Canceled")
                    close()
                    fileDialogLoader.active = false
                }
            }
        }
    }

    UiMessageDialog {
        id: deleteDialog
        property url fileUrl: ""
        text: qsTr("Delete workout?")
        informativeText: qsTr("This cannot be undone.")
        // modern look: Cancel / Delete in red, as the other questions that cannot be undone;
        // the native dialog of the classic look keeps its Yes / No
        buttons: modern ? (MessageDialog.Yes | MessageDialog.Cancel) : (MessageDialog.Yes | MessageDialog.No)
        yesText: qsTr("Delete")
        destructive: true
        onYesClicked: {
            if (rootItem.deleteTrainingProgramFile(fileUrl)) {
                pendingWorkoutUrl = ""
                isSearching = false
                stackView.clear()
                stackView.push(masterView)
            }
            visible = false
        }
        onNoClicked: visible = false
    }

    StackView {
        id: stackView
        Layout.fillWidth: true
        Layout.fillHeight: true
        initialItem: masterView

        // MASTER VIEW - Lista Workout
        Component {
            id: masterView

            ColumnLayout {
                spacing: window.ui.modern ? 8 : 5

                // files of a previous install were copied back into the QZ folder
                Connections {
                    target: rootItem
                    function onFolderFilesRestored() {
                        var currentFolder = folderModel.folder
                        folderModel.folder = ""
                        folderModel.folder = currentFolder
                    }
                }

                Row {
                    id: filterRow
                    Layout.fillWidth: true
                    Layout.topMargin: window.ui.modern ? 8 : 0
                    spacing: window.ui.modern ? 8 : 5
                    leftPadding: window.ui.modern ? programsPage.modernMargin : 0
                    rightPadding: window.ui.modern ? programsPage.modernMargin : 0

                    Text {
                        // a plain Text does not inherit the window font: the text size while it is not 100 %
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        visible: !window.ui.modern
                        text: qsTr("Filter")
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                    }

                    UiTextField {
                        id: filterField
                        Layout.fillWidth: true
                        // Row has no fill: the modern field takes what the up button leaves. In a Row
                        // fillWidth does nothing: the classic field kept its 120 of Material and
                        // showed less of the hint as the text grew. It grows with the text, but
                        // leaves room for the back button on a narrow screen
                        width: window.ui.modern ? filterRow.width - filterRow.leftPadding - filterRow.rightPadding
                                         - (upButton.visible ? upButton.width + filterRow.spacing : 0)
                               : window.textScale !== 1
                               ? Math.max(implicitWidth, Math.min(window.fontPx(implicitWidth),
                                     parent.width - x - (upButton.visible ? upButton.width + parent.spacing : 0)))
                               : implicitWidth
                        leftPadding: window.ui.modern ? 44 : padding
                        placeholderText: qsTr("Search (recursive)...")

                        UiIcon {
                            visible: window.ui.modern
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            width: 22
                            height: 22
                            name: "search"
                            color: window.ui.textMuted
                        }

                        function updateFilter() {
                            var text = filterField.text.trim()

                            if (text === "") {
                                // No filter - use normal folder browsing
                                isSearching = false
                            } else {
                                // Trigger recursive C++ search
                                var baseFolder = "file://" + rootItem.getWritableAppDir() + 'training'
                                searchRecursively(baseFolder, text)
                            }
                        }

                        onTextChanged: {
                            searchTimer.restart()
                        }

                        Timer {
                            id: searchTimer
                            interval: 300
                            repeat: false
                            onTriggered: filterField.updateFilter()
                        }
                    }

                    UiButton {
                        id: upButton
                        text: window.ui.modern ? "" : "←"
                        width: window.ui.modern ? 48 : implicitWidth
                        leftPadding: window.ui.modern ? 0 : horizontalPadding
                        rightPadding: window.ui.modern ? 0 : horizontalPadding
                        visible: !isSearching
                        onClicked: folderModel.folder = folderModel.parentFolder
                        // Modern look: an icon without text, the screen reader needs a name
                        Accessible.name: window.ui.modern ? qsTr("Parent folder") : text
                        UiIcon {
                            visible: window.ui.modern
                            anchors.centerIn: parent
                            width: 22
                            height: 22
                            name: "arrow_back"
                            color: window.ui.textMain
                        }
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: window.ui.modern ? programsPage.modernMargin : 0
                    Layout.rightMargin: window.ui.modern ? programsPage.modernMargin : 0
                    spacing: window.ui.modern ? 8 : 0
                    clip: window.ui.modern
                    ScrollBar.vertical: ScrollBar {}
                    id: list

                    FolderListModel {
                        id: folderModel
                        nameFilters: ["*.xml", "*.zwo", "*.mrc", "*.erg", "*.json", "*.xsr"]
                        folder: "file://" + rootItem.getWritableAppDir() + 'training'
                        showDotAndDotDot: false
                        showDirs: true
                        sortField: "Name"
                        showDirsFirst: true
                    }

                    model: isSearching ? searchResultsModel : folderModel

                    delegate: ItemDelegate {
                        id: workoutDelegate
                        width: ListView.view.width
                        // a bigger text: the row grows with the name and the path of a search result
                        height: window.ui.modern ? Math.max(itemRelativePath !== "" ? 72 : 56, implicitContentHeight + topPadding + bottomPadding) : (window.textScale !== 1 ? Math.max(50, implicitContentHeight + topPadding + bottomPadding) : 50)

                        // Determine item properties based on which model is active
                        property bool isItemFolder: isSearching ? model.isFolder : folderModel.isFolder(index)
                        // || "": while the folder is set again (onFolderFilesRestored) the model
                        // is empty for a moment and get() gives undefined to the rows still shown
                        property string itemFileName: isSearching ? model.fileName : (folderModel.get(index, "fileName") || "")
                        property string itemFileUrl: isSearching ? model.filePath : (folderModel.get(index, 'fileUrl') || folderModel.get(index, 'fileURL') || "")
                        property string itemRelativePath: isSearching ? model.relativePath : ""

                        // Modern look: the card is read under its name, with the folder of a search
                        // result (contentItem replaces the label, so the delegate text is empty).
                        // Both looks: a connected press action makes the row clickable for the screen
                        // reader, and Qt 5.15 then only emits it, so it opens the row as a tap does
                        Accessible.name: window.ui.modern
                                         ? (itemRelativePath !== "" ? fileName.text + ", " + itemRelativePath : fileName.text)
                                         : text
                        Accessible.onPressAction: workoutDelegate.clicked()

                        background: Rectangle {
                            radius: window.ui.modern ? 16 : 0
                            color: window.ui.modern ? (workoutDelegate.pressed ? window.ui.surfaceHigh : window.ui.surface)
                                         : (ListView.isCurrentItem ? Material.color(Material.Green, Material.Shade800) : Material.backgroundColor)
                        }

                        // Modern look (Material 3 list item: one line at least 56, a search result with its
                        // folder two lines at least 72, a bigger text grows it): the icon 16 from the card edge and 16 from the text, as on the
                        // GPX, settings file and profile cards; the classic row as before
                        leftPadding: window.ui.modern ? 16 : horizontalPadding

                        contentItem: RowLayout {
                            spacing: window.ui.modern ? 16 : 10

                            // hidden, not zero-wide, in the modern look: a visible empty item still
                            // takes the spacing of the row
                            Item {
                                visible: !window.ui.modern
                                width: 10
                                height: 1
                            }

                            Text {
                                id: fileIcon
                                visible: !window.ui.modern
                                text: isItemFolder ? "📁" : "📄"
                                font.pixelSize: window.fontPx(24)
                            }

                            Rectangle {
                                visible: window.ui.modern
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                radius: 20
                                color: window.ui.surfaceHighest
                                UiIcon {
                                    anchors.centerIn: parent
                                    width: 22
                                    height: 22
                                    name: isItemFolder ? "folder" : "list_alt"
                                    color: window.ui.textMuted
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    id: fileName
                                    // modern look: the card is read through the delegate; a touch of
                                    // the screen reader goes to the deepest accessible item under the
                                    // finger, and the texts would take it from the card. The classic
                                    // row as before
                                    Accessible.ignored: window.ui.modern
                                    Layout.fillWidth: true
                                    text: !isItemFolder ?
                                          itemFileName.substring(0, itemFileName.lastIndexOf(".")) :
                                          itemFileName
                                    color: window.ui.modern ? window.ui.textMain
                                         : isItemFolder ? Material.color(Material.Orange)
                                         : "white"
                                    font.pixelSize: window.ui.fontBody
                                    font.weight: window.ui.modern ? Font.DemiBold : Font.Normal
                                    elide: Text.ElideRight
                                }

                                Text {
                                    Accessible.ignored: window.ui.modern
                                    Layout.fillWidth: true
                                    text: itemRelativePath
                                    color: window.ui.modern ? window.ui.textMuted : Material.color(Material.Grey)
                                    font.pixelSize: window.ui.modern ? window.ui.fontCaption : window.fontPx(12)
                                    elide: Text.ElideMiddle
                                    visible: isSearching && itemRelativePath !== ""
                                }
                            }

                            Text {
                                text: "›"
                                font.pixelSize: window.fontPx(24)
                                color: Material.color(Material.Grey)
                                visible: !window.ui.modern && !ListView.isCurrentItem
                            }

                            UiIcon {
                                visible: window.ui.modern
                                Layout.preferredWidth: 22
                                Layout.preferredHeight: 22
                                name: "chevron_right"
                                color: window.ui.textMuted
                            }
                        }

                        onClicked: {
                            list.currentIndex = index

                            if (isItemFolder) {
                                // Navigate to folder (only in browse mode)
                                if (!isSearching) {
                                    folderModel.folder = itemFileUrl
                                }
                            } else if (itemFileUrl) {
                                // Load preview and show detail view
                                openWorkoutPreview(itemFileUrl)
                            }
                        }
                    }

                    focus: true

                    Label {
                        parent: list
                        anchors.centerIn: parent
                        // only once the folder is read: the model fills in asynchronously
                        visible: window.ui.modern && list.count === 0
                                 && (isSearching || folderModel.status === FolderListModel.Ready)
                        text: isSearching ? qsTr("No workouts found") : qsTr("No workouts here")
                        color: window.ui.textMuted
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: window.ui.modern ? programsPage.modernMargin : 0
                    Layout.rightMargin: window.ui.modern ? programsPage.modernMargin : 0
                    // 16 to the pill: UiButton has a 6 inset below it
                    Layout.bottomMargin: window.ui.modern ? 10 : 0
                    // Classic look: the platform default spacing, as before (8 dp on Android and
                    // iOS, 5 px elsewhere) - a written value would replace it everywhere
                    Binding on spacing { when: window.ui.modern; value: 8; restoreMode: Binding.RestoreBindingOrValue }
                    // a layout in a layout fills the height by default, and the fillHeight of the pills
                    // lifts its maximum: the row would take the height of the page
                    Layout.fillHeight: false

                    UiButton {
                        Layout.fillWidth: true
                        height: 50
                        // modern look: one height for both pills, a long label wraps instead of eliding
                        Layout.fillHeight: window.ui.modern
                        wrapsText: true
                        text: qsTr("Other folders")
                        onClicked: {
                            if (Qt.platform.os === "android") {
                                rootItem.openAndroidDocumentPicker("training")
                            } else {
                                fileDialogLoader.active = true
                            }
                        }
                    }

                    // files of a previous install that no bundled file points to are found
                    // only on request; with a saved grant on the QZ folder the page restores
                    // them by itself
                    UiButton {
                        Layout.fillWidth: true
                        height: 50
                        // modern look: one height for both pills, a long label wraps instead of eliding
                        Layout.fillHeight: window.ui.modern
                        wrapsText: true
                        visible: rootItem.qzFolderRestorable && !rootItem.qzFolderGrantSaved
                        text: qsTr("Restore files")
                        onClicked: rootItem.offerFolderRestore()
                    }
                }
            }
        }

        // DETAIL VIEW - Anteprima Workout
        Component {
            id: detailView

            ColumnLayout {
                spacing: 10

                // Header con pulsanti
                RowLayout {
                    Layout.fillWidth: true
                    Layout.margins: 5
                    Layout.leftMargin: window.ui.modern ? programsPage.modernMargin - 8 : 5
                    Layout.rightMargin: window.ui.modern ? programsPage.modernMargin : 5
                    spacing: window.ui.modern ? 8 : 10

                    UiButton {
                        text: qsTr("← Back")
                        // Modern look: the toolbar arrow goes back to the list (main.qml)
                        visible: !window.ui.modern
                        onClicked: stackView.pop()
                    }

                    Item { Layout.fillWidth: true }

                    UiButton {
                        text: qsTr("Delete")
                        visible: pendingWorkoutUrl.toString() !== ""
                        danger: true
                        Material.background: Material.Red
                        onClicked: {
                            deleteDialog.fileUrl = pendingWorkoutUrl
                            deleteDialog.visible = true
                        }
                    }

                    UiButton {
                        text: qsTr("Start Workout")
                        highlighted: true
                        Material.background: Material.Green
                        onClicked: {
                            trainprogram_open_clicked(pendingWorkoutUrl)
                            trainprogram_autostart_requested()
                            stackView.pop()
                        }
                    }

                }

                // Descrizione workout
                Text {
                    Layout.fillWidth: true
                    Layout.margins: 10
                    text: rootItem.previewWorkoutDescription
                    // Modern look: an empty line kept its height and margins, a gap above the chart
                    visible: !window.ui.modern || text !== ""
                    font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(14)
                    font.weight: window.ui.modern ? Font.DemiBold : Font.Bold
                    color: window.ui.ink("white")
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    text: rootItem.previewWorkoutTags
                    visible: !window.ui.modern || text !== ""
                    font.pixelSize: window.ui.modern ? window.ui.fontCaption : window.fontPx(12)
                    wrapMode: Text.WordWrap
                    color: window.ui.modern ? window.ui.textMuted : Material.color(Material.Grey, Material.Shade400)
                    horizontalAlignment: Text.AlignHCenter
                }

                // WebView con grafico
                // Preview data is now loaded via WebSocket, no runJavaScript needed
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    // Modern look: the native view is white until the page is loaded, so it is
                    // kept hidden till then, as in WorkoutEditor.qml. It is also drawn above every
                    // QML item, so it hides while the card-styled delete question is open
                    BusyIndicator {
                        anchors.centerIn: parent
                        visible: window.ui.modern && !previewWebView.pageLoaded
                        running: visible
                    }

                    WebView {
                        id: previewWebView
                        anchors.fill: parent
                        visible: !window.ui.modern || (pageLoaded && !deleteDialog.visible)
                        url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/workoutpreview/preview.html"
                        property bool pageLoaded: false
                        // a failed load is shown too: the page tells why, not an endless spinner
                        onLoadingChanged: {
                            if (loadRequest.status === WebView.LoadSucceededStatus
                                    || loadRequest.status === WebView.LoadFailedStatus)
                                pageLoaded = true
                        }

                        // a new app language: the page takes its texts when loaded and asks the server
                        // for the program itself, so a reload loses nothing
                        Connections {
                            target: rootItem
                            function onAppLanguageChanged() {
                                // hidden again until loaded, as at the first load
                                previewWebView.pageLoaded = false
                                previewWebView.reload()
                            }
                        }
                    }
                }
            }
        }
    }
}
