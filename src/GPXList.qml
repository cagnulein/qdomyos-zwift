import QtQuick
import Qt.labs.folderlistmodel
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCharts
import Qt.labs.settings
import QtPositioning
import QtLocation

ColumnLayout {
    signal trainprogram_open_clicked(url name)
    signal trainprogram_open_other_folder(url name)
    signal trainprogram_preview(url name)
    FileDialog {
        id: fileDialogTrainProgram
        // In Qt6, StandardPaths è stato spostato in un modulo separato
        // quindi usiamo un approccio più generico
        currentFolder: StandardPaths ? StandardPaths.standardLocations(StandardPaths.HomeLocation)[0] : ""
        title: "Please choose a file"
        onAccepted: {
            console.log("You chose: " + fileDialogTrainProgram.selectedFile)
            if(OS_VERSION === "Android") {
                trainprogram_open_other_folder(fileDialogTrainProgram.selectedFile)
            } else {
                trainprogram_open_clicked(fileDialogTrainProgram.selectedFile)
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
                        folderModel.nameFilters = [filter + ".gpx"]
                    }
                    id: filterField
                    onTextChanged: updateFilter()
                }
                Button {
                     // Rimuovere o aggiornare questo riferimento a mainRect se non esiste
                     // anchors.left: mainRect.right
                     // anchors.leftMargin: 5
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
                    nameFilters: ["*.gpx"]
                    folder: "file://" + rootItem.getWritableAppDir() + 'gpx'
                    showDotAndDotDot: false
                    showDirs: true
                    sortField: FolderListModel.Name  // Usa l'enum corretto in Qt6
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
                                color: (!folderModel.isFolder(index) ? Material.color(Material.Grey) : Material.color(Material.Orange))
                                font.pixelSize: Qt.application.font.pixelSize * 1.6
                                text: fileName.substring(0, fileName.length-4)
                                NumberAnimation on x {
                                    Component.onCompleted: {
                                        if(fileName.length > 30) {
                                            running = true;  // Corretto da running: true a running = true
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
                                    // In Qt6, l'accesso ai modelli è leggermente diverso
                                    let fileUrl = folderModel.get(list.currentIndex, "fileUrl") || folderModel.get(list.currentIndex, "fileURL");
                                    if (fileUrl && !folderModel.isFolder(list.currentIndex)) {
                                        trainprogram_open_clicked(fileUrl);
                                        if (typeof popup !== 'undefined' && popup) {
                                            popup.open()
                                        }
                                    } else {
                                        folderModel.folder = fileUrl
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

                Text {
                    id: distance
                    width: parent.width
                    text: rootItem.previewWorkoutDescription
                    font.pixelSize: 16
                    color: "white"
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                // Modifica nella creazione del plugin per seguire lo schema di Qt6
                Plugin {
                    id: osmMapPlugin
                    name: "osm"
                    // In Qt6, è preferibile usare PluginParameter esplicito
                    PluginParameter {
                        name: "osm.useragent"
                        value: "QZ Fitness"
                    }
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

                    // In Qt6, MapPolyline ha mantenuto la stessa API ma verifichiamo
                    MapPolyline {
                        id: pl
                        line {
                            width: 3
                            color: 'red'
                        }
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
                    distance.text = "Distance " + (pathController.geopath.length() / 1000.0).toFixed(1) + " km Elevation Gain: " + elevationGain.toFixed(1) + " meters"
                    return lines;
                }

                // In Qt6, la sintassi delle connessioni è cambiata
                Connections{
                    target: pathController
                    // Utilizziamo la nuova sintassi on<SignalName>
                    function onGeopathChanged() {
                        pl.path = row.loadPath();
                    }
                    function onCenterChanged() {
                        map.center = pathController.center;
                    }
                }

                Component.onCompleted: pl.path = loadPath()
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
            console.log("folder is " + rootItem.getWritableAppDir() + 'gpx')
            // Create a fresh FileDialog instance
            fileDialogLoader.active = true
        }
        anchors {
            bottom: parent.bottom
        }
    }
}
