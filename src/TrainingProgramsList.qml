import QtQuick 2.7
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0
import QtCharts 2.2
import Qt.labs.settings 1.0

ColumnLayout {
    signal trainprogram_open_clicked(url name)
    signal trainprogram_preview(url name)
    FileDialog {
        id: fileDialogTrainProgram
        title: "Please choose a file"
        folder: shortcuts.home
        onAccepted: {
            console.log("You chose: " + fileDialogTrainProgram.fileUrl)
            trainprogram_open_clicked(fileDialogTrainProgram.fileUrl)
            fileDialogTrainProgram.close()
        }
        onRejected: {
            console.log("Canceled")
            fileDialogTrainProgram.close()
        }
    }

    RowLayout{
        spacing: 2
        anchors.top: parent.top

        ListView {
            Layout.fillWidth: true
            Layout.minimumWidth: 50
            Layout.preferredWidth: 100
            Layout.maximumWidth: 300
            Layout.minimumHeight: 150
            id: list
            anchors.fill: parent
            FolderListModel {
                id: folderModel
                nameFilters: ["*.xml", "*.zwo"]
                folder: "file://" + rootItem.getWritableAppDir() + 'training'
                showDotAndDotDot: false
                showDirs: true
            }
            model: folderModel
            delegate: Component {
                Rectangle {
                    property alias textColor: fileTextBox.color
                    width: parent.width
                    height: 40
                    color: Material.backgroundColor
                    z: 1
                    Text {
                        id: fileTextBox
                        color: Material.color(Material.Grey)
                        font.pixelSize: Qt.application.font.pixelSize * 1.6
                        text: fileName.substring(0, fileName.length-4)
                    }
                    MouseArea {
                        anchors.fill: parent
                        z: 100
                        onClicked: {
                            console.log('onclicked ' + index+ " count "+list.count);
                            if (index == list.currentIndex) {
                                let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                                if (fileUrl) {
                                    trainprogram_open_clicked(fileUrl);
                                    popup.open()
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
                /*Text {
                        anchors.centerIn: parent
                        text: 'Selected ' + folderModel.get(list.currentIndex, "fileName")
                        color: "white"
                    }*/
            }
            focus: true
            onCurrentItemChanged: {
                let fileUrl = folderModel.get(list.currentIndex, 'fileUrl') || folderModel.get(list.currentIndex, 'fileURL');
                if (fileUrl) {
                    list.currentItem.textColor = Material.color(Material.Yellow)
                    console.log(fileUrl + ' selected');
                    trainprogram_preview(fileUrl)
                    powerSeries.clear();
                    for(var i=0;i<rootItem.preview_workout_points;i+=10)
                    {
                        powerSeries.append(i * 1000, rootItem.preview_workout_watt[i]);
                    }
                    rootItem.update_chart_power(powerChart);
                    //trainprogram_open_clicked(fileUrl);
                    //popup.open()
                }
            }
            Component.onCompleted: {

            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 100
            Layout.preferredWidth: 200
            Layout.preferredHeight: 100

            property alias powerSeries: powerSeries
            property alias powerChart: powerChart

            Settings {
                id: settings
                property real ftp: 200.0
            }

            Row {
                id: row
                anchors.fill: parent

                Text {
                    id: date
                    text: rootItem.workoutStartDate
                    font.pixelSize: 16
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    anchors.top: date.bottom
                    id: title
                    text: rootItem.workoutName
                    font.pixelSize: 24
                    color: "yellow"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    anchors.top: title.bottom
                    id: description
                    text: rootItem.instructorName
                    font.pixelSize: 18
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                ScrollView {
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOff
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: instructor.bottom
                    anchors.bottom: parent.bottom
                    contentHeight: powerChart.height

                    ChartView {
                        id: powerChart
                        objectName: "powerChart"
                        antialiasing: true
                        legend.visible: false
                        height: 400
                        width: parent.width
                        title: "Power"
                        titleFont.pixelSize: 20

                        DateTimeAxis {
                            id: valueAxisX
                            tickCount: 7
                            min: new Date(0)
                            max: new Date(rootItem.preview_workout_points * 1000)
                            format: "mm:ss"
                            //labelsVisible: false
                            gridVisible: false
                            //lineVisible: false
                            labelsFont.pixelSize: 10
                        }

                        ValueAxis {
                            id: valueAxisY
                            min: 0
                            max: rootItem.wattMaxChart
                            //tickCount: 60
                            tickCount: 8
                            labelFormat: "%.0f"
                            //labelsVisible: false
                            //gridVisible: false
                            //lineVisible: false
                            labelsFont.pixelSize: 10
                        }

                        LineSeries {
                            //name: "Power"
                            id: powerSeries
                            visible: true
                            axisX: valueAxisX
                            axisY: valueAxisY
                            color: "black"
                            width: 1
                        }
                    }
                }
            }
        }
    }

    Button {
        id: searchButton
        height: 50
        width: parent.width
        text: "Other folders"
        Layout.alignment: Qt.AlignCenter | Qt.AlignVCenter
        onClicked: {
            console.log("folder is " + rootItem.getWritableAppDir() + 'training')
            fileDialogTrainProgram.visible = true
        }
        anchors {
            bottom: parent.bottom
        }
    }
}
