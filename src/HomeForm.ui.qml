import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import QtCharts 2.15

Page {

    title: qsTr("qDomyos-Zwift")
    id: page

    property alias start: start
    property alias stop: stop
    property alias lap: lap
    property alias row: row

    property alias series1: series1
    property alias series2: series2
    property alias series3: series3
    property alias series4: series4
    property alias series5: series5
    property alias series6: series6
    property alias series7: series7

    Item {
        width: parent.width
        height: rootItem.topBarHeight
        id: topBar

        Row {
            id: row
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            height: topBar.height - 20
            spacing: 5
            padding: 5

            Rectangle {
                width: 50
                height: row.height
                color: Material.backgroundColor
                Column {
                    id: column
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    height: row.height
                    spacing: 0
                    padding: 0
                    Rectangle {
                        width: 50
                        height: row.height
                        color: Material.backgroundColor

                        Image {
                            anchors.verticalCenter: parent.verticalCenter
                            id: treadmill_connection
                            width: 48
                            height: row.height - 52
                            source: "icons/icons/bluetooth-icon.png"
                            enabled: rootItem.device
                            smooth: true
                        }
                        ColorOverlay {
                            anchors.fill: treadmill_connection
                            source: treadmill_connection
                            color: treadmill_connection.enabled ? "#00000000" : "#B0D3d3d3"
                        }
                    }
                    Image {
                        anchors.horizontalCenter: parent.horizontalCenter
                        id: treadmill_signal
                        width: 24
                        height: row.height - 76
                        source: rootItem.signal
                        smooth: true
                    }
                }
            }

            Rectangle {
                width: 120
                height: row.height
                color: Material.backgroundColor
                RoundButton {
                    icon.source: rootItem.startIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.startText
                    enabled: true
                    id: start
                    width: 120
                    height: row.height - 4
                }
                ColorOverlay {
                    anchors.fill: start
                    source: start
                    color: rootItem.startColor
                    enabled: rootItem.startColor === "red" ? true : false
                }
            }

            Rectangle {
                width: 120
                height: row.height
                color: Material.backgroundColor

                RoundButton {
                    icon.source: rootItem.stopIcon
                    icon.height: row.height - 54
                    icon.width: 46
                    text: rootItem.stopText
                    enabled: true
                    id: stop
                    width: 120
                    height: row.height - 4
                }
                ColorOverlay {
                    anchors.fill: stop
                    source: stop
                    color: rootItem.stopColor
                    enabled: rootItem.stopColor === "red" ? true : false
                }
            }

            Rectangle {
                id: item2
                width: 50
                height: row.height
                color: Material.backgroundColor
                RoundButton {
                    anchors.verticalCenter: parent.verticalCenter
                    id: lap
                    width: 48
                    height: row.height - 52
                    icon.source: "icons/icons/lap.png"
                    icon.width: 48
                    icon.height: 48
                    enabled: rootItem.lap
                    smooth: true
                }
                ColorOverlay {
                    anchors.fill: lap
                    source: lap
                    color: lap.enabled ? "#00000000" : "#B0D3d3d3"
                }
            }
        }

        Row {
            id: row1
            width: parent.width
            anchors.bottom: row.bottom
            anchors.bottomMargin: -10

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: rootItem.info
            }
        }

        Label {
            id: lblHelp
            width: parent.width
            anchors.top: row1.bottom
            anchors.topMargin: 30
            text: "This app should automatically connects to your bike/treadmill. <b>If it doesn't, please check</b>:<br>1) your Echelon/Domyos App MUST be closed while qdomyos-zwift is running;<br>2) bluetooth and bluetooth permission MUST be on<br>3) your bike/treadmill should be turned on BEFORE starting this app<br>4) try to restart your device<br><br>If your bike/treadmill disconnects every 30 seconds try to disable the 'virtual device' setting on the left bar.<br><br>In case of issue, please, feel free to contact me to roberto.viola83@gmail.com.<br><br><b>Have a nice ride!</b><br>Roberto Viola"
            wrapMode: Label.WordWrap
            visible: rootItem.labelHelp
        }
    }

    footer:
        ChartView {
            id: chartView
            backgroundColor: Material.backgroundColor
            margins.bottom: 0
            margins.left: 0
            margins.right: 0
            margins.top: 0
            plotAreaColor: Material.backgroundColor
            //title: "Spline"
            //anchors.top: gridView.bottom
            antialiasing: true
            legend.visible: false
            width: parent.width
            height: 130 * settings.ui_zoom / 100
            //Label.visible: false

            ValueAxis {
                id: valueAxisX
                min: 0
                max: 60
                tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }

            ValueAxis {
                id: valueAxisY1
                min: 0
                max: rootItem.wattMax
                //tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }
            ValueAxis {
                id: valueAxisY2
                min: rootItem.wattZ1Max
                max: rootItem.wattMax
                //tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }
            ValueAxis {
                id: valueAxisY3
                min: rootItem.wattZ2Max
                max: rootItem.wattMax
                //tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }
            ValueAxis {
                id: valueAxisY4
                min: rootItem.wattZ3Max
                max: rootItem.wattMax
                //tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }
            ValueAxis {
                id: valueAxisY5
                min: rootItem.wattZ4Max
                max: rootItem.wattMax
                //tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }
            ValueAxis {
                id: valueAxisY6
                min: rootItem.wattZ5Max
                max: rootItem.wattMax
                //tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }
            ValueAxis {
                id: valueAxisY7
                min: rootItem.wattZ6Max
                max: rootItem.wattMax
                //tickCount: 60
                labelFormat: "%.0f"
                labelsVisible: false
                gridVisible: false
                lineVisible: false
            }

            AreaSeries {
                id: series1
                useOpenGL: true
                axisX: valueAxisX
                axisY: valueAxisY1
                name: "Z1"
                color: "white"
                borderColor: color
                upperSeries: LineSeries {}
            }
            AreaSeries {
                id: series2
                useOpenGL: true
                axisX: valueAxisX
                axisY: valueAxisY2
                color: "limegreen"
                borderColor: color
                upperSeries: LineSeries {}
            }
            AreaSeries {
                id: series3
                useOpenGL: true
                axisX: valueAxisX
                axisY: valueAxisY3
                color: "gold"
                borderColor: color
                upperSeries: LineSeries {}
            }
            AreaSeries {
                id: series4
                useOpenGL: true
                axisX: valueAxisX
                axisY: valueAxisY4
                color: "orange"
                borderColor: color
                upperSeries: LineSeries {}
            }
            AreaSeries {
                id: series5
                useOpenGL: true
                axisX: valueAxisX
                axisY: valueAxisY5
                color: "darkorange"
                borderColor: color
                upperSeries: LineSeries {}
            }
            AreaSeries {
                id: series6
                useOpenGL: true
                axisX: valueAxisX
                axisY: valueAxisY6
                color: "orangered"
                borderColor: color
                upperSeries: LineSeries {}
            }
            AreaSeries {
                id: series7
                useOpenGL: true
                axisX: valueAxisX
                axisY: valueAxisY7
                color: "red"
                borderColor: color
                upperSeries: LineSeries {}
            }
        }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorZoom:0.6600000262260437;height:480;width:640}
}
##^##*/

