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
    property var foldersToSearch: []  // Queue for iterative search
    property int currentSearchDepth: 0

    // Pool of reusable FolderListModels (pre-created for iOS compatibility)
    Component {
        id: folderModelComponent
        FolderListModel {
            nameFilters: ["*.xml", "*.zwo"]
            showDirs: true
            showDotAndDotDot: false
        }
    }

    // JavaScript functions for iterative file search (iOS compatible)
    function searchRecursively(folderUrl, filter) {
        console.log("=== SEARCH STARTED (ITERATIVE) ===")
        console.log("Search folder:", folderUrl)
        console.log("Search filter:", filter)

        searchResultsModel.clear()
        isSearching = true
        var searchPattern = filter.toLowerCase()

        // Use iterative approach with queue instead of recursion
        foldersToSearch = [folderUrl]
        searchNextFolder(searchPattern)
    }

    function searchNextFolder(pattern) {
        if (foldersToSearch.length === 0) {
            console.log("=== SEARCH COMPLETED ===")
            console.log("Total results found:", searchResultsModel.count)
            return
        }

        var folderUrl = foldersToSearch.shift()
        console.log("Processing folder:", folderUrl, "(remaining:", foldersToSearch.length, ")")

        // Create a model using Component (works better on iOS than Qt.createQmlObject)
        var tempModel = folderModelComponent.createObject(stackView, {
            "folder": folderUrl
        })

        if (!tempModel) {
            console.log("ERROR: Failed to create FolderListModel!")
            searchNextFolder(pattern)  // Continue with next folder
            return
        }

        console.log("Model created, folder:", tempModel.folder)

        // Wait for model to populate
        var attempts = 0
        var maxAttempts = 50
        while (tempModel.count === 0 && attempts < maxAttempts) {
            attempts++
            if (attempts % 10 === 0) {
                console.log("  Waiting... attempt", attempts)
            }
            var startDelay = Date.now()
            while (Date.now() - startDelay < 20) {}
        }

        console.log("Model loaded:", tempModel.count, "items after", attempts, "attempts")

        processFolderItems(tempModel, pattern, folderUrl)

        tempModel.destroy()

        // Process next folder in queue
        searchNextFolder(pattern)
    }

    function processFolderItems(model, pattern, baseFolder) {
        for (var i = 0; i < model.count; i++) {
            var isFolder = model.isFolder(i)
            var fileName = model.get(i, "fileName")
            var fileUrl = model.get(i, "fileUrl") || model.get(i, "fileURL")

            if (isFolder) {
                console.log("  Found subfolder:", fileName, "- adding to queue")
                // Add to queue for later processing
                foldersToSearch.push(fileUrl)
            } else {
                var matches = fileName.toLowerCase().indexOf(pattern) !== -1
                if (matches) {
                    var trainingBaseFolder = "file://" + rootItem.getWritableAppDir() + 'training'
                    var relativePath = fileUrl.toString().replace(trainingBaseFolder, "")
                    if (relativePath.startsWith("/")) {
                        relativePath = relativePath.substring(1)
                    }

                    console.log("  Found match:", fileName, "->", relativePath)

                    searchResultsModel.append({
                        "fileName": fileName,
                        "fileUrl": fileUrl,
                        "relativePath": relativePath,
                        "isFolder": false
                    })
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

                            console.log("=== FILTER UPDATED ===")
                            console.log("Filter text:", "'" + text + "'")
                            console.log("Text length:", text.length)

                            if (text === "") {
                                console.log("Empty filter - switching to folder browsing mode")
                                // No filter - use normal folder browsing
                                isSearching = false
                            } else {
                                console.log("Non-empty filter - triggering recursive search")
                                // Trigger recursive search
                                var baseFolder = "file://" + rootItem.getWritableAppDir() + 'training'
                                console.log("Base folder:", baseFolder)
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

                        onCountChanged: {
                            console.log("FolderListModel count changed:", count)
                        }
                    }

                    model: isSearching ? searchResultsModel : folderModel

                    onModelChanged: {
                        console.log("=== ListView MODEL CHANGED ===")
                        console.log("isSearching:", isSearching)
                        console.log("Using model:", isSearching ? "searchResultsModel" : "folderModel")
                        console.log("Model count:", model.count)
                    }

                    delegate: Component {
                        Rectangle {
                            width: ListView.view.width
                            height: 50
                            color: ListView.isCurrentItem ? Material.color(Material.Green, Material.Shade800) : Material.backgroundColor

                            property bool isItemFolder: isSearching ? model.isFolder : folderModel.isFolder(index)
                            property string itemFileName: isSearching ? model.fileName : folderModel.get(index, "fileName")
                            property string itemFileUrl: isSearching ? model.fileUrl : (folderModel.get(index, 'fileUrl') || folderModel.get(index, 'fileURL'))
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
                                        console.log('Loading preview for: ' + itemFileUrl);
                                        trainprogram_preview(itemFileUrl)
                                        pendingWorkoutUrl = itemFileUrl

                                        // Wait for preview to load then push detail view
                                        detailViewTimer.restart()
                                    }
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
