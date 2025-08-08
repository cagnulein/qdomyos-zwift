import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15
import Qt.labs.calendar 1.0

Page {
    id: workoutHistoryPage

    FontLoader {
        id: emojiFont
        source: "qrc:/fonts/NotoColorEmoji_WindowsCompatible.ttf"
    }

    // Signal for chart preview
    signal fitfile_preview_clicked(var url)

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

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // Header
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#f5f5f5"

            // Calendar Icon Button - positioned absolutely on the left
            Button {
                id: calendarButton
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 12
                width: 48
                height: 48
                
                background: Rectangle {
                    radius: 8
                    color: calendarButton.pressed ? "#e0e0e0" : "#f0f0f0"
                    border.color: "#d0d0d0"
                    border.width: 1
                }
                
                contentItem: Text {
                    text: Qt.platform.os === "android" ? 
                          '<font face="' + emojiFont.name + '">📅</font>' : 
                          "📅"
                    textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                    font.pixelSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    calendarPopup.open()
                }
            }

            // Title with filter status - centered
            Column {
                anchors.centerIn: parent
                
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Workout History"
                    font.pixelSize: 24
                    font.bold: true
                }
                
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: workoutModel && workoutModel.isDateFiltered ? 
                          "Filtered: " + workoutModel.filteredDate.toLocaleDateString() : ""
                    font.pixelSize: 12
                    color: "#666666"
                    visible: workoutModel && workoutModel.isDateFiltered
                }
            }

            // Clear Filter Button - positioned absolutely on the right
            Button {
                id: clearFilterButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 12
                width: 100
                height: 36
                visible: workoutModel && workoutModel.isDateFiltered
                
                background: Rectangle {
                    radius: 6
                    color: clearFilterButton.pressed ? "#ff6666" : "#ff8888"
                    border.color: "#ff4444"
                    border.width: 1
                }
                
                contentItem: Text {
                    text: "Clear Filter"
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    workoutModel.clearDateFilter()
                }
            }
        }

        // Loading indicator
        BusyIndicator {
            id: loadingIndicator
            Layout.alignment: Qt.AlignHCenter
            visible: workoutModel ? (workoutModel.isLoading || workoutModel.isDatabaseProcessing) : false
            running: visible
        }
        
        // Database processing message
        Text {
            Layout.alignment: Qt.AlignHCenter
            visible: workoutModel ? workoutModel.isDatabaseProcessing : false
            text: "Processing workout files...\nThis may take a few moments on first startup."
            horizontalAlignment: Text.AlignHCenter
            color: "#666666"
            font.pixelSize: 16
        }

        // Workout List
        ListView {
            id: workoutListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.bottomMargin: streakBanner.visible ? streakBanner.height + 10 : 10
            model: workoutModel
            spacing: 8
            clip: true
            
            onContentYChanged: {
                // Hide banner when scrolling down, show when at top
                streakBanner.visible = contentY <= 20
            }

            delegate: SwipeDelegate {
                id: swipeDelegate
                width: parent.width
                height: 135

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
                    color: "#FF4444"
                    clip: true

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.rightMargin: 20

                        Text {
                            text: Qt.platform.os === "android" ? 
                                  '<font face="' + emojiFont.name + '">🗑️</font> Delete' : 
                                  "🗑️ Delete"
                            textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                            color: "white"
                            font.pixelSize: 16
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                swipe.onCompleted: {
                    // Show confirmation dialog
                    confirmDialog.workoutId = model.id
                    confirmDialog.workoutTitle = model.title
                    confirmDialog.open()
                }

                // Card-like container
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 8
                    radius: 10
                    color: "white"
                    border.color: "#e0e0e0"

                    // Workout Type Tag - positioned absolutely in top-right
                    WorkoutTypeTag {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        workoutSource: workoutModel ? workoutModel.getWorkoutSource(model.id) : "QZ"
                    }

                    // Action buttons - positioned absolutely in bottom-right
                    Row {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 12
                        spacing: 8
                        
                        // Peloton URL button
                        Button {
                            width: 40
                            height: 45
                            visible: workoutModel && workoutModel.getWorkoutSource(model.id) === "PELOTON" && 
                                    workoutModel.getPelotonUrl(model.id) !== ""
                            
                            background: Rectangle {
                                color: parent.pressed ? "#ff8855" : "#ff6b35"
                                radius: 6
                                border.color: "#cc5529"
                                border.width: 1
                            }
                            
                            contentItem: Text {
                                text: Qt.platform.os === "android" ? 
                                      '<font face="' + emojiFont.name + '">🌐</font>' : 
                                      "🌐"
                                textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                                font.pixelSize: 16
                                color: "white"
                                anchors.centerIn: parent
                            }
                            
                            onClicked: {
                                workoutModel.openPelotonUrl(model.id)
                            }
                        }
                        
                        // Training Program button
                        Button {
                            width: 40
                            height: 45
                            visible: workoutModel && workoutModel.hasTrainingProgram(model.id)
                            
                            background: Rectangle {
                                color: parent.pressed ? "#1976d2" : "#2196f3"
                                radius: 6
                                border.color: "#1565c0"
                                border.width: 1
                            }
                            
                            contentItem: Text {
                                text: Qt.platform.os === "android" ? 
                                      '<font face="' + emojiFont.name + '">📋</font>' : 
                                      "📋"
                                textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                                font.pixelSize: 16
                                color: "white"
                                anchors.centerIn: parent
                            }
                            
                            onClicked: {
                                var success = workoutModel.loadTrainingProgram(model.id)
                                if (success) {
                                    trainingProgramDialog.title = "Success"
                                    trainingProgramDialog.message = "Training program loaded successfully!"
                                    trainingProgramDialog.isSuccess = true
                                } else {
                                    trainingProgramDialog.title = "Error"
                                    trainingProgramDialog.message = "Failed to load training program. Please check if the file exists."
                                    trainingProgramDialog.isSuccess = false
                                }
                                trainingProgramDialog.open()
                            }
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 16

                        // Sport icon
                        Column {
                            Layout.alignment: Qt.AlignVCenter
                            Text {
                                text: Qt.platform.os === "android" ? 
                                      '<font face="' + emojiFont.name + '">' + getSportIcon(sport) + '</font>' : 
                                      getSportIcon(sport)
                                textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                                font.pixelSize: 32
                            }
                        }

                        // Workout info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            // Title row (without tag) with auto-scrolling
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.rightMargin: 80 // Reserve space for tag
                                Layout.preferredHeight: 24
                                clip: true
                                color: "transparent"
                                
                                Text {
                                    id: titleText
                                    text: title
                                    font.bold: true
                                    font.pixelSize: 18
                                    anchors.verticalCenter: parent.verticalCenter
                                    
                                    // Auto-scroll animation for long titles
                                    SequentialAnimation on x {
                                        running: titleText.contentWidth > titleText.parent.width
                                        loops: Animation.Infinite
                                        NumberAnimation {
                                            from: 0
                                            to: -(titleText.contentWidth - titleText.parent.width + 20)
                                            duration: Math.max(3000, titleText.contentWidth * 30)
                                        }
                                        PauseAnimation { duration: 1500 }
                                        NumberAnimation {
                                            from: -(titleText.contentWidth - titleText.parent.width + 20)
                                            to: 0
                                            duration: Math.max(3000, titleText.contentWidth * 30)
                                        }
                                        PauseAnimation { duration: 2000 }
                                    }
                                }
                            }

                            Text {
                                text: date
                                color: "#666666"
                            }

                            // Stats row
                            RowLayout {
                                spacing: 16

                                Text {
                                    text: "⏱ " + duration
                                }

                                Text {
                                    text: "📏 " + distance.toFixed(2) + " km"
                                }
                            }

                            RowLayout {
                                spacing: 16

                                Text {
                                    text: Qt.platform.os === "android" ? 
                                          '<font face="' + emojiFont.name + '">🔥</font> ' + Math.round(calories) + ' kcal' : 
                                          "🔥 " + Math.round(calories) + " kcal"
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
                    stackView.push("PreviewChart.qml")
                }
            }
        }
    }

    // Confirmation Dialog
    Dialog {
        id: confirmDialog

        property int workoutId
        property string workoutTitle

        title: "Delete Workout"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        Text {
            text: "Are you sure you want to delete '" + confirmDialog.workoutTitle + "'?"
        }

        onAccepted: {
            workoutModel.deleteWorkout(confirmDialog.workoutId)
            swipeDelegate.swipe.close()
        }
        onRejected: {
            swipeDelegate.swipe.close()
        }
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
                font.pixelSize: 18
                font.bold: true
            }
        }

        contentItem: ColumnLayout {
            spacing: 16

            Text {
                Layout.margins: 20
                Layout.preferredWidth: 300
                Layout.preferredHeight: 120
                text: Qt.platform.os === "android" ? 
                      '<font face="' + emojiFont.name + '">' + 
                      (trainingProgramDialog.isSuccess ? '✅' : '❌') + 
                      '</font> ' + trainingProgramDialog.message : 
                      (trainingProgramDialog.isSuccess ? '✅ ' : '❌ ') + trainingProgramDialog.message
                textFormat: Qt.platform.os === "android" ? Text.RichText : Text.PlainText
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 14
            }
        }
    }

    // Streak Banner at the bottom
    Rectangle {
        id: streakBanner
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80
        visible: workoutModel
        
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
            anchors.centerIn: parent
            spacing: 4

            // Current streak with count
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 15

                // Fire emoji with animation
                Text {
                    text: Qt.platform.os === "android" ? (
                          workoutModel && workoutModel.currentStreak >= 365 ? '<font face="' + emojiFont.name + '">👑🔥</font>' :
                          workoutModel && workoutModel.currentStreak >= 180 ? '<font face="' + emojiFont.name + '">🎖️🔥</font>' :
                          workoutModel && workoutModel.currentStreak >= 90 ? '<font face="' + emojiFont.name + '">🦁🔥</font>' :
                          workoutModel && workoutModel.currentStreak >= 30 ? '<font face="' + emojiFont.name + '">🎊🔥</font>' :
                          workoutModel && workoutModel.currentStreak >= 7 ? '<font face="' + emojiFont.name + '">🏆🔥</font>' : '<font face="' + emojiFont.name + '">🔥</font>'
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
                    text: workoutModel ? workoutModel.currentStreak + " day" + (workoutModel.currentStreak !== 1 ? "s" : "") + " streak" : ""
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    visible: workoutModel
                }

                // Another fire emoji
                Text {
                    text: Qt.platform.os === "android" ? (
                          workoutModel && workoutModel.currentStreak >= 365 ? '<font face="' + emojiFont.name + '">🔥👑</font>' :
                          workoutModel && workoutModel.currentStreak >= 180 ? '<font face="' + emojiFont.name + '">🔥🎖️</font>' :
                          workoutModel && workoutModel.currentStreak >= 90 ? '<font face="' + emojiFont.name + '">🔥🦁</font>' :
                          workoutModel && workoutModel.currentStreak >= 30 ? '<font face="' + emojiFont.name + '">🔥🎊</font>' :
                          workoutModel && workoutModel.currentStreak >= 7 ? '<font face="' + emojiFont.name + '">🔥🏆</font>' : '<font face="' + emojiFont.name + '">🔥</font>'
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
                font.pixelSize: 14
                font.italic: true
                color: "white"
                visible: workoutModel && workoutModel.streakMessage !== ""
                opacity: 0.9
            }
            
            // Best streak (smaller text)
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: workoutModel ? "Personal best: " + workoutModel.longestStreak + " day" + (workoutModel.longestStreak !== 1 ? "s" : "") : ""
                font.pixelSize: 12
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
    Popup {
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
            color: "white"
            radius: 12
            border.color: "#d0d0d0"
            border.width: 1
            
            // Shadow effect
            Rectangle {
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
                
                Button {
                    text: "<"
                    onClicked: calendar.selectedDate = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth() - 1, 1)
                }
                
                Text {
                    Layout.fillWidth: true
                    text: calendar.selectedDate.toLocaleDateString(Qt.locale(), "MMMM yyyy")
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
                
                Button {
                    text: ">"
                    onClicked: calendar.selectedDate = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth() + 1, 1)
                }
            }

            // Calendar Grid
            GridLayout {
                id: calendar
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 7
                
                property date selectedDate: new Date()
                property var workoutDates: workoutModel ? workoutModel.getWorkoutDates() : []
                
                // Debug: print workout dates when they change
                onWorkoutDatesChanged: {
                    console.log("Calendar workout dates updated:", JSON.stringify(workoutDates))
                }
                
                // Day headers
                Repeater {
                    model: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
                    Text {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        text: modelData
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: "#666666"
                    }
                }
                
                // Calendar days
                Repeater {
                    model: getCalendarDays()
                    
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 40
                        
                        property date dayDate: modelData.date
                        property bool isCurrentMonth: modelData.currentMonth
                        property bool hasWorkout: modelData.hasWorkout
                        property bool isToday: dayDate.toDateString() === new Date().toDateString()
                        
                        color: {
                            if (mouseArea.pressed) return "#e3f2fd"
                            if (isToday) return "#bbdefb"
                            if (!isCurrentMonth) return "#f5f5f5"
                            return "white"
                        }
                        
                        border.color: isToday ? "#2196f3" : "#e0e0e0"
                        border.width: isToday ? 2 : 1
                        radius: 4
                        
                        Column {
                            anchors.centerIn: parent
                            spacing: 2
                            
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: dayDate.getDate()
                                color: isCurrentMonth ? "black" : "#cccccc"
                                font.pixelSize: 14
                            }
                            
                            // Workout indicator dot
                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                width: 8
                                height: 8
                                radius: 4
                                color: "#ff6b35"
                                visible: hasWorkout
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
                            
                            onClicked: {
                                if (isCurrentMonth) {
                                    var year = dayDate.getFullYear();
                                    var month = dayDate.getMonth() + 1; // i mesi JS sono 0-indicizzati
                                    var day = dayDate.getDate();
                                    var dateString = year + "-" + (month < 10 ? '0' + month : month) + "-" + (day < 10 ? '0' + day : day);

                                    workoutModel.setDateFilter(dateString);
                                    calendarPopup.close();
                                }
                            }
                        }
                    }
                }
            }
            
            // Close button
            Button {
                Layout.alignment: Qt.AlignHCenter
                text: "Close"
                onClicked: calendarPopup.close()
            }
        }
    }

    // JavaScript functions for calendar
    function getCalendarDays() {
        var days = []
        var firstDay = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth(), 1)
        var lastDay = new Date(calendar.selectedDate.getFullYear(), calendar.selectedDate.getMonth() + 1, 0)
        var startDate = new Date(firstDay)
        startDate.setDate(startDate.getDate() - firstDay.getDay()) // Go back to start of week
        
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
