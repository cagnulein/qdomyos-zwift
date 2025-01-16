import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15

Page {
    id: workoutHistoryPage

    // Signal for chart preview
    signal fitfile_preview_clicked(var url)

    // Sport type to icon mapping
    function getSportIcon(sport) {
        switch(sport) {
            case 0: return "🏃"; // Running/Treadmill
            case 1: return "🚴"; // Cycling
            case 2: return "⭕"; // Elliptical
            case 3: return "🚣"; // Rowing
            default: return "💪";
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
            visible: workoutModel ? workoutModel.isLoading : false
            running: visible
        }

        // Workout List
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: workoutModel
            spacing: 8
            clip: true

            delegate: SwipeDelegate {
                id: swipeDelegate
                width: ListView.view.width
                height: 100

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
                            Text {
                                text: "Raw sport: " + sport
                                color: "red"
                                font.pixelSize: 10
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
                                    text: "📏 " + distance + " km"
                                }

                                Text {
                                    text: "🔥 " + calories + " kcal"
                                }
                            }
                        }
                    }
                }

                onClicked: {
                    // Get workout details from the model
                    var details = workoutModel.getWorkoutDetails(model.id)

                    // Emit signal with file URL for chart preview
                    workoutHistoryPage.fitfile_preview_clicked(details.filePath)

                    // Push the ChartJsTest view
                    stackView.push("ChartJsTest.qml")
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
}
