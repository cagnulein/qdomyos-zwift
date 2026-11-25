import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtCharts 2.15
import Qt.labs.calendar 1.0
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1 as P
import QtQuick.Dialogs 1.0 as Dialogs

Page {
    id: workoutHistoryPage

    // Modern look: theme colours and icons instead of the fixed light palette and emoji of
    // this page; the classic look keeps them
    readonly property bool modern: window.ui.modern


    Settings {
        id: settings
        property bool miles_unit: false
    }

    // Signal for chart preview
    signal fitfile_preview_clicked(var url)

    // a pending offer to recover the workouts of a previous install is made here
    Component.onCompleted: rootItem.historyPageOpened()

    // Helper function to wrap text with emoji font only on Android. Android shows an emoji
    // only with the downloaded font, so until it is there the emoji is left out
    function wrapEmoji(emoji) {
        if (Qt.platform.os !== "android")
            return emoji;
        if (!fontManager || fontManager.emojiFontFamily === "Arial")
            return "";
        // A trailing space stays out of the emoji font: its space is a wide one
        var space = "";
        if (emoji.slice(-1) === " ") {
            space = " ";
            emoji = emoji.slice(0, -1);
        }
        return '<font face="' + fontManager.emojiFontFamily + '">' + emoji + '</font>' + space;
    }

    // Sport type to ShapeIcon name, drawn instead of the emoji on Android
    function getSportIconName(sport) {
        switch(parseInt(sport)) {
            case 1:  // FIT_SPORT_RUNNING
                return "directions_run";
            case 11: // FIT_SPORT_WALKING
                return "directions_walk";
            case 2:  // FIT_SPORT_CYCLING
                return "pedal_bike";
            case 15: // FIT_SPORT_ROWING
                return "rowing";
            default:
                return "fitness_center";
        }
    }

    // Sport type to icon mapping (using FIT_SPORT values)
    function getSportIcon(sport) {
        switch(parseInt(sport)) {
            case 1:  // FIT_SPORT_RUNNING
            case 11: // FIT_SPORT_WALKING
                return "🏃"; // Running/Walking
            case 2:  // FIT_SPORT_CYCLING
                return "🚴"; // Cycling
            case 4:  // FIT_SPORT_FITNESS_EQUIPMENT (Elliptical)
                return "⭕"; // Elliptical
            case 15: // FIT_SPORT_ROWING
                return "🚣"; // Rowing
            case 84: // FIT_SPORT_JUMPROPE
                return "🪢"; // Jump Rope
            default: 
                return "💪"; // Generic workout
        }
    }

    // Opens with any services or none: Share is always there
    function openUploadMenu(delegateItem, workoutId, workoutTitle) {
        if (!workoutModel) {
            return
        }

        var details = workoutModel.getWorkoutDetails(workoutId)
        if (!details.filePath || details.filePath === "") {
            return
        }

        uploadMenu.workoutId = workoutId
        uploadMenu.workoutTitle = workoutTitle
        uploadMenu.filePath = details.filePath

        var popupPoint = delegateItem.mapToItem(workoutHistoryPage, delegateItem.width / 2, delegateItem.height / 2)
        if (workoutHistoryPage.modern) {
            modernUploadMenu.openFor(delegateItem, popupPoint.x)
            return
        }
        uploadMenu.x = Math.max(8, popupPoint.x - 40)
        uploadMenu.y = Math.max(8, popupPoint.y - 20)
        uploadMenu.open()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // Modern header: the title on the left, the streak (or the date filter) under it as a
        // small chip, the calendar and import on the right. The streak used to be a big orange
        // banner at the bottom, the loudest thing on the page even at zero days. The filter
        // chip clears the filter itself: a "Clear Filter" button beside the round ones cut the
        // title down to a few letters on a phone
        Item {
            id: modernHeader
            visible: workoutHistoryPage.modern
            Layout.fillWidth: true
            Layout.leftMargin: Math.max(16, window.contentSideMargin)
            Layout.rightMargin: Math.max(16, window.contentSideMargin)
            // The title and the buttons share the top line, the chip goes under it: centred in
            // the whole header, the buttons slid down when the chip came and up when it left
            implicitHeight: Math.max(modernHeaderButtons.implicitHeight,
                                     modernTitleColumn.anchors.topMargin + modernTitleColumn.implicitHeight)

            readonly property int streak: workoutModel ? workoutModel.currentStreak : 0
            readonly property bool filtered: workoutModel ? workoutModel.isDateFiltered : false

            Column {
                id: modernTitleColumn
                anchors.left: parent.left
                anchors.right: modernHeaderButtons.left
                anchors.rightMargin: 8
                anchors.top: parent.top
                anchors.topMargin: Math.max(0, (modernHeaderButtons.implicitHeight - modernTitleText.implicitHeight) / 2)
                spacing: 6

                Text {
                    id: modernTitleText
                    width: parent.width
                    text: qsTr("Workout History")
                    font.pixelSize: window.ui.fontHeadline
                    font.weight: Font.DemiBold
                    color: window.ui.textMain
                    elide: Text.ElideRight
                }

                AbstractButton {
                    id: filterChip
                    visible: modernHeader.filtered
                    // From the text's implicit width, not the row's: the row's width follows the
                    // elided text, and measuring by it would loop
                    width: Math.min(implicitWidth, parent.width)
                    implicitWidth: 24 + 2 * 18 + 2 * filterChipRow.spacing + filterChipText.implicitWidth
                    // 32 as an M3 chip, taller with a bigger text
                    height: Math.max(32, filterChipText.implicitHeight + 12)
                    onClicked: workoutModel.clearDateFilter()
                    background: Rectangle {
                        radius: 16
                        color: window.ui.alpha(window.ui.accent, filterChip.down ? 0.28 : 0.16)
                    }
                    contentItem: Item {
                        Row {
                            id: filterChipRow
                            anchors.verticalCenter: parent.verticalCenter
                            x: 12
                            spacing: 6
                            UiIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 18
                                height: 18
                                name: "calendar_month"
                                color: window.ui.accent
                            }
                            Text {
                                id: filterChipText
                                anchors.verticalCenter: parent.verticalCenter
                                width: Math.max(0, Math.min(implicitWidth, filterChip.width - (filterChip.implicitWidth - implicitWidth)))
                                // "пн, 5 жовт.", the year only when it is not this one: the month
                                // as a word reads the same in every language, 5.10 does not
                                text: {
                                    if (!modernHeader.filtered)
                                        return ""
                                    // Qt 5 hands a QDate over as midnight UTC: read west of
                                    // Greenwich, that is the day before
                                    var d0 = workoutModel.filteredDate
                                    var d = d0.getUTCHours() === 0 && d0.getUTCMinutes() === 0
                                            ? new Date(d0.getUTCFullYear(), d0.getUTCMonth(), d0.getUTCDate())
                                            : d0
                                    var f = d.getFullYear() === new Date().getFullYear() ? "ddd, d MMM" : "ddd, d MMM yyyy"
                                    return d.toLocaleDateString(Qt.locale(), f)
                                }
                                // chips: fontLabel (notes/modern-ui-style-guide.md)
                                font.pixelSize: window.ui.fontLabel
                                font.weight: Font.DemiBold
                                color: window.ui.accent
                                elide: Text.ElideRight
                            }
                            UiIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                width: 18
                                height: 18
                                name: "close"
                                color: window.ui.accent
                            }
                        }
                    }
                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("Clear Filter")
                }

                Rectangle {
                    visible: !modernHeader.filtered && modernHeader.streak > 0
                    // The height of the filter chip in the same place: switching between them
                    // keeps the header still
                    width: streakChipRow.implicitWidth + 24
                    height: Math.max(32, streakChipRow.implicitHeight + 12)
                    radius: 16
                    color: window.ui.alpha(window.ui.accent, 0.16)

                    Row {
                        id: streakChipRow
                        anchors.centerIn: parent
                        spacing: 4
                        UiIcon {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 18
                            height: 18
                            name: "local_fire_department"
                            color: window.ui.accent
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: (modernHeader.streak !== 1 ? qsTr("%1 days streak") : qsTr("%1 day streak")).arg(modernHeader.streak)
                            font.pixelSize: window.ui.fontLabel
                            font.weight: Font.DemiBold
                            color: window.ui.accent
                        }
                    }
                }
            }

            Row {
                id: modernHeaderButtons
                anchors.right: parent.right
                anchors.top: parent.top
                // Icon buttons as elsewhere in the modern look (the parent folder button of the
                // GPX and workout lists): UiButton 48 wide, its pill 36 high inside 48 to touch
                spacing: 4

                UiButton {
                    anchors.verticalCenter: parent.verticalCenter
                    implicitWidth: 48
                    leftPadding: 0
                    rightPadding: 0
                    onClicked: calendarPopup.open()
                    Accessible.name: qsTr("Calendar")
                    UiIcon {
                        anchors.centerIn: parent
                        width: 22
                        height: 22
                        name: "calendar_month"
                        color: window.ui.textMain
                    }
                }

                UiButton {
                    id: modernImportButton
                    anchors.verticalCenter: parent.verticalCenter
                    implicitWidth: 48
                    // one import at a time, as the classic button: a second copy would report
                    // its count into the same block; a disabled item is left out for a screen
                    // reader (Qt 5.15 does not report it as disabled)
                    enabled: !(rootItem && rootItem.fitImportRunning)
                    opacity: enabled ? 1 : 0.4
                    Accessible.ignored: !enabled
                    leftPadding: 0
                    rightPadding: 0
                    onClicked: modernImportMenu.openUnder(modernImportButton)
                    // the button opens the menu of both imports: the name of the classic button, not of an item
                    Accessible.name: qsTr("Import workouts")
                    // The icon of loading settings from a file in the toolbar: both take a file
                    // from the phone
                    UiIcon {
                        anchors.centerIn: parent
                        width: 22
                        height: 22
                        name: "upload_file"
                        color: window.ui.textMain
                    }
                }
            }
        }

        // Header
        Rectangle {
            visible: !workoutHistoryPage.modern
            Layout.fillWidth: true
            height: 60
            // a bigger text: the title and the filter line keep inside the header (the layout reads
            // the height only once, a preferred height it follows)
            Layout.preferredHeight: window.textScale !== 1 ? Math.max(60, headerTitle.height + 12) : -1
            color: "#f5f5f5"

            // Calendar Icon Button - positioned absolutely on the left
            Button {
                id: calendarButton
                Accessible.name: qsTr("Filter by date")
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: window.contentSideMargin
                width: 48
                height: 48
                
                background: Rectangle {
                    radius: 8
                    color: calendarButton.pressed ? "#e0e0e0" : "#f0f0f0"
                    border.color: "#d0d0d0"
                    border.width: 1
                }
                
                contentItem: Item {
                    Text {
                        anchors.fill: parent
                        visible: Qt.platform.os !== "android"
                        text: "📅"
                        font.pixelSize: 26
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    // Android shows an emoji only with a downloaded font, so the icon is drawn
                    ShapeIcon {
                        anchors.centerIn: parent
                        visible: Qt.platform.os === "android"
                        name: "calendar_month"
                        size: 26
                        color: "#444444"
                    }
                }
                
                onClicked: {
                    calendarPopup.open()
                }
            }

            // Clear Filter Button - right of the calendar; before the title and the import button, as a screen reader goes through the header in this order
            Button {
                id: clearFilterButton
                Accessible.name: qsTr("Clear Filter")
                // next to the calendar that set the filter; small, so the title keeps its room
                anchors.left: calendarButton.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 6
                width: 32
                height: 32
                visible: workoutModel && workoutModel.isDateFiltered

                background: Rectangle {
                    radius: 16
                    color: clearFilterButton.pressed ? "#ff6666" : "#ff8888"
                    border.color: "#ff4444"
                    border.width: 1
                }

                contentItem: Text {
                    text: "×"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    workoutModel.clearDateFilter()
                }
            }

            // Title with filter status - between the buttons, shrinking on narrow screens
            Column {
                id: headerTitle
                anchors.left: clearFilterButton.visible ? clearFilterButton.right : calendarButton.right
                anchors.right: importButton.left
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Workout History")
                    font.pixelSize: window.fontPx(24)
                    font.bold: true
                    fontSizeMode: Text.HorizontalFit
                    minimumPixelSize: 14
                    // the fitted (smaller) line keeps the height of the full font size (QTBUG-55873):
                    // centred, so a long translation or a bigger text stays in the middle of the header
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: workoutModel && workoutModel.isDateFiltered ?
                          qsTr("Filtered: %1").arg(workoutModel.filteredDate.toLocaleDateString()) : ""
                    font.pixelSize: window.fontPx(12)
                    color: "#666666"
                    elide: Text.ElideRight
                    visible: workoutModel && workoutModel.isDateFiltered
                }
            }

            // Import Button - on the right: a single .fit file, or every workout of the QZ folder
            Button {
                id: importButton
                Accessible.name: qsTr("Import workouts")
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: window.contentSideMargin
                width: 48
                height: 48
                // one import at a time: a second copy would report its count into the same block
                enabled: !(rootItem && rootItem.fitImportRunning)
                opacity: enabled ? 1 : 0.4
                // Qt 5.15 does not report a disabled item to TalkBack (fixed in Qt 6 by
                // qtdeclarative 20fd2902a6): left in, it would be read as a button that works
                Accessible.ignored: !enabled

                background: Rectangle {
                    radius: 8
                    color: importButton.pressed ? "#e0e0e0" : "#f0f0f0"
                    border.color: "#d0d0d0"
                    border.width: 1
                }

                contentItem: Item {
                    Text {
                        anchors.fill: parent
                        visible: Qt.platform.os !== "android"
                        text: "📥"
                        font.pixelSize: 26
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    // Android shows an emoji only with a downloaded font, so the icon is drawn
                    ShapeIcon {
                        anchors.centerIn: parent
                        visible: Qt.platform.os === "android"
                        name: "download"
                        size: 26
                        color: "#444444"
                    }
                }

                onClicked: importMenu.popup(importButton, 0, importButton.height)
            }
        }

        // Loading indicator: the short parts (loading the list, checking the fit folder at start,
        // reading a picked file); a pass over many files shows its count below instead
        BusyIndicator {
            id: loadingIndicator
            Layout.alignment: Qt.AlignHCenter
            visible: ((workoutModel ? (workoutModel.isLoading || workoutModel.isDatabaseProcessing) : false) ||
                      (rootItem && rootItem.fitImportRunning)) && !importProgress.visible
            running: visible
        }

        // A workout import (copying the files out of the picked folder, then adding them to the
        // history) and the new files of the fit folder at start: how far it is, above the list
        UiProgress {
            id: importProgress
            Layout.fillWidth: true
            // the edges of the workout cards below
            Layout.leftMargin: window.contentSideMargin
            Layout.rightMargin: window.contentSideMargin
            topPadding: 8
            done: rootItem ? rootItem.fitImportDone : 0
            // one file (a picked workout) is over at once: the spinner, not a count flashing 1 / 1
            total: rootItem && rootItem.fitImportTotal > 1 ? rootItem.fitImportTotal : 0
            text: rootItem && rootItem.fitImportAdding ? qsTr("Adding workouts to the history")
                                                       : qsTr("Copying workout files")
            accessibleText: rootItem && rootItem.fitImportAdding ? qsTr("Adding workouts to the history: %1 of %2")
                                                                 : qsTr("Copying workout files: %1 of %2")
        }

        // Workout List
        ListView {
            id: workoutListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            // the gap under the progress is its own (UiProgress)
            Layout.topMargin: importProgress.visible ? -parent.spacing : 0
            Layout.bottomMargin: streakBanner.visible ? streakBanner.height + 10 : 10
            model: workoutModel
            // Modern look: the gap between cards is only the card's own margins (2 x 4 = 8)
            spacing: workoutHistoryPage.modern ? 0 : 8
            clip: true
            // Modern look: a thin bar while the list moves, as in the other lists of the modern look; the
            // classic look has none, as in master
            ScrollIndicator.vertical: ScrollIndicator { visible: workoutHistoryPage.modern }

            // Modern look: an empty list says so, instead of a blank page
            Column {
                // On the list itself: children of a ListView land in its content item
                parent: workoutListView
                anchors.centerIn: parent
                width: Math.min(parent.width - 48, 360)
                spacing: 10
                // not while a workout import runs either: its progress above says what is coming
                visible: workoutHistoryPage.modern && workoutListView.count === 0
                         && !(workoutModel && (workoutModel.isLoading || workoutModel.isDatabaseProcessing))
                         && !(rootItem && rootItem.fitImportRunning)

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 88
                    height: 88
                    radius: 44
                    color: window.ui.alpha(window.ui.accent, 0.14)
                    UiIcon {
                        anchors.centerIn: parent
                        width: 44
                        height: 44
                        name: "history"
                        color: window.ui.accent
                    }
                }
                Text {
                    width: parent.width
                    topPadding: 4
                    text: workoutModel && workoutModel.isDateFiltered ? qsTr("No workouts on this day")
                                                                      : qsTr("No workouts yet")
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.pixelSize: window.ui.fontTitle
                    font.weight: Font.DemiBold
                    color: window.ui.textMain
                }
                Text {
                    width: parent.width
                    visible: !(workoutModel && workoutModel.isDateFiltered)
                    text: qsTr("Finished workouts appear here: open one to see its charts.")
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.pixelSize: window.ui.fontLabel
                    color: window.ui.textMuted
                }
            }

            onContentYChanged: {
                // Hide banner when scrolling down, show when at top (the modern look has
                // the streak as a chip in the header instead)
                streakBanner.visible = !workoutHistoryPage.modern && contentY <= 20
            }

            delegate: SwipeDelegate {
                id: swipeDelegate
                // The row is a list item for Qt 5, which has no press action for a screen reader: with
                // the handler a double tap opens the workout
                Accessible.onPressAction: swipeDelegate.clicked()
                // The row has no text of its own: without a name a screen reader stops on the texts
                // inside it, which cannot be pressed, and the workout cannot be opened
                Accessible.name: title + ", " + date + ", " + duration
                // The width of the list: a row filtered out or deleted is destroyed with no parent, and
                // parent.width then logged "Cannot read property 'width' of null"
                width: ListView.view ? ListView.view.width : 0
                // Modern look: three lines (the calories join the stats), a list item of three lines:
                // the card at least 88 high, growing with a bigger text (the lines + 8 above and
                // below), and the gap of 8 between the cards.
                // Classic with a bigger text: the card grows with its lines (margins 8 + 12 on each side)
                height: workoutHistoryPage.modern ? Math.max(88, workoutInfo.implicitHeight + 16) + 8
                    : window.textScale !== 1 ? Math.max(135, workoutInfo.implicitHeight + 40) : 135

                Component.onCompleted: {
                    console.log("Delegate data:", JSON.stringify({
                        sport: sport,
                        title: title,
                        date: date,
                        duration: duration,
                        distance: distance,
                        calories: calories,
                        id: id
                    }))
                }

                swipe.right: Rectangle {
                    width: parent.width
                    height: parent.height
                    // Modern look: the danger colour of the theme; the ink on it is accentInk, white on
                    // the light danger and dark on the dark one (white on #FF8A80 is about 2.3 : 1)
                    color: workoutHistoryPage.modern ? window.ui.danger : "#FF4444"
                    clip: true

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 20
                        spacing: 4

                        ShapeIcon {
                            visible: Qt.platform.os === "android"
                            name: "delete"
                            size: 18
                            color: workoutHistoryPage.modern ? window.ui.accentInk : "white"
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: Qt.platform.os === "android" ? qsTr("Delete") : "🗑️ " + qsTr("Delete")
                            color: workoutHistoryPage.modern ? window.ui.accentInk : "white"
                            font.pixelSize: workoutHistoryPage.modern ? window.ui.fontBody : window.fontPx(16)
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                swipe.onCompleted: {
                    if (workoutHistoryPage.modern) {
                        modernDeleteDialog.workoutId = model.id
                        modernDeleteDialog.workoutTitle = model.title
                        modernDeleteDialog.swipeItem = swipeDelegate
                        modernDeleteDialog.open()
                        return
                    }
                    // Show confirmation dialog
                    confirmDialog.workoutId = model.id
                    confirmDialog.workoutTitle = model.title
                    confirmDialog.open()
                }

                // Modern look: the long press card is placed by this card and lights it up
                readonly property Item cardItem: rowCard
                readonly property string workoutDate: date

                // Card-like container
                Rectangle {
                    id: rowCard
                    anchors.fill: parent
                    anchors.margins: workoutHistoryPage.modern ? 4 : 8
                    // Modern look: the page margin of the header, so the column has one edge
                    anchors.leftMargin: workoutHistoryPage.modern ? Math.max(16, window.contentSideMargin) : window.contentSideMargin
                    anchors.rightMargin: workoutHistoryPage.modern ? Math.max(16, window.contentSideMargin) : window.contentSideMargin
                    radius: workoutHistoryPage.modern ? 16 : 10
                    // Modern look: the workout of the open long press card is tinted with the accent
                    color: !workoutHistoryPage.modern ? "white"
                         : modernUploadMenu.visible && modernUploadMenu.row === swipeDelegate
                           ? Qt.tint(window.ui.surface, window.ui.alpha(window.ui.accent, 0.10))
                           : window.ui.surface
                    border.color: workoutHistoryPage.modern ? "transparent" : "#e0e0e0"

                    // Workout Type Tag - positioned absolutely in top-right
                    WorkoutTypeTag {
                        id: sourceTag
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: workoutHistoryPage.modern ? 16 : 12
                        workoutSource: workoutModel ? workoutModel.getWorkoutSource(model.id) : "QZ"
                    }

                    // Action buttons - positioned absolutely in bottom-right
                    Row {
                        id: actionRow
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 12
                        // Modern look: 48 x 48 to touch, inside it a tonal pill 40 x 24 of the
                        // source colour, as the tag above (insets 4 and 12): the pill is 16 from the
                        // right edge of the card, under the tag, and 16 from its bottom; pills 8 apart
                        anchors.bottomMargin: workoutHistoryPage.modern ? 4 : 12
                        spacing: workoutHistoryPage.modern ? 0 : 8

                        // Peloton URL button
                        Button {
                            Accessible.name: qsTr("Open on Peloton")
                            width: workoutHistoryPage.modern ? 48 : 40
                            height: workoutHistoryPage.modern ? 48 : 45
                            visible: workoutModel && workoutModel.getWorkoutSource(model.id) === "PELOTON" && 
                                    workoutModel.getPelotonUrl(model.id) !== ""
                            // the modern pill 40 x 24 (Material Button default: 0 and 6)
                            topInset: workoutHistoryPage.modern ? 12 : 6
                            bottomInset: workoutHistoryPage.modern ? 12 : 6
                            leftInset: workoutHistoryPage.modern ? 4 : 0
                            rightInset: workoutHistoryPage.modern ? 4 : 0
                            
                            background: Rectangle {
                                color: workoutHistoryPage.modern ? (parent.pressed ? window.ui.alpha(sourceTag.inkColor, 0.28) : sourceTag.color)
                                             : (parent.pressed ? "#ff8855" : "#ff6b35")
                                radius: workoutHistoryPage.modern ? 12 : 6
                                border.color: "#cc5529"
                                border.width: workoutHistoryPage.modern ? 0 : 1
                            }
                            
                            contentItem: Item {
                                Text {
                                    visible: !workoutHistoryPage.modern && Qt.platform.os !== "android"
                                    text: "🌐"
                                    font.pixelSize: 16
                                    color: "white"
                                    anchors.centerIn: parent
                                }

                                ShapeIcon {
                                    visible: !workoutHistoryPage.modern && Qt.platform.os === "android"
                                    name: "language"
                                    size: 20
                                    color: "white"
                                    anchors.centerIn: parent
                                }

                                // the new look: "open outside" of the icon font on every platform
                                // (the classic draws an emoji off Android)
                                UiIcon {
                                    visible: workoutHistoryPage.modern
                                    name: "open_in_new"
                                    width: 16
                                    height: 16
                                    color: sourceTag.inkColor
                                    anchors.centerIn: parent
                                }
                            }
                            
                            onClicked: {
                                workoutModel.openPelotonUrl(model.id)
                            }
                        }
                        
                        // Training Program button
                        Button {
                            Accessible.name: qsTr("Load training program")
                            width: workoutHistoryPage.modern ? 48 : 40
                            height: workoutHistoryPage.modern ? 48 : 45
                            visible: workoutModel && workoutModel.hasTrainingProgram(model.id)
                            // the modern pill 40 x 24, as the Peloton button
                            topInset: workoutHistoryPage.modern ? 12 : 6
                            bottomInset: workoutHistoryPage.modern ? 12 : 6
                            leftInset: workoutHistoryPage.modern ? 4 : 0
                            rightInset: workoutHistoryPage.modern ? 4 : 0
                            
                            background: Rectangle {
                                color: workoutHistoryPage.modern ? (parent.pressed ? window.ui.alpha(sourceTag.inkColor, 0.28) : sourceTag.color)
                                             : (parent.pressed ? "#1976d2" : "#2196f3")
                                radius: workoutHistoryPage.modern ? 12 : 6
                                border.color: "#1565c0"
                                border.width: workoutHistoryPage.modern ? 0 : 1
                            }
                            
                            contentItem: Item {
                                Text {
                                    visible: !workoutHistoryPage.modern && Qt.platform.os !== "android"
                                    text: "📋"
                                    font.pixelSize: 16
                                    color: "white"
                                    anchors.centerIn: parent
                                }

                                ShapeIcon {
                                    visible: !workoutHistoryPage.modern && Qt.platform.os === "android"
                                    name: "list_alt"
                                    size: 20
                                    color: "white"
                                    anchors.centerIn: parent
                                }

                                UiIcon {
                                    visible: workoutHistoryPage.modern
                                    name: "list_alt"
                                    width: 16
                                    height: 16
                                    color: sourceTag.inkColor
                                    anchors.centerIn: parent
                                }
                            }
                            
                            onClicked: {
                                var success = workoutModel.loadTrainingProgram(model.id)
                                if (success) {
                                    trainingProgramDialog.title = qsTr("Success")
                                    trainingProgramDialog.message = qsTr("Training program loaded successfully!")
                                    trainingProgramDialog.isSuccess = true
                                } else {
                                    trainingProgramDialog.title = qsTr("Error")
                                    trainingProgramDialog.message = qsTr("Failed to load training program. Please check if the file exists.")
                                    trainingProgramDialog.isSuccess = false
                                }
                                if (workoutHistoryPage.modern)
                                    modernInfoDialog.open()
                                else
                                    trainingProgramDialog.open()
                            }
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        // Modern look: the icon 16 from the card edge, the lines centred in a card
                        // that leaves them 8 above and below at least. Centred, not on top as in other
                        // three-line items: a workout always has exactly three lines, so they line up
                        // from card to card either way, and centred looked better
                        anchors.leftMargin: workoutHistoryPage.modern ? 16 : 12
                        anchors.rightMargin: workoutHistoryPage.modern ? 16 : 12
                        anchors.topMargin: workoutHistoryPage.modern ? 8 : 12
                        anchors.bottomMargin: workoutHistoryPage.modern ? 8 : 12
                        spacing: 16

                        // Sport icon
                        Column {
                            Layout.alignment: Qt.AlignVCenter
                            Rectangle {
                                visible: workoutHistoryPage.modern
                                width: 40
                                height: 40
                                radius: 20
                                color: window.ui.surfaceHighest
                                UiIcon {
                                    anchors.centerIn: parent
                                    width: 22
                                    height: 22
                                    name: workoutHistoryPage.getSportIconName(sport)
                                    color: window.ui.accent
                                }
                            }
                            Text {
                                visible: !workoutHistoryPage.modern && Qt.platform.os !== "android"
                                text: getSportIcon(sport)
                                font.pixelSize: 32
                            }

                            // Android shows an emoji only with a downloaded font, so the icon is drawn
                            ShapeIcon {
                                visible: !workoutHistoryPage.modern && Qt.platform.os === "android"
                                name: getSportIconName(sport)
                                size: 32
                                color: "#444444"
                            }
                        }

                        // Workout info
                        ColumnLayout {
                            id: workoutInfo
                            Layout.fillWidth: true
                            spacing: 4

                            // Title row (without tag) with auto-scrolling
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.rightMargin: window.fontPx(80) // Reserve space for tag
                                Layout.preferredHeight: workoutHistoryPage.modern ? titleText.implicitHeight
                                    : window.textScale !== 1 ? Math.max(24, titleText.implicitHeight) : 24
                                clip: true
                                color: "transparent"
                                
                                Text {
                                    id: titleText
                                    text: title
                                    // Modern look: the first line of a list item (fontBody, DemiBold)
                                    font.weight: workoutHistoryPage.modern ? Font.DemiBold : Font.Bold
                                    font.pixelSize: workoutHistoryPage.modern ? window.ui.fontBody : window.fontPx(18)
                                    color: window.ui.ink("black")
                                    anchors.verticalCenter: parent.verticalCenter
                                    
                                    // Auto-scroll animation for long titles.
                                    // The row width settles only after the layout runs (the first
                                    // delegate passes through narrower widths), and an animation started
                                    // on one of them kept running: decide after the layout, again on every
                                    // width change, and put the title back when it stops mid-scroll.
                                    readonly property bool overflows: parent.width > 0 && contentWidth > parent.width
                                    function updateScroll() {
                                        if (overflows) {
                                            titleScroll.restart()
                                        } else {
                                            titleScroll.stop()
                                            x = 0
                                        }
                                    }
                                    onOverflowsChanged: Qt.callLater(updateScroll)
                                    onContentWidthChanged: if (overflows) Qt.callLater(updateScroll)
                                    Connections {
                                        target: titleText.parent
                                        function onWidthChanged() { if (titleText.overflows) Qt.callLater(titleText.updateScroll) }
                                    }

                                    SequentialAnimation {
                                        id: titleScroll
                                        loops: Animation.Infinite
                                        NumberAnimation {
                                            target: titleText
                                            property: "x"
                                            from: 0
                                            to: -(titleText.contentWidth - titleText.parent.width + 20)
                                            duration: Math.max(3000, titleText.contentWidth * 30)
                                        }
                                        PauseAnimation { duration: 1500 }
                                        NumberAnimation {
                                            target: titleText
                                            property: "x"
                                            from: -(titleText.contentWidth - titleText.parent.width + 20)
                                            to: 0
                                            duration: Math.max(3000, titleText.contentWidth * 30)
                                        }
                                        PauseAnimation { duration: 2000 }
                                    }
                                }
                            }

                            Text {
                                // a plain Text does not inherit the window font: the text size while it is not 100 %
                                // (modern look: the second line of a list item, fontCaption, scaled already)
                                Binding on font.pixelSize { when: workoutHistoryPage.modern || window.scaledFontPx > 0; value: workoutHistoryPage.modern ? window.ui.fontCaption : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                text: date
                                color: workoutHistoryPage.modern ? window.ui.textMuted : "#666666"
                            }

                            // Modern look: one stats line, the calories included; it stops short
                            // of the action buttons in the bottom-right corner when they are shown
                            Text {
                                font.pixelSize: window.ui.fontCaption
                                visible: workoutHistoryPage.modern
                                Layout.fillWidth: true
                                Layout.rightMargin: actionRow.width > 0 ? actionRow.width + 8 : 0
                                elide: Text.ElideRight
                                text: {
                                    var useMiles = settings && settings.miles_unit
                                    var displayDistance = useMiles ? (distance / 1.60934) : distance
                                    return duration + " · " + displayDistance.toFixed(2) + " " + (useMiles ? qsTr("mi") : qsTr("km"))
                                           + " · " + Math.round(calories) + " " + qsTr("kcal")
                                }
                                color: window.ui.textMuted
                            }

                            // Stats row; with a bigger text one under the other, clear of the action
                            // buttons in the bottom-right corner
                            GridLayout {
                                visible: !workoutHistoryPage.modern
                                columns: window.textScale > 1 ? 1 : 2
                                columnSpacing: 16
                                rowSpacing: 4

                                Text {
                                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                    text: wrapEmoji("⏱ ") + duration
                                    textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                                }

                                Text {
                                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                    text: {
                                        var useMiles = settings && settings.miles_unit
                                        var displayDistance = useMiles ? (distance / 1.60934) : distance
                                        return wrapEmoji("📏 ") + displayDistance.toFixed(2) + " " + (useMiles ? qsTr("mi") : qsTr("km"))
                                    }
                                    textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                                }
                            }

                            RowLayout {
                                visible: !workoutHistoryPage.modern
                                spacing: 16

                                Text {
                                    Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                                    text: Qt.platform.os === "android" ? 
                                          wrapEmoji("🔥 ") + Math.round(calories) + " " + qsTr("kcal") :
                                          "🔥 " + Math.round(calories) + " " + qsTr("kcal")
                                    textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                                }
                            }
                        }
                        
                    }
                }

                onClicked: {
                    console.log("Workout clicked, ID:", model.id)
                    
                    // Get workout details from the model
                    var details = workoutModel.getWorkoutDetails(model.id)
                    console.log("Workout details:", JSON.stringify(details))

                    // Emit signal with file URL for chart preview - same pattern as profiles.qml
                    console.log("Emitting fitfile_preview_clicked with path:", details.filePath)
                    // Convert to URL like profiles.qml does with FolderListModel
                    var fileUrl = "file://" + details.filePath
                    console.log("Converted to URL:", fileUrl)
                    workoutHistoryPage.fitfile_preview_clicked(fileUrl)

                    // Push the ChartJsTest view
                    // the file goes along for the Share button of the toolbar
                    stackView.push("PreviewChart.qml", { fitFilePath: details.filePath })
                }

                onPressAndHold: {
                    workoutHistoryPage.openUploadMenu(swipeDelegate, model.id, model.title)
                }
            }
        }
    }

    Menu {
        // A MenuItem has no press action for a screen reader in Qt 5 (its role is reset to
        // MenuItem): it is read, but a double tap does nothing without the handler
        // The name is set as well: the one Qt takes from the text was empty for TalkBack on a phone
        id: importMenu
        // Modal, as the calendar: otherwise a screen reader touching an item got the workout
        // row under the menu; no dim, so it looks as before
        modal: true
        dim: false

        MenuItem {
            Accessible.onPressAction: triggered()
            Accessible.name: text
            text: qsTr("Import FIT File...")
            onTriggered: {
                if (Qt.platform.os === "android") {
                    rootItem.openAndroidDocumentPicker("fit")
                } else {
                    fitFileDialogLoader.active = true
                }
            }
        }

        MenuItem {
            Accessible.onPressAction: triggered()
            Accessible.name: text
            // after a reinstall the workouts of the previous install stay in this folder,
            // but Android hides them from the app until the folder is picked once
            text: qsTr("Import from QZ Folder...")
            onTriggered: rootItem.importFitFolder()
        }
    }

    // Modern look: the same two items in a Material 3 menu under the button (UiMenuBackground,
    // as the UiComboBox list): as wide as the longest item, 112 to 280 with the padding of 8,
    // the pressed item a rounded block inset from the edges. It was 240 wide, and the longer
    // translations ("Імпортувати з папки QZ...") filled it to the edge
    UiPopup {
        id: modernImportMenu
        padding: 8
        // Modal without a dim, as the classic menu: otherwise a screen reader touching an
        // item got the workout row under the menu
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        readonly property var items: [
            { label: qsTr("Import FIT File..."), folder: false },
            { label: qsTr("Import from QZ Folder..."), folder: true }
        ]
        // Item width: the longest label with 8 on both sides (16 from the menu edge, as in the
        // UiComboBox list), within the 112..280 of a menu less the padding, and never past the
        // page. Measured when the menu opens (as the UiComboBox list), not in a binding: the
        // binding would change the text it reads
        property real itemWidth: 240
        // Given, not from the content: the Column takes its new width only at the next polish,
        // and openUnder() placed the menu by the old one
        width: itemWidth + leftPadding + rightPadding
        TextMetrics {
            id: importMenuMetrics
            font.pixelSize: window.ui.fontBody
        }
        function measureItems() {
            var widest = 0
            for (var i = 0; i < items.length; ++i) {
                importMenuMetrics.text = items[i].label
                widest = Math.max(widest, importMenuMetrics.advanceWidth)
            }
            itemWidth = Math.min(Math.max(Math.ceil(widest) + 16, 112 - 16), 280 - 16,
                                 workoutHistoryPage.width - 32)
        }

        background: UiMenuBackground { }

        function openUnder(item) {
            measureItems()
            var p = item.mapToItem(workoutHistoryPage, 0, item.height)
            x = Math.max(8, Math.min(p.x + item.width - width, workoutHistoryPage.width - width - 8))
            y = p.y + 4
            open()
        }

        contentItem: Column {
            width: modernImportMenu.itemWidth
            Repeater {
                model: modernImportMenu.items
                delegate: AbstractButton {
                    id: modernImportItem
                    width: modernImportMenu.itemWidth
                    height: 48
                    Accessible.role: Accessible.MenuItem
                    Accessible.name: modelData.label
                    Accessible.onPressAction: clicked()
                    // Pressed: the state layer of the text colour (Material 3, 10 %)
                    background: Rectangle {
                        radius: 12
                        color: modernImportItem.down ? window.ui.alpha(window.ui.textMain, 0.10) : "transparent"
                    }
                    contentItem: Text {
                        leftPadding: 8
                        rightPadding: 8
                        text: modelData.label
                        font.pixelSize: window.ui.fontBody
                        color: window.ui.textMain
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        Accessible.ignored: true
                    }
                    onClicked: {
                        modernImportMenu.close()
                        if (modelData.folder)
                            rootItem.importFitFolder()
                        else if (Qt.platform.os === "android")
                            rootItem.openAndroidDocumentPicker("fit")
                        else
                            fitFileDialogLoader.active = true
                    }
                }
            }
        }
    }

    // Modern look: the long press menu of a workout on a UiPopup card: the workout title and
    // date on top, then Share and the upload services that are set up, each with its icon.
    // Only the shown items are in the list, so there are no empty rows. The card opens under
    // the pressed workout (above it when there is no room below), the workout is tinted and
    // stays light in a hole of the scrim, so it is clear which workout the actions are for.
    // Modal: a screen reader stays inside the card
    UiPopup {
        id: modernUploadMenu
        padding: 8
        modal: true
        dim: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        readonly property int itemWidth: Math.min(296, workoutHistoryPage.width - 32)
        property var items: []
        // the delegate of the pressed workout and its card in the coordinates of the window
        property Item row: null
        property rect hole: Qt.rect(0, 0, 0, 0)
        property string subtitle: ""

        onClosed: row = null

        // White as the workout cards: a grey card on the scrim reads as a disabled block
        background: UiFrame {
            radius: 20
            fill: window.ui.surface
            stroke: window.ui.outline
            strokeWidth: window.ui.dark ? 0 : 1
        }

        // The scrim of the page with a hole for the pressed workout: four bands around the hole
        // and a rounded piece in each of its corners. Not one rounded rectangle with a border far
        // wider than the window: such a border left most of the header above the hole undimmed
        // (Android emulator, API 30 and 35)
        Overlay.modal: Item {
            id: holeScrim
            readonly property rect h: modernUploadMenu.hole
            // the radius of the workout card, smaller when the visible part of the card is low
            readonly property real r: Math.max(0, Math.min(16, h.width / 2, h.height / 2))

            Rectangle { x: 0; y: 0; width: parent.width; height: holeScrim.h.y; color: window.ui.scrim }
            Rectangle {
                x: 0; y: holeScrim.h.y + holeScrim.h.height
                width: parent.width; height: Math.max(0, parent.height - y)
                color: window.ui.scrim
            }
            Rectangle { x: 0; y: holeScrim.h.y; width: holeScrim.h.x; height: holeScrim.h.height; color: window.ui.scrim }
            Rectangle {
                x: holeScrim.h.x + holeScrim.h.width; y: holeScrim.h.y
                width: Math.max(0, parent.width - x); height: holeScrim.h.height
                color: window.ui.scrim
            }

            // A corner: an r x r square showing the border of a rectangle whose inner edge is
            // rounded with the radius r at that corner
            Repeater {
                model: [[0, 0], [1, 0], [0, 1], [1, 1]]
                Item {
                    x: holeScrim.h.x + modelData[0] * (holeScrim.h.width - holeScrim.r)
                    y: holeScrim.h.y + modelData[1] * (holeScrim.h.height - holeScrim.r)
                    width: holeScrim.r
                    height: holeScrim.r
                    clip: true
                    Rectangle {
                        x: modelData[0] ? -2 * holeScrim.r : -holeScrim.r
                        y: modelData[1] ? -2 * holeScrim.r : -holeScrim.r
                        width: 4 * holeScrim.r
                        height: 4 * holeScrim.r
                        radius: 2 * holeScrim.r
                        color: "transparent"
                        border.width: holeScrim.r
                        border.color: window.ui.scrim
                    }
                }
            }
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        // Under the card of the pressed workout with a gap of 8, above it when the card does
        // not fit below; when it fits on neither side (a short page in landscape), pinned to
        // the edge of the side with more room. Across: centred on the row, kept inside the page
        function openFor(delegateItem, centerX) {
            var card = delegateItem.cardItem
            var onPage = card.mapToItem(workoutHistoryPage, 0, 0)
            // the hole is the part of the workout card the list shows: a row scrolled half
            // under the header does not light up the header above the list
            var inWindow = card.mapToItem(null, 0, 0)
            var list = workoutListView.mapToItem(null, 0, 0)
            var top = Math.max(inWindow.y, list.y)
            var bottom = Math.min(inWindow.y + card.height, list.y + workoutListView.height)
            hole = Qt.rect(inWindow.x, top, card.width, Math.max(0, bottom - top))
            subtitle = delegateItem.workoutDate
            row = delegateItem
            buildItems()
            var pageH = workoutHistoryPage.height
            x = Math.max(8, Math.min(centerX - width / 2, workoutHistoryPage.width - width - 8))
            var below = onPage.y + card.height + 8
            var above = onPage.y - 8 - height
            if (below + height <= pageH - 8)
                y = below
            else if (above >= 8)
                y = above
            else
                y = pageH - 8 - below >= onPage.y - 16 ? Math.max(8, pageH - height - 8) : 8
            open()
        }

        function buildItems() {
            var list = [{ label: qsTr("Share FIT file"), icon: "share", action: "share" }]
            if (rootItem && rootItem.isStravaLoggedIn())
                list.push({ label: qsTr("Upload to Strava"), icon: "upload_file", action: "strava" })
            if (rootItem && rootItem.isGarminUploadConfigured())
                list.push({ label: qsTr("Upload to Garmin"), icon: "upload_file", action: "garmin" })
            if (rootItem && rootItem.isIntervalsICUUploadConfigured())
                list.push({ label: qsTr("Upload to Intervals.icu"), icon: "upload_file", action: "intervals" })
            if (Qt.platform.os === "ios" && workoutModel && workoutModel.canWriteAppleHealth(uploadMenu.workoutId))
                list.push({ label: qsTr("Upload to Apple Health"), icon: "upload_file", action: "health" })
            items = list
            // the Column lays out the new rows only on the next frame: do it now,
            // so the placement uses the height of this list, not the previous one
            contentItem.forceLayout()
        }

        function run(action) {
            close()
            if (action === "share")
                rootItem.shareWorkoutFile(uploadMenu.filePath)
            else if (action === "strava")
                rootItem.uploadHistoricalWorkoutToStrava(uploadMenu.filePath)
            else if (action === "garmin")
                rootItem.uploadHistoricalWorkoutToGarmin(uploadMenu.filePath)
            else if (action === "intervals")
                rootItem.uploadHistoricalWorkoutToIntervalsICU(uploadMenu.filePath)
            else if (action === "health")
                workoutModel.uploadWorkoutToAppleHealth(uploadMenu.workoutId)
        }

        contentItem: Column {
            width: modernUploadMenu.itemWidth

            // which workout the actions are for
            Text {
                width: parent.width
                leftPadding: 12
                rightPadding: 12
                topPadding: 6
                bottomPadding: 8
                // the date tells apart the many workouts of the same name ("1 minutes Run")
                text: modernUploadMenu.subtitle !== "" ? uploadMenu.workoutTitle + "  ·  " + modernUploadMenu.subtitle
                                                       : uploadMenu.workoutTitle
                font.pixelSize: window.ui.fontLabel
                font.weight: Font.DemiBold
                color: window.ui.textMuted
                elide: Text.ElideRight
                Accessible.role: Accessible.Heading
                Accessible.name: text
            }

            Repeater {
                model: modernUploadMenu.items
                delegate: AbstractButton {
                    id: modernUploadItem
                    width: modernUploadMenu.itemWidth
                    height: 48
                    Accessible.role: Accessible.MenuItem
                    Accessible.name: modelData.label
                    Accessible.onPressAction: modernUploadMenu.run(modelData.action)
                    background: Rectangle {
                        // a menu item pressed: radius 12, the state layer of the text colour (10 %)
                        radius: 12
                        color: modernUploadItem.down ? window.ui.alpha(window.ui.textMain, 0.10) : "transparent"
                    }
                    contentItem: Item {
                        // the share icon is not in the icon font: drawn as the one of the toolbar,
                        // at the pixel density of the screen and scaled down into a 20 x 20 box
                        Item {
                            visible: modelData.icon === "share"
                            x: 12
                            anchors.verticalCenter: parent.verticalCenter
                            width: 20
                            height: 20
                            Canvas {
                                property color color: window.ui.textMain
                                anchors.centerIn: parent
                                width: 20 * Screen.devicePixelRatio
                                height: 20 * Screen.devicePixelRatio
                                scale: 1 / Screen.devicePixelRatio
                                onColorChanged: requestPaint()
                                onPaint: {
                                    var ctx = getContext("2d")
                                    ctx.reset()
                                    ctx.scale(width / 24, height / 24)
                                    ctx.strokeStyle = color
                                    ctx.fillStyle = color
                                    ctx.lineWidth = 1.6
                                    ctx.beginPath()
                                    ctx.moveTo(18, 5)
                                    ctx.lineTo(6, 12)
                                    ctx.lineTo(18, 19)
                                    ctx.stroke()
                                    var dots = [[18, 5], [6, 12], [18, 19]]
                                    for (var i = 0; i < dots.length; i++) {
                                        ctx.beginPath()
                                        ctx.arc(dots[i][0], dots[i][1], 3, 0, 2 * Math.PI)
                                        ctx.fill()
                                    }
                                }
                            }
                        }
                        UiIcon {
                            visible: modelData.icon !== "share"
                            x: 12
                            anchors.verticalCenter: parent.verticalCenter
                            width: 20
                            height: 20
                            name: modelData.icon
                            color: window.ui.textMain
                        }
                        Text {
                            x: 44
                            width: parent.width - x - 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.label
                            font.pixelSize: window.ui.fontBody
                            color: window.ui.textMain
                            elide: Text.ElideRight
                            Accessible.ignored: true
                        }
                    }
                    onClicked: modernUploadMenu.run(modelData.action)
                }
            }
        }
    }

    Loader {
        id: fitFileDialogLoader
        active: false
        sourceComponent: Component {
            Dialogs.FileDialog {
                title: qsTr("Please choose a file")
                folder: shortcuts.home
                nameFilters: [qsTr("FIT files (*.fit *.FIT)"), qsTr("All files (*)")]
                visible: true
                onAccepted: {
                    rootItem.importFitFile(fileUrl)
                    fitFileDialogLoader.active = false
                }
                onRejected: fitFileDialogLoader.active = false
            }
        }
    }

    Menu {
        // A MenuItem has no press action for a screen reader in Qt 5 (its role is reset to
        // MenuItem): it is read, but a double tap does nothing without the handler
        // The name is set as well: the one Qt takes from the text was empty for TalkBack on a phone
        id: uploadMenu
        // Modal, as the calendar: otherwise a screen reader touching an item got the workout
        // row under the menu; no dim, so it looks as before
        modal: true
        dim: false

        property int workoutId: -1
        property string workoutTitle: ""
        property string filePath: ""

        title: workoutTitle

        MenuItem {
            Accessible.onPressAction: triggered()
            Accessible.name: text
            text: qsTr("Share FIT file")
            onTriggered: rootItem.shareWorkoutFile(uploadMenu.filePath)
        }

        // The menu opens without any service as well: a hidden item has no height, or the list
        // of the menu keeps an empty row for it
        MenuItem {
            Accessible.onPressAction: triggered()
            Accessible.name: text
            text: qsTr("Upload to Strava")
            visible: rootItem && rootItem.isStravaLoggedIn()
            height: visible ? implicitHeight : 0
            onTriggered: rootItem.uploadHistoricalWorkoutToStrava(uploadMenu.filePath)
        }

        MenuItem {
            Accessible.onPressAction: triggered()
            Accessible.name: text
            text: qsTr("Upload to Garmin")
            visible: rootItem && rootItem.isGarminUploadConfigured()
            height: visible ? implicitHeight : 0
            onTriggered: rootItem.uploadHistoricalWorkoutToGarmin(uploadMenu.filePath)
        }

        MenuItem {
            Accessible.onPressAction: triggered()
            Accessible.name: text
            text: qsTr("Upload to Intervals.icu")
            visible: rootItem && rootItem.isIntervalsICUUploadConfigured()
            height: visible ? implicitHeight : 0
            onTriggered: rootItem.uploadHistoricalWorkoutToIntervalsICU(uploadMenu.filePath)
        }

        MenuItem {
            Accessible.onPressAction: triggered()
            Accessible.name: text
            text: qsTr("Upload to Apple Health")
            visible: Qt.platform.os === "ios" &&
                     workoutModel &&
                     workoutModel.canWriteAppleHealth(uploadMenu.workoutId)
            height: visible ? implicitHeight : 0
            onTriggered: workoutModel.uploadWorkoutToAppleHealth(uploadMenu.workoutId)
        }
    }

    // Confirmation Dialog
    Dialog {
        id: confirmDialog

        property int workoutId
        property string workoutTitle

        title: qsTr("Delete Workout")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        // a bigger text: the dialog keeps inside the screen and the question wraps
        width: window.textScale !== 1 ? Math.min(implicitWidth, parent.width - 32) : implicitWidth

        Text {
            Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
            width: window.textScale !== 1 ? confirmDialog.availableWidth : implicitWidth
            wrapMode: window.labelWrap
            text: qsTr("Are you sure you want to delete '%1'?").arg(confirmDialog.workoutTitle)
        }

        onAccepted: {
            workoutModel.deleteWorkout(confirmDialog.workoutId)
            swipeDelegate.swipe.close()
        }
        onRejected: {
            swipeDelegate.swipe.close()
        }
    }

    // Modern look: the same questions in the card dialog of the other screens
    UiMessageDialog {
        id: modernDeleteDialog
        property int workoutId
        property string workoutTitle
        property var swipeItem: null
        title: qsTr("Delete Workout")
        text: qsTr("Are you sure you want to delete '%1'?").arg(workoutTitle)
        // the positive button named after the action and in the danger colour, as in the
        // other questions that cannot be undone (Stop on Home, Reset in the settings)
        buttons: P.MessageDialog.Yes | P.MessageDialog.Cancel
        yesText: qsTr("Delete")
        destructive: true
        onAccepted: workoutModel.deleteWorkout(workoutId)
        // any way out (Cancel, the back key, a deleted row) closes the swiped row again
        onClosed: {
            if (swipeItem)
                swipeItem.swipe.close()
            swipeItem = null
        }
    }

    UiMessageDialog {
        id: modernInfoDialog
        title: trainingProgramDialog.title
        text: trainingProgramDialog.message
        buttons: P.MessageDialog.Ok
    }

    // Training Program Loading Dialog
    Dialog {
        id: trainingProgramDialog

        property string message: ""
        property bool isSuccess: true

        modal: true
        standardButtons: Dialog.Ok

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        background: Rectangle {
            color: "white"
            radius: 8
            border.color: trainingProgramDialog.isSuccess ? "#4caf50" : "#f44336"
            border.width: 2
        }

        header: Rectangle {
            height: 50
            color: trainingProgramDialog.isSuccess ? "#4caf50" : "#f44336"
            radius: 8

            Text {
                anchors.centerIn: parent
                text: trainingProgramDialog.title
                color: "white"
                font.pixelSize: window.fontPx(18)
                font.bold: true
            }
        }

        contentItem: ColumnLayout {
            spacing: 16

            Text {
                Layout.margins: 20
                Layout.preferredWidth: 300
                Layout.preferredHeight: window.textScale !== 1 ? Math.max(120, implicitHeight) : 120
                text: Qt.platform.os === "android" ? 
                      wrapEmoji("🔥 ") +
                      wrapEmoji(trainingProgramDialog.isSuccess ? '✅ ' : '❌ ') +
                      trainingProgramDialog.message :
                      "🔥 " + (trainingProgramDialog.isSuccess ? '✅ ' : '❌ ') + trainingProgramDialog.message
                textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: window.fontPx(14)
            }
        }
    }

    // Streak Banner at the bottom
    Rectangle {
        id: streakBanner
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: window.textScale !== 1 ? Math.max(80, streakColumn.implicitHeight + 16) : 80
        visible: workoutModel && !workoutHistoryPage.modern
        
        Behavior on visible {
            NumberAnimation {
                properties: "opacity"
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }
        
        // Special pulsing effect for major milestones
        SequentialAnimation on opacity {
            running: workoutModel && workoutModel.currentStreak >= 30
            loops: Animation.Infinite
            NumberAnimation { from: 0.9; to: 1.0; duration: 1500; easing.type: Easing.InOutSine }
            NumberAnimation { from: 1.0; to: 0.9; duration: 1500; easing.type: Easing.InOutSine }
        }
        
        gradient: Gradient {
            GradientStop { 
                position: 0.0; 
                color: workoutModel && (workoutModel.currentStreak >= 365) ? "#FFD700" : 
                       workoutModel && (workoutModel.currentStreak >= 180) ? "#9932CC" : 
                       workoutModel && (workoutModel.currentStreak >= 90) ? "#FF1493" : 
                       workoutModel && (workoutModel.currentStreak >= 30) ? "#FF4500" : 
                       workoutModel && (workoutModel.currentStreak >= 7) ? "#FF6347" : "#FF6B35"
            }
            GradientStop { 
                position: 1.0; 
                color: workoutModel && (workoutModel.currentStreak >= 365) ? "#FFA500" : 
                       workoutModel && (workoutModel.currentStreak >= 180) ? "#8A2BE2" : 
                       workoutModel && (workoutModel.currentStreak >= 90) ? "#DC143C" : 
                       workoutModel && (workoutModel.currentStreak >= 30) ? "#FF6B35" : 
                       workoutModel && (workoutModel.currentStreak >= 7) ? "#FF4500" : "#F7931E"
            }
        }
        
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#40FFFFFF" }
                GradientStop { position: 1.0; color: "#00FFFFFF" }
            }
        }
        
        ColumnLayout {
            id: streakColumn
            anchors.centerIn: parent
            spacing: 4

            // Current streak with count
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 15

                // Fire emoji with animation
                Text {
                    text: Qt.platform.os === "android" ? (
                          workoutModel && workoutModel.currentStreak >= 365 ? wrapEmoji("👑🔥") :
                          workoutModel && workoutModel.currentStreak >= 180 ? wrapEmoji("🎖️🔥") :
                          workoutModel && workoutModel.currentStreak >= 90 ? wrapEmoji("🦁🔥") :
                          workoutModel && workoutModel.currentStreak >= 30 ? wrapEmoji("🎊🔥") :
                          workoutModel && workoutModel.currentStreak >= 7 ? wrapEmoji("🏆🔥") : wrapEmoji("🔥")
                          ) : (
                          workoutModel && workoutModel.currentStreak >= 365 ? "👑🔥" :
                          workoutModel && workoutModel.currentStreak >= 180 ? "🎖️🔥" :
                          workoutModel && workoutModel.currentStreak >= 90 ? "🦁🔥" :
                          workoutModel && workoutModel.currentStreak >= 30 ? "🎊🔥" :
                          workoutModel && workoutModel.currentStreak >= 7 ? "🏆🔥" : "🔥"
                          )
                    textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                    font.pixelSize: workoutModel && workoutModel.currentStreak >= 7 ? 28 : 24

                    SequentialAnimation on scale {
                        running: workoutModel && workoutModel.currentStreak > 0
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: 1.0;
                            to: workoutModel && workoutModel.currentStreak >= 7 ? 1.4 : 1.2;
                            duration: workoutModel && workoutModel.currentStreak >= 365 ? 600 : 800;
                            easing.type: Easing.InOutSine
                        }
                        NumberAnimation {
                            from: workoutModel && workoutModel.currentStreak >= 7 ? 1.4 : 1.2;
                            to: 1.0;
                            duration: workoutModel && workoutModel.currentStreak >= 7 ? 600 : 800;
                            easing.type: Easing.InOutSine
                        }
                    }

                    // Special sparkle effect for year achievement
                    SequentialAnimation on rotation {
                        running: workoutModel && workoutModel.currentStreak >= 7
                        loops: Animation.Infinite
                        NumberAnimation { from: 0; to: 360; duration: 3000; easing.type: Easing.Linear }
                    }
                }

                // Current streak count
                Text {
                    text: workoutModel ? (workoutModel.currentStreak !== 1 ? qsTr("%1 days streak") : qsTr("%1 day streak")).arg(workoutModel.currentStreak) : ""
                    font.pixelSize: window.fontPx(18)
                    font.bold: true
                    color: "white"
                    visible: workoutModel
                }

                // Another fire emoji
                Text {
                    text: Qt.platform.os === "android" ? (
                          workoutModel && workoutModel.currentStreak >= 365 ? wrapEmoji("🔥👑") :
                          workoutModel && workoutModel.currentStreak >= 180 ? wrapEmoji("🔥🎖️") :
                          workoutModel && workoutModel.currentStreak >= 90 ? wrapEmoji("🔥🦁") :
                          workoutModel && workoutModel.currentStreak >= 30 ? wrapEmoji("🔥🎊") :
                          workoutModel && workoutModel.currentStreak >= 7 ? wrapEmoji("🔥🏆") : wrapEmoji("🔥")
                          ) : (
                          workoutModel && workoutModel.currentStreak >= 365 ? "🔥👑" :
                          workoutModel && workoutModel.currentStreak >= 180 ? "🔥🎖️" :
                          workoutModel && workoutModel.currentStreak >= 90 ? "🔥🦁" :
                          workoutModel && workoutModel.currentStreak >= 30 ? "🔥🎊" :
                          workoutModel && workoutModel.currentStreak >= 7 ? "🔥🏆" : "🔥"
                          )
                    textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                    font.pixelSize: workoutModel && workoutModel.currentStreak >= 365 ? 28 : 24

                    SequentialAnimation on scale {
                        running: workoutModel && workoutModel.currentStreak > 0
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: 1.0;
                            to: workoutModel && workoutModel.currentStreak >= 7 ? 1.4 : 1.2;
                            duration: workoutModel && workoutModel.currentStreak >= 7 ? 700 : 1000;
                            easing.type: Easing.InOutSine
                        }
                        NumberAnimation {
                            from: workoutModel && workoutModel.currentStreak >= 7 ? 1.4 : 1.2;
                            to: 1.0;
                            duration: workoutModel && workoutModel.currentStreak >= 7 ? 700 : 1000;
                            easing.type: Easing.InOutSine
                        }
                    }

                    // Counter-rotation for variety
                    SequentialAnimation on rotation {
                        running: workoutModel && workoutModel.currentStreak >= 7
                        loops: Animation.Infinite
                        NumberAnimation { from: 0; to: -360; duration: 3500; easing.type: Easing.Linear }
                    }
                }
            }
            
            // Motivational message
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: workoutModel ? workoutModel.streakMessage : ""
                font.pixelSize: window.fontPx(14)
                font.italic: true
                color: "white"
                visible: workoutModel && workoutModel.streakMessage !== ""
                opacity: 0.9
            }
            
            // Best streak (smaller text)
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: workoutModel ? (workoutModel.longestStreak !== 1 ? qsTr("Personal best: %1 days") : qsTr("Personal best: %1 day")).arg(workoutModel.longestStreak) : ""
                font.pixelSize: window.fontPx(12)
                color: "white"
                visible: workoutModel && workoutModel.longestStreak > workoutModel.currentStreak && workoutModel.longestStreak > 0
                opacity: 0.7
            }
        }
        
        // Subtle shadow effect at the top
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#40000000" }
                GradientStop { position: 1.0; color: "#00000000" }
            }
        }
    }

    // Calendar Popup
    UiPopup {
        id: calendarPopup
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width * 0.9, 400)
        height: Math.min(parent.height * 0.8, 500)
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        
        onOpened: {
            // Refresh workout dates when calendar opens
            if (workoutModel) {
                calendar.workoutDates = workoutModel.getWorkoutDates()
                console.log("Calendar opened, refreshed workout dates:", JSON.stringify(calendar.workoutDates))
            }
        }

        background: Rectangle {
            color: workoutHistoryPage.modern ? window.ui.surfaceHigh : "white"
            radius: workoutHistoryPage.modern ? 28 : 12
            border.color: "#d0d0d0"
            border.width: workoutHistoryPage.modern ? 0 : 1

            // Shadow effect
            Rectangle {
                visible: !workoutHistoryPage.modern
                anchors.fill: parent
                anchors.topMargin: 2
                anchors.leftMargin: 2
                radius: parent.radius
                color: "#40000000"
                z: -1
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            // Calendar Header
            RowLayout {
                Layout.fillWidth: true
                
                UiButton {
                    text: workoutHistoryPage.modern ? "" : "<"
                    flat: workoutHistoryPage.modern
                    implicitWidth: workoutHistoryPage.modern ? 48 : Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
                    Accessible.name: qsTr("Previous month")
                    onClicked: calendar.selectedDate = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth() - 1, 1)
                    // Modern: an accent chevron on a tonal circle, like the move buttons of the
                    // workout editor; the bare thin chevron was hard to see
                    Rectangle {
                        anchors.centerIn: parent
                        visible: workoutHistoryPage.modern
                        width: 44
                        height: 44
                        radius: 22
                        color: window.ui.alpha(window.ui.accent, parent.pressed ? 0.28 : 0.16)
                    }
                    UiIcon {
                        anchors.centerIn: parent
                        visible: workoutHistoryPage.modern
                        width: 30
                        height: 30
                        name: "chevron_left"
                        color: window.ui.accent
                    }
                }
                
                Text {
                    Layout.fillWidth: true
                    // Modern: the standalone month name of the locale (nominative, capitalised)
                    text: workoutHistoryPage.modern
                          ? workoutHistoryPage.monthTitle(calendar.selectedDate)
                          : calendar.selectedDate.toLocaleDateString(Qt.locale(), "MMMM yyyy")
                    font.pixelSize: workoutHistoryPage.modern ? window.ui.fontTitle : window.fontPx(18)
                    // Modern look: with a bigger text size a long month may not fit between the
                    // buttons; it then shrinks to the width, down to the caption size. The item
                    // keeps the height of the full size and Text puts the line at its top, so the
                    // shrunk month is centred in it to stay level with the buttons
                    fontSizeMode: workoutHistoryPage.modern ? Text.HorizontalFit : Text.FixedSize
                    minimumPixelSize: window.ui.fontCaption
                    verticalAlignment: workoutHistoryPage.modern ? Text.AlignVCenter : Text.AlignTop
                    font.bold: true
                    color: window.ui.ink("black")
                    horizontalAlignment: Text.AlignHCenter
                }
                
                UiButton {
                    text: workoutHistoryPage.modern ? "" : ">"
                    flat: workoutHistoryPage.modern
                    implicitWidth: workoutHistoryPage.modern ? 48 : Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
                    Accessible.name: qsTr("Next month")
                    onClicked: calendar.selectedDate = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth() + 1, 1)
                    // Modern: an accent chevron on a tonal circle, like the move buttons of the
                    // workout editor; the bare thin chevron was hard to see
                    Rectangle {
                        anchors.centerIn: parent
                        visible: workoutHistoryPage.modern
                        width: 44
                        height: 44
                        radius: 22
                        color: window.ui.alpha(window.ui.accent, parent.pressed ? 0.28 : 0.16)
                    }
                    UiIcon {
                        anchors.centerIn: parent
                        visible: workoutHistoryPage.modern
                        width: 30
                        height: 30
                        name: "chevron_right"
                        color: window.ui.accent
                    }
                }
            }

            // Calendar Grid
            GridLayout {
                id: calendar
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 7
                
                // Modern look: one size for the round days, from the mean column width. The
                // layout snaps the column edges to whole pixels (QGridLayoutEngine with
                // snapToPixelGrid), so of seven equal columns some are 1 dp narrower, and a circle
                // sized by its own cell came out 1 dp smaller there. floor(mean) - 2 still fits
                // the narrowest cell, which is at most 1 dp below the mean
                readonly property real dayCircle: Math.min(40, Math.floor((width - 6 * columnSpacing) / 7) - 2)

                // Modern look: one size for the whole row of day names. With a bigger text size
                // the widest name may not fit its column ("Wed" at 170 %); then the row shrinks
                // toward its width, but not below the caption size (at 170 % on a 411 dp wide
                // screen the row stops there, the names still apart). Shrinking each name to its own
                // column left "Fri" and "Sat" bigger than the rest
                FontMetrics {
                    id: dayNameMetrics
                    font.pixelSize: window.ui.fontBody
                }
                readonly property int dayNamePx: {
                    // the size of the metrics, not ui.fontBody: so this is computed again after
                    // the metrics took a new text size, not before
                    var full = dayNameMetrics.font.pixelSize
                    var room = Math.floor((width - 6 * columnSpacing) / 7) - 2
                    var names = workoutHistoryPage.weekDayNames()
                    var widest = 0
                    for (var i = 0; i < names.length; i++)
                        widest = Math.max(widest, dayNameMetrics.advanceWidth(names[i]))
                    if (room <= 0 || widest <= room)
                        return full
                    return Math.max(window.ui.fontCaption, Math.floor(full * room / widest))
                }

                property date selectedDate: new Date()
                property var workoutDates: workoutModel ? workoutModel.getWorkoutDates() : []
                
                // Debug: print workout dates when they change
                onWorkoutDatesChanged: {
                    console.log("Calendar workout dates updated:", JSON.stringify(workoutDates))
                }
                
                // Day headers
                Repeater {
                    // Modern: the short day names of the locale, from its first day of the week
                    model: workoutHistoryPage.modern ? workoutHistoryPage.weekDayNames()
                         : [qsTr("Sun"), qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr("Thu"), qsTr("Fri"), qsTr("Sat")]
                    Text {
                        // Modern look: the day names of the Material 3 date picker, Body Large regular
                        // (fontBody, as the days below); classic: the app font, bold
                        // (dayNamePx: fontBody, smaller only when the widest name does not fit)
                        Binding on font.pixelSize { when: workoutHistoryPage.modern || window.scaledFontPx > 0; value: workoutHistoryPage.modern ? calendar.dayNamePx : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.fillWidth: true
                        // Modern look: seven equal columns. A GridLayout grows each column from its
                        // preferred width (the width of the day name), so "Fri" and "Sat" stayed narrower
                        // with smaller circles; the same preferred width for every cell makes them equal
                        // (Qt 6.6 has GridLayout.uniformCellWidths for this, Qt 5.15 does not). -1: the
                        // default, the classic look as before
                        Layout.preferredWidth: workoutHistoryPage.modern ? 1 : -1
                        Layout.preferredHeight: 30
                        text: modelData
                        font.bold: !workoutHistoryPage.modern
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: workoutHistoryPage.modern ? window.ui.textMuted : "#666666"
                    }
                }
                
                // Calendar days
                Repeater {
                    model: getCalendarDays()
                    
                    Rectangle {
                        Layout.fillWidth: true
                        // the equal columns of the modern look (the day names above)
                        Layout.preferredWidth: workoutHistoryPage.modern ? 1 : -1
                        Layout.fillHeight: true
                        Layout.minimumHeight: 40
                        
                        property date dayDate: modelData.date
                        property bool isCurrentMonth: modelData.currentMonth
                        property bool hasWorkout: modelData.hasWorkout
                        property bool isToday: dayDate.toDateString() === new Date().toDateString()

                        // A day is a rectangle with a MouseArea: a screen reader sees it only as a button
                        // with a name and a press action; days of the other months do nothing on a tap
                        Accessible.role: Accessible.Button
                        Accessible.name: dayDate.toLocaleDateString(Qt.locale(), Locale.LongFormat) + (hasWorkout ? ", " + qsTr("has a workout") : "")
                        Accessible.ignored: !isCurrentMonth
                        Accessible.onPressAction: selectDay()

                        function selectDay() {
                            if (isCurrentMonth) {
                                var year = dayDate.getFullYear();
                                var month = dayDate.getMonth() + 1; // i mesi JS sono 0-indicizzati
                                var day = dayDate.getDate();
                                var dateString = year + "-" + (month < 10 ? '0' + month : month) + "-" + (day < 10 ? '0' + day : day);

                                workoutModel.setDateFilter(dateString);
                                calendarPopup.close();
                            }
                        }

                        // Modern: the cell itself is bare; the round day on top of it is filled
                        // only when there was a workout, and today gets a ring
                        color: {
                            if (workoutHistoryPage.modern)
                                return "transparent"
                            if (mouseArea.pressed) return "#e3f2fd"
                            if (isToday) return "#bbdefb"
                            if (!isCurrentMonth) return "#f5f5f5"
                            return "white"
                        }

                        border.color: isToday ? "#2196f3" : "#e0e0e0"
                        border.width: workoutHistoryPage.modern ? 0 : (isToday ? 2 : 1)
                        radius: workoutHistoryPage.modern ? 10 : 4

                        UiFrame {
                            visible: workoutHistoryPage.modern
                            anchors.centerIn: parent
                            width: Math.min(calendar.dayCircle, parent.height - 2)
                            height: width
                            radius: width / 2
                            base: window.ui.surfaceHigh
                            fill: mouseArea.pressed ? window.ui.surfaceHighest
                                : hasWorkout ? window.ui.alpha(window.ui.accent, isCurrentMonth ? 0.28 : 0.12)
                                : "transparent"
                            stroke: window.ui.accent
                            strokeWidth: isToday ? 2 : 0
                        }

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: dayDate.getDate()
                                color: workoutHistoryPage.modern ? (isCurrentMonth ? window.ui.textMain : window.ui.textMuted)
                                                                 : (isCurrentMonth ? "black" : "#cccccc")
                                opacity: workoutHistoryPage.modern && !isCurrentMonth ? 0.6 : 1
                                // Modern look: Body Large of the Material 3 date picker (fontBody)
                                font.pixelSize: workoutHistoryPage.modern ? window.ui.fontBody : window.fontPx(14)
                                font.weight: workoutHistoryPage.modern && (hasWorkout || isToday) ? Font.DemiBold : Font.Normal
                            }
                            
                            // Workout indicator dot
                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 8
                                height: 8
                                radius: 4
                                color: "#ff6b35"
                                visible: hasWorkout && !workoutHistoryPage.modern
                                border.width: 1
                                border.color: "#cc5529"
                                
                                // Debug: log when a dot should be visible
                                Component.onCompleted: {
                                    if (hasWorkout) {
                                        console.log("Workout dot visible for date:", dayDate.toDateString())
                                    }
                                }
                            }
                        }
                        
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            
                            onClicked: parent.selectDay()
                        }
                    }
                }
            }
            
            // Close button
            UiButton {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Close")
                cardSurface: true
                onClicked: calendarPopup.close()
            }
        }
    }

    // JavaScript functions for calendar

    // First day of the week (0 = Sunday, as Date.getDay()): the locale's in the modern look
    function weekStart() {
        return workoutHistoryPage.modern ? Qt.locale().firstDayOfWeek % 7 : 0
    }

    function weekDayNames() {
        var names = []
        for (var i = 0; i < 7; i++)
            names.push(Qt.locale().dayName((weekStart() + i) % 7, Locale.ShortFormat))
        return names
    }

    function monthTitle(date) {
        var name = Qt.locale().standaloneMonthName(date.getMonth(), Locale.LongFormat)
        return name.charAt(0).toUpperCase() + name.slice(1) + " " + date.getFullYear()
    }

    function getCalendarDays() {
        var days = []
        var firstDay = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth(), 1)
        var lastDay = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth() + 1, 0)
        var startDate = new Date(firstDay)
        // Go back to start of week: Sunday in the classic look, the locale's first day in the modern one
        startDate.setDate(startDate.getDate() - (firstDay.getDay() - weekStart() + 7) % 7)
        
        var workoutDates = calendar.workoutDates || []
        console.log("getCalendarDays: workoutDates received:", JSON.stringify(workoutDates))
        
        // workoutDates is now a QStringList (array of strings in format "yyyy-MM-dd")
        var workoutDateStrings = workoutDates || []
        console.log("Final workout date strings:", JSON.stringify(workoutDateStrings))
        
        for (var i = 0; i < 42; i++) { // 6 rows x 7 days
            var currentDate = new Date(startDate)
            currentDate.setDate(startDate.getDate() + i)
            
            // Costruisci la stringa YYYY-MM-DD dai componenti della data locale per evitare problemi di fuso orario
            var year = currentDate.getFullYear();
            var month = currentDate.getMonth() + 1; // i mesi JS sono 0-indicizzati
            var day = currentDate.getDate();
            var localDateString = year + "-" + (month < 10 ? '0' + month : month) + "-" + (day < 10 ? '0' + day : day);

            var hasWorkout = workoutDateStrings.indexOf(localDateString) !== -1;
            if (hasWorkout) {
                // Questo console.log ora utilizza la stringa della data locale corretta per la corrispondenza
                console.log("Found workout match for:", localDateString);
            }
            
            var isCurrentMonth = currentDate.getMonth() === calendar.selectedDate.getMonth()
            
            days.push({
                date: currentDate,
                currentMonth: isCurrentMonth,
                hasWorkout: hasWorkout
            })
        }
        
        console.log("getCalendarDays: returning", days.length, "days")
        return days
    }
}
