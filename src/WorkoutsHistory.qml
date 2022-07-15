import QtQuick 2.7
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import QtQuick.Dialogs 1.0
import QtCharts 2.2
import Qt.labs.settings 1.0
import QtWebView 1.1

ColumnLayout {
    signal fitfile_preview(url name)

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
                        folderModel.nameFilters = [filter + ".fit"]
                    }
                    id: filterField
                    onTextChanged: updateFilter()
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
                    nameFilters: ["*.fit"]
                    folder: "file://" + rootItem.getWritableAppDir()
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
                        Item {
                            id: root
                            property alias text: fileTextBox.text
                            property int spacing: 30
                            width: fileTextBox.width + spacing
                            height: fileTextBox.height
                            clip: true
                            Text {
                                id: fileTextBox
                                color: Material.color(Material.Grey)
                                font.pixelSize: Qt.application.font.pixelSize * 1.6
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
                        fitfile_preview(fileUrl)
                        webView.reload();
                    }
                }
                Component.onCompleted: {

                }
            }
        }

        ScrollView {
            anchors.top: parent.top
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            //contentHeight: map.height
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 100
            Layout.preferredWidth: 200

            Row {
                id: row
                anchors.fill: parent

                Settings {
                    id: settings
                }
                WebView {
                    height: parent.height
                    width: parent.width
                    id: webView
                    url: "http://localhost:" + settings.value("template_inner_QZWS_port") + "/chartjs/chart.htm"
                    visible: true
                    onLoadingChanged: {
                        if (loadRequest.errorString) {
                            console.error(loadRequest.errorString);
                            console.error("port " + settings.value("template_inner_QZWS_port"));
                        }
                    }
                }

            }
        }
    }
}
