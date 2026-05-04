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
        var results = fileSearcher.searchRecursively(folderUrl, filter, ["*.xml", "*.zwo"])

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
                    }

                    model: isSearching ? searchResultsModel : folderModel

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 50
                        color: ListView.isCurrentItem ? Material.color(Material.Green, Material.Shade800) : Material.backgroundColor

                        // Determine item properties based on which model is active
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
                // Preview data is now loaded via WebSocket, no runJavaScript needed
                WebView {
                    id: previewWebView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/workoutpreview/preview.html"
                }
            }
        }
    }
}
