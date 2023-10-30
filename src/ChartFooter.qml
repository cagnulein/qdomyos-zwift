import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ColumnLayout {    
    anchors.fill: parent
    Loader {
        id: chartFooterLoader
        sourceComponent: ChartFooterInnerJS
        anchors.fill: parent
        active: false
    }

    Loader {
        anchors.fill: parent
        source: CHARTJS ? "ChartFooterInnerJS.qml":"ChartFooterInnerNoJS.qml"
        onLoaded: {
            if(CHARTJS) {
                chartFooterLoader.active = true;
            }
        }
    }
}
