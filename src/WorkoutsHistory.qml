import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15

Page {
    id: workoutHistoryPage

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

            Text {
                anchors.centerIn: parent
                text: "Workout History"
                font.pixelSize: 24
                font.bold: true
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
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.bottomMargin: streakBanner.height + 10
            model: workoutModel
            spacing: 8
            clip: true

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
                            text: "🗑️ Delete"
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

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 16

                        // Sport icon
                        Column {
                            Layout.alignment: Qt.AlignVCenter
                            Text {
                                text: getSportIcon(sport)
                                font.pixelSize: 32
                            }
                        }

                        // Workout info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: title
                                font.bold: true
                                font.pixelSize: 18
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
                                    text: "🔥 " + Math.round(calories) + " kcal"
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

    // Streak Banner at the bottom
    Rectangle {
        id: streakBanner
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80
        visible: workoutModel && (workoutModel.currentStreak > 0 || workoutModel.longestStreak > 0)
        
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
                    text: workoutModel && workoutModel.currentStreak >= 365 ? "👑🔥" :
                          workoutModel && workoutModel.currentStreak >= 180 ? "🎖️🔥" :
                          workoutModel && workoutModel.currentStreak >= 90 ? "🦁🔥" :
                          workoutModel && workoutModel.currentStreak >= 30 ? "🎊🔥" :
                          workoutModel && workoutModel.currentStreak >= 7 ? "🏆🔥" : "🔥"
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
                    text: workoutModel && workoutModel.currentStreak >= 365 ? "🔥👑" :
                          workoutModel && workoutModel.currentStreak >= 180 ? "🔥🎖️" :
                          workoutModel && workoutModel.currentStreak >= 90 ? "🔥🦁" :
                          workoutModel && workoutModel.currentStreak >= 30 ? "🔥🎊" :
                          workoutModel && workoutModel.currentStreak >= 7 ? "🔥🏆" : "🔥"
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
}
