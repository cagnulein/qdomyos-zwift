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
        rootItem.trainingProgramsPageOpened()
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

                // programs of a previous install were copied back into the training folder
                Connections {
                    target: rootItem
                    function onTrainingProgramsRestored() {
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
                        // || "": while the folder is set again (onTrainingProgramsRestored) the model
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

                    UiButton {
                        Layout.fillWidth: true
                        height: 50
                        text: qsTr("Other folders")
                        onClicked: {
                            if (Qt.platform.os === "android") {
                                rootItem.openAndroidDocumentPicker("training")
                            } else {
                                fileDialogLoader.active = true
                            }
                        }
                    }

                    // programs of a previous install that no bundled program points to are
                    // found only on request
                    UiButton {
                        Layout.fillWidth: true
                        height: 50
                        visible: Qt.platform.os === "android"
                        text: qsTr("Restore programs")
                        onClicked: rootItem.offerTrainingRestore()
                    }
                }
            }
        }

        // DETAIL VIEW - Anteprima Workout
        Component {
            id: detailView

            ColumnLayout {
                id: detailColumn
                spacing: 10
                // Landscape: the description with the tags on the left, the chart over the whole
                // height on the right, as the GPX page; upright one under another
                readonly property bool sideBySide: width > height

                // Header con pulsanti
                RowLayout {
                    id: detailHeader
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

                GridLayout {
                    id: detailBody
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    columns: detailColumn.sideBySide ? 2 : 1
                    columnSpacing: 6
                    rowSpacing: detailColumn.spacing

                    ColumnLayout {
                        id: textColumn
                        Layout.row: 0
                        Layout.column: 0
                        Layout.fillWidth: true
                        Layout.fillHeight: detailColumn.sideBySide
                        // the two columns share the width equally
                        Layout.preferredWidth: 1
                        spacing: detailColumn.spacing
                        // from the data, not from the children: their visibility follows this one
                        visible: !window.ui.modern || rootItem.previewWorkoutDescription !== ""
                            || rootItem.previewWorkoutTags !== ""

                        // Descrizione workout
                        // Upright: folded to at most 8 lines, and to no more than 30 % of the page below the
                        // buttons, with "Show more" under its right end: the chart below is a native view
                        // that takes only the height left, and a long description (the bundled programs cite
                        // their sources) left it little. Unfolded, the text scrolls in at most half of the
                        // page below the buttons. Either way it keeps to textRoom: on a low page the button
                        // and the tags stay on the page and the chart keeps 80. Side by side: the whole text,
                        // no fold, scrolled in its column above the tags
                        ColumnLayout {
                            id: descriptionBlock
                            property bool expanded: false
                            // side by side: the page below the buttons less the margins of the buttons
                            // row (10), the spacing under it, the own margins (20), the tags and the
                            // spacing before the filler under them. Not from textColumn.height: the
                            // column grows to its content, and the text pushed the tags and the chart
                            // past the bottom of the page
                            readonly property real sideRoom: Math.max(descriptionMetrics.lineSpacing,
                                pageBelowButtons - 10 - detailColumn.spacing - 20
                                - (tagsText.visible ? tagsText.height + textColumn.spacing : 0) - textColumn.spacing)
                            readonly property real pageBelowButtons: Math.max(0, detailColumn.height - detailHeader.height)
                            // the page below the buttons less all but the text: the margins of the buttons row (10),
                            // the spacing of the page, the own margins (20), the button (48), the tags, the chart
                            readonly property real textRoom: Math.max(descriptionMetrics.lineSpacing,
                                pageBelowButtons - 10 - 3 * detailColumn.spacing - 20 - 48
                                - (tagsText.visible ? tagsText.height : 0) - 80)
                            readonly property int foldedLines: Math.max(1, Math.min(8,
                                Math.floor(Math.min(pageBelowButtons * 0.3, textRoom) / Math.max(1, descriptionMetrics.lineSpacing))))
                            // the height of the whole lines that fit in h: the line pitch as the text
                            // lays it out (lineSpacing is a little off, and over ten lines the top of
                            // the next one showed under the last)
                            function wholeLines(h) {
                                var line = descriptionText.lineCount > 0
                                    ? descriptionText.contentHeight / descriptionText.lineCount
                                    : descriptionMetrics.lineSpacing
                                line = Math.max(1, line)
                                return Math.max(1, Math.floor(h / line)) * line
                            }
                            Layout.fillWidth: true
                            Layout.margins: 10
                            spacing: 0
                            // Modern look: an empty line kept its height and margins, a gap above the chart
                            visible: !window.ui.modern || rootItem.previewWorkoutDescription !== ""

                            Flickable {
                                id: descriptionFlick
                                Layout.fillWidth: true
                                // whole lines, so the last one shown is not cut in half
                                Layout.preferredHeight: detailColumn.sideBySide
                                    ? Math.min(contentHeight, descriptionBlock.wholeLines(descriptionBlock.sideRoom))
                                    : descriptionBlock.expanded
                                    ? Math.min(contentHeight, descriptionBlock.wholeLines(
                                        Math.min(descriptionBlock.pageBelowButtons / 2, descriptionBlock.textRoom)))
                                    : contentHeight
                                contentHeight: descriptionText.height
                                interactive: contentHeight > height
                                clip: true
                                boundsBehavior: Flickable.StopAtBounds
                                ScrollIndicator.vertical: ScrollIndicator { }

                                Text {
                                    id: descriptionText
                                    width: descriptionFlick.width
                                    text: rootItem.previewWorkoutDescription
                                    font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(14)
                                    font.weight: window.ui.modern ? Font.DemiBold : Font.Bold
                                    color: window.ui.ink("white")
                                    // Wrap, not WordWrap: a word wider than the column (a link) is broken, not
                                    // cut on one line - that cut set truncated, and "Show more" unfolded nothing
                                    wrapMode: Text.Wrap
                                    maximumLineCount: detailColumn.sideBySide || descriptionBlock.expanded
                                        ? 100000 : descriptionBlock.foldedLines
                                    elide: Text.ElideRight
                                    horizontalAlignment: Text.AlignLeft
                                }

                                FontMetrics {
                                    id: descriptionMetrics
                                    font: descriptionText.font
                                }
                            }

                            // Shown only when the folded text is really cut. Modern look: a tonal pill (the
                            // accent at 16 %, 28 % pressed, as the profile chip of the drawer) ending at the
                            // right edge of the text; classic: the bare label, its touch area past the text
                            AbstractButton {
                                id: moreButton
                                Layout.alignment: Qt.AlignRight
                                Layout.rightMargin: window.ui.modern ? 0 : -8
                                visible: !detailColumn.sideBySide && (descriptionBlock.expanded || descriptionText.truncated)
                                implicitHeight: 48
                                leftPadding: window.ui.modern ? 12 : 8
                                rightPadding: 8
                                implicitWidth: moreRow.implicitWidth + leftPadding + rightPadding
                                text: descriptionBlock.expanded ? qsTr("Show less") : qsTr("Show more")
                                onClicked: {
                                    descriptionBlock.expanded = !descriptionBlock.expanded
                                    if (!descriptionBlock.expanded)
                                        descriptionFlick.contentY = 0
                                }

                                background: Item {
                                    Rectangle {
                                        visible: window.ui.modern
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: parent.width
                                        height: 32
                                        radius: height / 2
                                        color: window.ui.alpha(window.ui.accent, moreButton.pressed ? 0.28 : 0.16)
                                    }
                                }

                                contentItem: Row {
                                    id: moreRow
                                    spacing: 4
                                    opacity: !window.ui.modern && moreButton.pressed ? 0.6 : 1

                                    Label {
                                        anchors.verticalCenter: parent.verticalCenter
                                        // the button carries the name; the label is no second node for TalkBack
                                        Accessible.ignored: true
                                        text: moreButton.text
                                        font.pixelSize: window.ui.modern ? window.ui.fontLabel : window.fontPx(14)
                                        font.weight: Font.Medium
                                        color: window.ui.modern ? window.ui.accent : Material.accent
                                    }

                                    // turned over when unfolded, as the chevron of a settings section
                                    UiIcon {
                                        anchors.verticalCenter: parent.verticalCenter
                                        width: 20
                                        height: 20
                                        name: "expand_more"
                                        color: window.ui.modern ? window.ui.accent : Material.accent
                                        rotation: descriptionBlock.expanded ? 180 : 0
                                        Behavior on rotation { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                                    }
                                }
                            }
                        }

                        Text {
                            id: tagsText
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            Layout.rightMargin: 10
                            text: rootItem.previewWorkoutTags
                            visible: !window.ui.modern || text !== ""
                            font.pixelSize: window.ui.modern ? window.ui.fontCaption : window.fontPx(12)
                            wrapMode: Text.WordWrap
                            color: window.ui.modern ? window.ui.textMuted : Material.color(Material.Grey, Material.Shade400)
                            horizontalAlignment: Text.AlignLeft
                        }

                        // side by side: the rest of the column, so the tags stay under the text
                        Item {
                            Layout.fillHeight: true
                            visible: detailColumn.sideBySide
                        }
                    }

                    // WebView con grafico
                    // Preview data is now loaded via WebSocket, no runJavaScript needed
                    Item {
                        Layout.row: detailColumn.sideBySide ? 0 : 1
                        Layout.column: detailColumn.sideBySide ? 1 : 0
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: 1

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
}
