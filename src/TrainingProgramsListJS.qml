import QtQuick 2.7
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import QtWebView 1.1

ColumnLayout {
    signal trainprogram_open_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal trainprogram_preview(url name)
    signal trainprogram_autostart_requested()

    property url pendingWorkoutUrl: ""

    Settings {
        id: settings
        property real ftp: 200.0
    }

    property var selectedFileUrl: ""
    property bool isSearching: false
    property bool waitingForModelReload: false
    property var foldersToSearch: []  // Queue for iterative search
    property var savedFolderUrl: ""   // Save original folder for restoration
    property var mainFolderModel: null  // Reference to main folderModel for iOS workaround
    property string currentSearchPattern: ""  // Current search filter

    // JavaScript functions for search using main folderModel (iOS workaround)
    // On iOS, dynamically created FolderListModels don't work in sandboxed environment
    // So we temporarily hijack the main folderModel for searching
    function searchRecursively(folderUrl, filter) {
        if (!mainFolderModel) {
            console.log("ERROR: mainFolderModel not set yet!")
            return
        }

        searchResultsModel.clear()

        // Save current folder to restore later
        savedFolderUrl = mainFolderModel.folder

        isSearching = true
        currentSearchPattern = filter.toLowerCase()

        // Use iterative approach with queue, reusing main folderModel
        foldersToSearch = [folderUrl]
        searchNextFolderUsingMain()
    }

    function searchNextFolderUsingMain() {
        if (foldersToSearch.length === 0) {
            console.log("Search complete, restoring folder")
            // Restore original folder
            isSearching = true  // Keep search mode active while restoring
            waitingForModelReload = false  // Don't process when restoring
            mainFolderModel.folder = savedFolderUrl
            return
        }

        var folderUrl = foldersToSearch.shift()
        console.log("Searching folder:", folderUrl, "- remaining:", foldersToSearch.length)

        // Use main folderModel - change its folder temporarily
        // onStatusChanged will trigger processing when ready
        waitingForModelReload = true
        mainFolderModel.folder = folderUrl
    }

    function processFolderContents() {
        // WORKAROUND: Build fileURL manually since we can't use .get() on FolderListModel
        // We'll create a temporary ListView delegate to access the roles
        processorLoader.active = true
    }

    // Loader for temporary processor ListView
    Loader {
        id: processorLoader
        active: false
        sourceComponent: Component {
            ListView {
                id: tempProcessor
                visible: false
                width: 1
                height: 1
                model: mainFolderModel

                Component.onCompleted: {
                    // Processor ready
                }

                delegate: Item {
                    Component.onCompleted: {
                        // Access FolderListModel roles
                        var itemIsFolder = fileIsDir
                        var itemFileName = fileName
                        var itemFileUrl = fileURL

                        console.log("Processing item", index, "/", count, ":", itemFileName, "isFolder:", itemIsFolder)

                        if (itemIsFolder) {
                            foldersToSearch.push(itemFileUrl)
                        } else {
                            var matches = itemFileName.toLowerCase().indexOf(currentSearchPattern) !== -1

                            if (matches) {
                                console.log("MATCH found:", itemFileName)
                                var trainingBaseFolder = "file://" + rootItem.getWritableAppDir() + 'training'
                                var relativePath = itemFileUrl.toString().replace(trainingBaseFolder, "")
                                if (relativePath.startsWith("/")) {
                                    relativePath = relativePath.substring(1)
                                }

                                searchResultsModel.append({
                                    "fileName": itemFileName,
                                    "filePath": itemFileUrl.toString(),
                                    "relativePath": relativePath,
                                    "isFolder": false
                                })
                            }
                        }

                        // When all items processed, continue to next folder
                        if (index === count - 1) {
                            console.log("Last item processed, moving to next folder")
                            // Unload processor and continue
                            processorLoader.active = false
                            // Use Qt.callLater to avoid re-entrancy issues
                            Qt.callLater(searchNextFolderUsingMain)
                        }
                    }
                }
            }
        }
    }

    // Model for search results
    ListModel {
        id: searchResultsModel
    }


    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: Component {
            FileDialog {
                id: fileDialog
                title: "Please choose a file"
                folder: shortcuts.home
                visible: true
                onAccepted: {
                    var chosenFile = fileDialog.fileUrl || fileDialog.file || (fileDialog.fileUrls && fileDialog.fileUrls.length > 0 ? fileDialog.fileUrls[0] : "")
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
                    close()
                    fileDialogLoader.active = false
                }
            }
        }
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
                spacing: 5

                Row {
                    Layout.fillWidth: true
                    spacing: 5

                    Text {
                        text: "Filter"
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                    }

                    TextField {
                        id: filterField
                        Layout.fillWidth: true
                        placeholderText: "Search (recursive)..."

                        function updateFilter() {
                            var text = filterField.text.trim()

                            if (text === "") {
                                // No filter - use normal folder browsing
                                isSearching = false
                            } else {
                                // Trigger recursive search
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

                    Button {
                        text: "←"
                        visible: !isSearching
                        onClicked: folderModel.folder = folderModel.parentFolder
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ScrollBar.vertical: ScrollBar {}
                    id: list

                    FolderListModel {
                        id: folderModel
                        nameFilters: ["*.xml", "*.zwo"]
                        folder: "file://" + rootItem.getWritableAppDir() + 'training'
                        showDotAndDotDot: false
                        showDirs: true
                        sortField: "Name"
                        showDirsFirst: true

                        Component.onCompleted: {
                            // Set reference for search functions (iOS workaround)
                            mainFolderModel = folderModel
                        }

                        onStatusChanged: {
                            console.log("FolderListModel status changed:", status, "waiting:", waitingForModelReload, "count:", count)
                            // When searching and model is ready, process contents immediately
                            if (waitingForModelReload && status === FolderListModel.Ready) {
                                console.log("Processing folder contents, count:", count)
                                waitingForModelReload = false
                                processFolderContents()
                            }
                        }
                    }

                    model: isSearching ? searchResultsModel : folderModel


                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 50
                        color: ListView.isCurrentItem ? Material.color(Material.Green, Material.Shade800) : Material.backgroundColor


                        // When using search results, access roles via modelData or explicit model object
                        // Note: We can't use direct role names because Text id="fileName" conflicts!
                        // IMPORTANT: Use "filePath" role name (not "fileUrl") to avoid QML auto-conversion to URL object!
                        property bool isItemFolder: isSearching ? model.isFolder : folderModel.isFolder(index)
                        property string itemFileName: isSearching ? model.fileName : folderModel.get(index, "fileName")
                        property string itemFileUrl: isSearching ? model.filePath : (folderModel.get(index, 'fileUrl') || folderModel.get(index, 'fileURL'))
                        property string itemRelativePath: isSearching ? model.relativePath : ""

                        RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10

                                Text {
                                    id: fileIcon
                                    text: isItemFolder ? "📁" : "📄"
                                    font.pixelSize: 24
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Text {
                                        id: fileName
                                        Layout.fillWidth: true
                                        text: !isItemFolder ?
                                              itemFileName.substring(0, itemFileName.length-4) :
                                              itemFileName
                                        color: isItemFolder ? Material.color(Material.Orange) : "white"
                                        font.pixelSize: 16
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: itemRelativePath
                                        color: Material.color(Material.Grey)
                                        font.pixelSize: 12
                                        elide: Text.ElideMiddle
                                        visible: isSearching && itemRelativePath !== ""
                                    }
                                }

                                Text {
                                    text: "›"
                                    font.pixelSize: 24
                                    color: Material.color(Material.Grey)
                                    visible: !ListView.isCurrentItem
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    list.currentIndex = index

                                    if (isItemFolder) {
                                        // Navigate to folder (only in browse mode)
                                        if (!isSearching) {
                                            folderModel.folder = itemFileUrl
                                        }
                                    } else if (itemFileUrl) {
                                        // Load preview and show detail view
                                        trainprogram_preview(itemFileUrl)
                                        pendingWorkoutUrl = itemFileUrl

                                        // Wait for preview to load then push detail view
                                        detailViewTimer.restart()
                                    } else {
                                        console.log("ERROR: No fileUrl available!")
                                    }
                                }
                            }
                        }

                    focus: true
                }

                Button {
                    Layout.fillWidth: true
                    height: 50
                    text: "Other folders"
                    onClicked: {
                        fileDialogLoader.active = true
                    }
                }

                // Timer to push detail view after preview loads
                Timer {
                    id: detailViewTimer
                    interval: 300
                    repeat: false
                    onTriggered: {
                        stackView.push(detailView)
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
                    spacing: 10

                    Button {
                        text: "← Back"
                        onClicked: stackView.pop()
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Start Workout"
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
                    font.pixelSize: 14
                    font.bold: true
                    color: "white"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                }

                Text {
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    text: rootItem.previewWorkoutTags
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    color: Material.color(Material.Grey, Material.Shade400)
                    horizontalAlignment: Text.AlignHCenter
                }

                // WebView con grafico
                WebView {
                    id: previewWebView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/workoutpreview/preview.html"

                    Component.onCompleted: {
                        // Update workout after a short delay to ensure data is loaded
                        updateTimer.restart()
                    }

                    Timer {
                        id: updateTimer
                        interval: 400
                        repeat: false
                        onTriggered: previewWebView.updateWorkout()
                    }

                    function updateWorkout() {
                        if (!rootItem.preview_workout_points) return;

                        // Build arrays for the workout data
                        var watts = [];
                        var speed = [];
                        var inclination = [];
                        var resistance = [];
                        var cadence = [];

                        var hasWatts = false;
                        var hasSpeed = false;
                        var hasInclination = false;
                        var hasResistance = false;
                        var hasCadence = false;

                        for (var i = 0; i < rootItem.preview_workout_points; i++) {
                            if (rootItem.preview_workout_watt && rootItem.preview_workout_watt[i] !== undefined && rootItem.preview_workout_watt[i] > 0) {
                                watts.push({ x: i, y: rootItem.preview_workout_watt[i] });
                                hasWatts = true;
                            }
                            if (rootItem.preview_workout_speed && rootItem.preview_workout_speed[i] !== undefined && rootItem.preview_workout_speed[i] > 0) {
                                speed.push({ x: i, y: rootItem.preview_workout_speed[i] });
                                hasSpeed = true;
                            }
                            if (rootItem.preview_workout_inclination && rootItem.preview_workout_inclination[i] !== undefined && rootItem.preview_workout_inclination[i] > -200) {
                                inclination.push({ x: i, y: rootItem.preview_workout_inclination[i] });
                                hasInclination = true;
                            }
                            if (rootItem.preview_workout_resistance && rootItem.preview_workout_resistance[i] !== undefined && rootItem.preview_workout_resistance[i] >= 0) {
                                resistance.push({ x: i, y: rootItem.preview_workout_resistance[i] });
                                hasResistance = true;
                            }
                            if (rootItem.preview_workout_cadence && rootItem.preview_workout_cadence[i] !== undefined && rootItem.preview_workout_cadence[i] > 0) {
                                cadence.push({ x: i, y: rootItem.preview_workout_cadence[i] });
                                hasCadence = true;
                            }
                        }

                        // Determine device type based on available data
                        var deviceType = 'bike'; // default

                        // Priority 1: If has resistance, it's a bike (regardless of inclination)
                        if (hasResistance) {
                            deviceType = 'bike';
                        }
                        // Priority 2: If has speed or inclination (without resistance), it's a treadmill
                        else if (hasSpeed || hasInclination) {
                            deviceType = 'treadmill';
                        }
                        // Priority 3: If has power or cadence (bike metrics), it's a bike
                        else if (hasWatts || hasCadence) {
                            deviceType = 'bike';
                        }

                        // Call JavaScript function in the WebView
                        var data = {
                            points: rootItem.preview_workout_points,
                            watts: watts,
                            speed: speed,
                            inclination: inclination,
                            resistance: resistance,
                            cadence: cadence,
                            deviceType: deviceType,
                            miles_unit: settings.value("miles_unit", false)
                        };

                        runJavaScript("if(window.setWorkoutData) window.setWorkoutData(" + JSON.stringify(data) + ");");
                    }
                }
            }
        }
    }
}
