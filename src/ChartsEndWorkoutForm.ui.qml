import QtQuick 2.4
import QtCharts 2.2
import Qt.labs.settings 1.0
import QtQuick.Controls 2.15

Item {
    anchors.fill: parent

    property alias powerSeries: powerSeries
    property alias powerChart: powerChart
    property alias heartSeries: heartSeries
    property alias heartChart: heartChart
    property alias cadenceSeries: cadenceSeries
    property alias cadenceChart: cadenceChart

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
/*
        ChartView {
            id: caloriesChart
            title: "KCal"
            anchors.left: parent.left
            anchors.top: parent.top
            legend.visible: false
            antialiasing: true
            width: 64
            height: 64
            margins.bottom: 1
            margins.top: 1
            margins.left: 1
            margins.right: 1

            Text {
                anchors.topMargin: 10
                anchors.fill: caloriesChart
                text: rootItem.Pcalories
                font.pixelSize: 16
                color: "black"
                z: 1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: cadenceChart.horizontalCenter
            }

            PieSeries {
                size: 1.0
                holeSize: 0.7
                PieSlice { value: 1.0; color: "red" }
            }
        }

        ChartView {
            id: outputChart
            title: "KJoul"
            anchors.left: caloriesChart.right
            anchors.top: parent.top
            legend.visible: false
            antialiasing: true
            width: 64
            height: 64
            margins.bottom: 1
            margins.top: 1
            margins.left: 1
            margins.right: 1

            Text {
                anchors.topMargin: 10
                anchors.fill: outputChart
                text: rootItem.Pkjoules
                font.pixelSize: 16
                color: "black"
                z: 1
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: cadenceChart.horizontalCenter
            }

            PieSeries {
                size: 1.0
                holeSize: 0.7
                PieSlice { value: 1.0; color: "yellow" }
            }
        }*/

        ScrollView {
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: title.bottom
            anchors.bottom: parent.bottom
            contentHeight: powerChart.height+heartChart.height+cadenceChart.height

            ChartView {
                id: powerChart
                antialiasing: true
                legend.visible: false
                height: 400
                width: parent.width
                title: "Power"
                titleFont.pixelSize: 20

                ValueAxis {
                    id: valueAxisX
                    min: 0
                    max: rootItem.workout_sample_points
                    //tickCount: 60
                    labelFormat: "%.0f"
                    //labelsVisible: false
                    gridVisible: false
                    //lineVisible: false
                }

                ValueAxis {
                    id: valueAxisY
                    min: 0
                    max: (settings.ftp * 2)
                    //tickCount: 60
                    labelFormat: "%.0f"
                    //labelsVisible: false
                    gridVisible: false
                    //lineVisible: false
                }

                LineSeries {
                    //name: "Power"
                    id: powerSeries
                    visible: true
                    axisX: valueAxisX
                    axisY: valueAxisY
                    color: "white"
                    width: 3
                }
            }

            ChartView {
                id: heartChart
                height: 400
                width: parent.width
                antialiasing: true
                legend.visible: false
                anchors.top: powerChart.bottom
                title: "Heart Rate"
                titleFont.pixelSize: 20

                ValueAxis {
                    id: valueAxisXHR
                    min: 0
                    max: rootItem.workout_sample_points
                    //tickCount: 60
                    labelFormat: "%.0f"
                    //labelsVisible: false
                    gridVisible: false
                    //lineVisible: false
                }

                ValueAxis {
                    id: valueAxisYHR
                    min: 0
                    max: 220
                    //tickCount: 60
                    labelFormat: "%.0f"
                    //labelsVisible: false
                    gridVisible: false
                    //lineVisible: false
                }

                LineSeries {
                    //name: "Power"
                    id: heartSeries
                    visible: true
                    axisX: valueAxisXHR
                    axisY: valueAxisYHR
                    color: "white"
                    width: 3
                }
            }


            ChartView {
                id: cadenceChart
                height: 400
                width: parent.width
                antialiasing: true
                legend.visible: false
                anchors.top: heartChart.bottom
                title: "Cadence"
                titleFont.pixelSize: 20

                ValueAxis {
                    id: valueAxisXCadence
                    min: 0
                    max: rootItem.workout_sample_points
                    //tickCount: 60
                    labelFormat: "%.0f"
                    //labelsVisible: false
                    gridVisible: false
                    //lineVisible: false
                }

                ValueAxis {
                    id: valueAxisYCadence
                    min: 0
                    max: 200
                    //tickCount: 60
                    labelFormat: "%.0f"
                    //labelsVisible: false
                    gridVisible: false
                    //lineVisible: false
                }

                LineSeries {
                    //name: "Power"
                    id: cadenceSeries
                    visible: true
                    axisX: valueAxisXCadence
                    axisY: valueAxisYCadence
                    color: "white"
                    width: 3
                }
            }
        }
    }
}
