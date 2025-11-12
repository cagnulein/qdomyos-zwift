import QtQuick 2.7
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0
import QtCharts 2.2
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1

ColumnLayout {
    signal trainprogram_open_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal trainprogram_preview(url name)
    signal trainprogram_autostart_requested()

    property url pendingWorkoutUrl: ""

    // Auto-start confirmation dialog
    MessageDialog {
        id: autoStartDialog
        text: "Start Workout?"
        informativeText: "Do you want to automatically start this workout?"
        buttons: (MessageDialog.Yes | MessageDialog.No)
        onYesClicked: {
            // Load workout and auto-start
            trainprogram_open_clicked(pendingWorkoutUrl)
            trainprogram_autostart_requested()
            this.visible = false
        }
        onNoClicked: {
            // Just load workout without auto-start
            trainprogram_open_clicked(pendingWorkoutUrl)
            this.visible = false
        }
        visible: false
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: Component {
            FileDialog {
                title: "Please choose a file"
                folder: shortcuts.home
                visible: true
                onAccepted: {
                    console.log("You chose: " + fileUrl)
                    if(OS_VERSION === "Android") {
                        trainprogram_open_other_folder(fileUrl)
                    } else {
                        trainprogram_open_clicked(fileUrl)
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

    SplitView {
        anchors.top: parent.top
        anchors.fill: parent
        orientation: Qt.Horizontal

        ColumnLayout {
            SplitView.preferredWidth: 300
            SplitView.minimumWidth: 200
            spacing: 0

            Row
            {
                spacing: 5
                Text
                {
                    text:"Filter"
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                }
                TextField
                {
                    function updateFilter()
                    {
                        var text = filterField.text
                        var filter = "*"
                        for(var i = 0; i<text.length; i++)
                           filter+= "[%1%2]".arg(text[i].toUpperCase()).arg(text[i].toLowerCase())
                        filter+="*"
                        print(filter)
                        folderModel.nameFilters = [filter + ".zwo", filter + ".xml"]
                    }
                    id: filterField
                    onTextChanged: updateFilter()
                }
                Button {
                    text: "←"
						  onClicked: folderModel.folder = folderModel.parentFolder
						}
            }

            ListView {
                Layout.fillWidth: true
                Layout.minimumWidth: 50
                Layout.preferredWidth: 100
                Layout.minimumHeight: 150
                Layout.preferredHeight: parent.height
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
                model: folderModel
                delegate: Component {
                    Rectangle {
                        property alias textColor: fileTextBox.color
                        width: parent.width
                        height: 40
								color: Material.backgroundColor
                        z: 1
                        Item {
                            id: root
                            property alias text: fileTextBox.text
                            property int spacing: 30
                            width: fileTextBox.width + spacing
                            height: fileTextBox.height
                            clip: true
                            Text {
                                id: fileTextBox
                                color: (!folderModel.isFolder(index)?Material.color(Material.Grey):Material.color(Material.Orange))
                                font.pixelSize: Qt.application.font.pixelSize * 1.6
                                text: (!folderModel.isFolder(index)?fileName.substring(0, fileName.length-4):fileName)
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
                                  font.pixelSize: Qt.application.font.pixelSize * 1.6
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
                                        // Show auto-start dialog
                                        console.log('Showing autostart dialog for: ' + fileUrl);
                                        pendingWorkoutUrl = fileUrl;
                                        autoStartDialog.visible = true;
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
        }

        ScrollView {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 300
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            clip: true

            property alias powerSeries: powerSeries
            property alias powerChart: powerChart

            Settings {
                id: settings
                property real ftp: 200.0
            }

            ColumnLayout {
                width: parent.width
                spacing: 5

                Text {
                    id: date
                    Layout.fillWidth: true
                    text: rootItem.previewWorkoutDescription
                    font.pixelSize: 14
                    color: "white"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Text {
                    id: description
                    Layout.fillWidth: true
                    text: rootItem.previewWorkoutTags
                    font.pixelSize: 10
                    wrapMode: Text.WordWrap
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                ChartView {
                    id: powerChart
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 300
                    objectName: "powerChart"
                    antialiasing: true
                    legend.visible: false
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
            // Create a fresh FileDialog instance
            fileDialogLoader.active = true
        }
        anchors {
            bottom: parent.bottom
        }
    }
}
