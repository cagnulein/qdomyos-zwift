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
    signal trainprogram_open_other_folder(url name)
    signal trainprogram_preview(url name)
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

    RowLayout{
        spacing: 2
        anchors.top: parent.top
        anchors.fill: parent

        ColumnLayout {
            spacing: 0
            anchors.top: parent.top
            anchors.fill: parent

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
                Layout.maximumWidth: row.left
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
            anchors.top: parent.top
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            contentHeight: date.height + description.height + powerChart.height
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            Layout.minimumWidth: 100
            Layout.preferredWidth: 200

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
                    width: parent.width
                    text: rootItem.previewWorkoutDescription
                    font.pixelSize: 14
                    color: "white"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    anchors.top: date.bottom
                    id: description
                    width: parent.width
                    text: rootItem.previewWorkoutTags
                    font.pixelSize: 10
                    wrapMode: Text.WordWrap
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: description.bottom
                    anchors.bottom: parent.bottom

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
            // Create a fresh FileDialog instance
            fileDialogLoader.active = true
        }
        anchors {
            bottom: parent.bottom
        }
    }
}
