import QtQuick 2.7
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0
import QtCharts 2.2
import Qt.labs.settings 1.0
import QtPositioning 5.5
import QtLocation 5.6

ColumnLayout {
    id: gpxPage
    signal trainprogram_open_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal trainprogram_preview(url name)
    property var selectedFileUrl: ""

    // Modern look: the list on top and the map below on a phone held upright, side by side
    // otherwise (the classic look is always side by side)
    readonly property bool sideBySide: !window.ui.modern || width > height
    // Modern look: side margin of the filter row, the list and the button
    readonly property int modernMargin: Math.max(16, window.contentSideMargin)

    // Case-insensitive name filter shared by the classic and the modern filter field
    function applyFilter(text) {
        var filter = "*"
        for(var i = 0; i<text.length; i++)
           filter+= "[%1%2]".arg(text[i].toUpperCase()).arg(text[i].toLowerCase())
        filter+="*"
        print(filter)
        folderModel.nameFilters = [filter + ".gpx", filter + ".GPX"]
    }

    function openFileAt(i) {
        let fileUrl = folderModel.get(i, 'fileUrl') || folderModel.get(i, 'fileURL');
        if (!fileUrl)
            return
        if (folderModel.isFolder(i)) {
            folderModel.folder = fileUrl
        } else {
            trainprogram_open_clicked(fileUrl);
            popup.open()
        }
    }

    Connections {
        target: rootItem
        function onAndroidDocumentPicked(kind, localUrl) {
            if (kind === "gpx") {
                trainprogram_open_clicked(localUrl)
            }
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
                nameFilters: [qsTr("GPX files (*.gpx *.GPX)"), qsTr("All files (*)")]
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

    function openOtherFolders() {
        console.log("folder is " + rootItem.getWritableAppDir() + 'gpx')
        if (Qt.platform.os === "android") {
            rootItem.openAndroidDocumentPicker("gpx")
        } else {
            fileDialogLoader.active = true
        }
    }

    // Cells by explicit row and column. Modern look upright: filter, list, map, Other folders,
    // one under another; side by side: the filter, the list and Other folders on the left and
    // the map over the whole height on the right. Classic look: the list column and the map in
    // one row, Other folders under the grid as before (the page-level button below).
    GridLayout {
        columns: gpxPage.sideBySide ? 2 : 1
        columnSpacing: 2
        rowSpacing: 12
        Layout.fillWidth: true
        Layout.fillHeight: true

        // Modern look: a filled filter field and a round "up one folder" button
        RowLayout {
            visible: window.ui.modern
            Layout.row: 0
            Layout.column: 0
            spacing: 8
            Layout.fillWidth: true
            Layout.preferredWidth: 100
            Layout.leftMargin: gpxPage.modernMargin
            Layout.rightMargin: gpxPage.sideBySide ? 4 : gpxPage.modernMargin
            Layout.topMargin: 8

            UiTextField {
                id: modernFilterField
                Layout.fillWidth: true
                placeholderText: qsTr("Filter")
                Accessible.name: qsTr("Filter") + ": " + text
                inputMethodHints: Qt.ImhNoPredictiveText
                leftPadding: 44
                onTextChanged: gpxPage.applyFilter(text)
                UiIcon {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    width: 22
                    height: 22
                    name: "search"
                    color: window.ui.textMuted
                }
            }

            UiButton {
                implicitWidth: 48
                leftPadding: 0
                rightPadding: 0
                onClicked: folderModel.folder = folderModel.parentFolder
                Accessible.name: qsTr("Parent folder")
                UiIcon {
                    anchors.centerIn: parent
                    width: 22
                    height: 22
                    name: "drive_folder_upload"
                    color: window.ui.textMain
                }
            }
        }

        // Modern look: cards like the profile list. A tap on a folder opens it; a tap on a
        // route selects it and shows it on the map, a second tap or Open loads it.
        ListView {
            id: modernList
            visible: window.ui.modern
            Layout.row: 1
            Layout.column: 0
            Layout.fillWidth: true
            Layout.fillHeight: gpxPage.sideBySide
            Layout.preferredWidth: 100
            Layout.preferredHeight: gpxPage.sideBySide ? -1 : gpxPage.height * 0.4
            Layout.leftMargin: gpxPage.modernMargin
            Layout.rightMargin: gpxPage.sideBySide ? 4 : gpxPage.modernMargin
            clip: true
            spacing: 8
            boundsBehavior: Flickable.StopAtBounds
            model: window.ui.modern ? folderModel : null
            ScrollIndicator.vertical: ScrollIndicator {}

            // The item, not the index, as in the classic list: a folder change or a new filter
            // rebuilds the rows and can leave the index number as it was over another file
            onCurrentItemChanged: {
                if (currentIndex < 0 || folderModel.isFolder(currentIndex))
                    return
                let fileUrl = folderModel.get(currentIndex, 'fileUrl') || folderModel.get(currentIndex, 'fileURL');
                if (fileUrl) {
                    console.log(fileUrl + ' selected');
                    trainprogram_preview(fileUrl)
                }
            }

            // UiFrame, not Rectangle.border: a thin border breaks up on Android
            delegate: UiFrame {
                id: gpxCard
                readonly property bool selected: ListView.isCurrentItem
                readonly property bool folder: folderModel.isFolder(index)
                width: ListView.view.width
                // Material 3 list item: one line 56, the icon and the button 16 from the edges,
                // the text 16 after the icon
                height: 56
                radius: 16
                fill: selected && !folder ? window.ui.alpha(window.ui.accent, 0.14)
                                          : (cardArea.pressed ? window.ui.surfaceHigh : window.ui.surface)
                strokeWidth: selected && !folder ? 1 : 0
                stroke: window.ui.alpha(window.ui.accent, 0.6)

                // a tap selects a file, a second tap (or a tap on a folder) opens it
                function activate() {
                    if (gpxCard.folder || index === modernList.currentIndex)
                        gpxPage.openFileAt(index)
                    else
                        modernList.currentIndex = index
                }

                MouseArea {
                    id: cardArea
                    anchors.fill: parent
                    onClicked: gpxCard.activate()
                    Accessible.role: Accessible.Button
                    Accessible.name: gpxName.text
                    Accessible.onPressAction: gpxCard.activate()
                }

                Rectangle {
                    id: gpxAvatar
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    width: 40
                    height: 40
                    radius: 20
                    color: window.ui.surfaceHighest
                    UiIcon {
                        anchors.centerIn: parent
                        width: 22
                        height: 22
                        name: gpxCard.folder ? "folder" : "route"
                        color: gpxCard.selected && !gpxCard.folder ? window.ui.accent : window.ui.textMuted
                    }
                }

                Label {
                    id: gpxName
                    // the card is read through cardArea: a touch of the screen reader goes to the
                    // deepest accessible item under the finger, and this label, drawn above
                    // cardArea, would take it from the card (the focus went to the whole window)
                    Accessible.ignored: true
                    anchors.left: gpxAvatar.right
                    anchors.leftMargin: 16
                    anchors.right: openButton.visible ? openButton.left : parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    text: gpxCard.folder ? fileName : fileName.substring(0, fileName.length-4)
                    color: gpxCard.selected && !gpxCard.folder ? window.ui.accent : window.ui.textMain
                    font.pixelSize: window.ui.fontBody
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                UiButton {
                    id: openButton
                    visible: gpxCard.selected && !gpxCard.folder
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Open")
                    highlighted: true
                    onClicked: gpxPage.openFileAt(index)
                }
            }

            Label {
                // On the list itself: children of a ListView land in its content item, empty here
                parent: modernList
                anchors.centerIn: parent
                // only once the folder is read: the model fills in asynchronously
                visible: modernList.count === 0 && folderModel.status === FolderListModel.Ready
                text: qsTr("No GPX files here")
                color: window.ui.textMuted
            }
        }

        ColumnLayout {
            visible: !window.ui.modern
            Layout.row: 0
            Layout.column: 0
            spacing: 0
            Layout.fillHeight: true

            Row
            {
                spacing: 5
                leftPadding: window.contentSideMargin
                Text
                {
                    text:qsTr("Filter")
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                }
                TextField
                {
                    function updateFilter()
                    {
                        gpxPage.applyFilter(filterField.text)
                    }
                    id: filterField
                    onTextChanged: updateFilter()
                }
                Button {
                     anchors.left: mainRect.right
                     anchors.leftMargin: 5
                     text: "←"
                     onClicked: folderModel.folder = folderModel.parentFolder
                }
            }

            ListView {
                Layout.fillWidth: true
                Layout.minimumWidth: 50
                Layout.preferredWidth: 100
                Layout.minimumHeight: 150
                Layout.fillHeight: true
                ScrollBar.vertical: ScrollBar {}
                id: list
                FolderListModel {
                    id: folderModel
                    nameFilters: ["*.gpx", "*.GPX"]
                    folder: "file://" + rootItem.getWritableAppDir() + 'gpx'
                    showDotAndDotDot: false
                    showDirs: true
                    sortField: "Name"
                    showDirsFirst: true
                }
                model: window.ui.modern ? null : folderModel
                delegate: Component {
                    Rectangle {
                        property alias textColor: fileTextBox.color
                        width: parent.width
                        height: window.textScale !== 1 ? Math.max(40, fileTextBox.implicitHeight) : 40
                        color: Material.backgroundColor
                        z: 1
                        Item {
                            id: root
                            x: window.contentSideMargin
                            property alias text: fileTextBox.text
                            property int spacing: 30
                            width: fileTextBox.width + spacing
                            height: fileTextBox.height
                            clip: true
                            Text {
                                id: fileTextBox
                                color: (!folderModel.isFolder(index)?Material.color(Material.Grey):Material.color(Material.Orange))
                                font.pixelSize: window.textScale === 1 ? Qt.application.font.pixelSize * 1.6 : (Qt.application.font.pixelSize > 0 ? window.fontPx(window.appFontPx * 1.6) : window.plainFontPx)
                                text: fileName.substring(0, fileName.length-4)
                                NumberAnimation on x {
                                    Component.onCompleted: {
                                        if(fileName.length > 30) {
                                            running: true;
                                        } else {
                                            stop();
                                        }
                                    }
                                    from: 0; to: -root.width; duration: 20000; loops: Animation.Infinite
                                }
                                Text {
                                  x: root.width
                                  text: fileTextBox.text
                                  color: Material.color(Material.Grey)
                                  font.pixelSize: window.textScale === 1 ? Qt.application.font.pixelSize * 1.6 : (Qt.application.font.pixelSize > 0 ? window.fontPx(window.appFontPx * 1.6) : window.plainFontPx)
                                }
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            z: 100
                            onClicked: {
                                console.log('onclicked ' + index+ " count "+list.count);
                                if (index == list.currentIndex) {
                                    let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                                    if (fileUrl && !folderModel.isFolder(list.currentIndex)) {
                                        trainprogram_open_clicked(fileUrl);
                                        popup.open()
                                    } else {
                                        folderModel.folder = fileURL
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
                }
                focus: true
                onCurrentItemChanged: {
                    let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                    if (fileUrl) {
                        list.currentItem.textColor = Material.color(Material.Yellow)
                        console.log(fileUrl + ' selected');
                        trainprogram_preview(fileUrl)
                    }
                }
                Component.onCompleted: {

                }
            }
        }

        ScrollView {
            // The map fills the view, there is nothing to scroll: in the modern look the bar
            // would only be a grey stripe next to the map
            ScrollBar.vertical.policy: window.ui.modern ? ScrollBar.AlwaysOff : ScrollBar.AlwaysOn
            // Padding, not a margin: the content moves in, the scroll bar stays at the edge
            rightPadding: window.ui.modern && !gpxPage.sideBySide ? gpxPage.modernMargin : window.contentSideMargin
            leftPadding: window.ui.modern && !gpxPage.sideBySide ? gpxPage.modernMargin : 0
            Layout.row: window.ui.modern && !gpxPage.sideBySide ? 2 : 0
            Layout.column: gpxPage.sideBySide ? 1 : 0
            Layout.rowSpan: window.ui.modern && gpxPage.sideBySide ? 3 : 1
            Layout.topMargin: window.ui.modern && gpxPage.sideBySide ? 8 : 0
            Layout.bottomMargin: window.ui.modern && gpxPage.sideBySide ? 8 : 0
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 100
            Layout.preferredWidth: 200

            Row {
                id: row
                anchors.fill: parent
                // Route figures for the modern chips; -1 until a route is loaded
                property real routeKm: -1
                property real routeGain: 0
                readonly property bool chips: window.ui.modern && routeKm >= 0

                Text {
                    id: distance
                    width: parent.width
                    // Modern look: the text gives way to the chips below, laid over it
                    height: row.chips ? routeChips.height + 8 : implicitHeight
                    text: rootItem.previewWorkoutDescription
                    font.pixelSize: window.ui.modern ? window.ui.fontLabel : window.fontPx(16)
                    color: window.ui.modern ? window.ui.textMuted : "white"
                    bottomPadding: window.ui.modern ? 8 : 0
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter

                    Row {
                        id: routeChips
                        visible: row.chips
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        spacing: 8

                        Repeater {
                            model: [
                                { icon: "straighten", text: qsTr("%1 km").arg(row.routeKm.toFixed(1)) },
                                { icon: "trending_up", text: qsTr("%1 m").arg(Math.round(row.routeGain)) }
                            ]
                            Rectangle {
                                height: 32
                                width: chipRow.implicitWidth + 24
                                radius: 16
                                color: window.ui.surfaceHigh
                                Row {
                                    id: chipRow
                                    anchors.centerIn: parent
                                    spacing: 6
                                    UiIcon {
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 18
                                        height: 18
                                        name: modelData.icon
                                        color: window.ui.textMuted
                                    }
                                    Label {
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: modelData.text
                                        color: window.ui.textMain
                                        font.pixelSize: window.ui.fontLabel
                                        font.weight: Font.DemiBold
                                    }
                                }
                            }
                        }
                    }
                }

                Plugin {
                    id: osmMapPlugin
                    name: "osm"
                    PluginParameter { name: "osm.useragent"; value: "QZ Fitness" }
                }

                Map {
                    height: parent.height - distance.height
                    width: parent.width
                    id: map
                    anchors.top: distance.bottom
                    plugin: osmMapPlugin
                    zoomLevel: 14
                    center: pathController.center
                    visible: true

                    MapPolyline {
                        id: pl
                        line.width: window.ui.modern ? 4 : 3
                        line.color: window.ui.modern ? window.ui.accent : 'red'
                    }
                    Component.onCompleted: {
                        console.log("Dimensions: ", width, height)
                    }
                }

                function loadPath(){
                    var lines = []
                    var elevationGain = 0
                    var offsetElevation = 0
                    for(var i = 0; i < pathController.geopath.size(); i++){
                        if(i > 0 && pathController.geopath.coordinateAt(i).altitude > pathController.geopath.coordinateAt(i-1).altitude)
                            elevationGain = elevationGain + (pathController.geopath.coordinateAt(i).altitude - pathController.geopath.coordinateAt(i-1).altitude)
                        lines[i] = pathController.geopath.coordinateAt(i)
                    }
                    row.routeKm = lines.length > 0 ? pathController.distance : -1
                    row.routeGain = elevationGain
                    distance.text = window.ui.modern ? "" : qsTr("Distance %1 km Elevation Gain: %2 meters").arg(pathController.distance.toFixed(1)).arg(elevationGain.toFixed(1))
                    return lines;
                }

                Connections{
                    target: pathController
                    onGeopathChanged: {
                        pl.path = row.loadPath();
                    }
                    onCenterChanged: {
                        map.center = pathController.center;
                    }
                }

                Component.onCompleted: pl.path = loadPath()
            }
        }

        UiButton {
            visible: window.ui.modern
            Layout.row: gpxPage.sideBySide ? 2 : 3
            Layout.column: 0
            Layout.fillWidth: true
            Layout.preferredWidth: 100
            Layout.leftMargin: gpxPage.modernMargin
            Layout.rightMargin: gpxPage.sideBySide ? 4 : gpxPage.modernMargin
            // 16 to the pill: UiButton has a 6 inset below it
            Layout.bottomMargin: 10
            text: qsTr("Other folders")
            onClicked: gpxPage.openOtherFolders()
        }
    }

    Button {
        id: searchButton
        visible: !window.ui.modern
        Layout.fillWidth: true
        Layout.preferredHeight: 50
        text: qsTr("Other folders")
        onClicked: gpxPage.openOtherFolders()
    }
}
