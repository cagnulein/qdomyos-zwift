import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ScrollView {
    contentWidth: -1
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent
    clip: true

    property string defaultInclinationResistanceTable: "0|4\n1|6\n2|8\n3|10\n4|11\n5|11.5\n6|12\n8|13\n10|14\n12|15\n15|16"

    Settings {
        id: settings
        property bool custom_inclination_resistance_table_enabled: false
        property string custom_inclination_resistance_table: defaultInclinationResistanceTable
    }

    Component.onCompleted: {
        inclinationResistanceTable.text = settings.custom_inclination_resistance_table
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Label {
            text: qsTr("Custom Inclination to Resistance Table")
            font.bold: true
            Layout.fillWidth: true
        }

        IndicatorOnlySwitch {
            text: qsTr("Enable Custom Inclination to Resistance Table")
            spacing: 0
            bottomPadding: 0
            topPadding: 0
            rightPadding: 0
            leftPadding: 0
            clip: false
            checked: settings.custom_inclination_resistance_table_enabled
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            Layout.fillWidth: true
            onClicked: settings.custom_inclination_resistance_table_enabled = checked
        }

        Label {
            text: qsTr("Used when QZ converts a virtual-app inclination into a resistance command. Enter one inclination|resistance pair per line. QZ interpolates between points and uses the nearest endpoint outside the configured range. The resistance value is the final target level, so the normal resistance gain/offset conversion is bypassed while this table is enabled.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            color: Material.accent
        }

        Label {
            text: qsTr("Example: 6|12 means that a 6% grade targets resistance level 12. Decimal values are allowed.")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: Qt.application.font.pixelSize - 2
            color: Material.accent
        }

        TextArea {
            id: inclinationResistanceTable
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            selectByMouse: true
            wrapMode: TextEdit.NoWrap
            placeholderText: "0|4\n1|6\n2|8\n..."
            background: Rectangle {
                color: "white"
                border.color: "#cccccc"
                radius: 2
            }
            color: "black"
            selectedTextColor: "white"
            selectionColor: Material.accent
        }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: qsTr("Save Table")
                Layout.fillWidth: true
                onClicked: settings.custom_inclination_resistance_table = inclinationResistanceTable.text
            }

            Button {
                text: qsTr("Reset Example")
                Layout.fillWidth: true
                onClicked: {
                    inclinationResistanceTable.text = defaultInclinationResistanceTable
                    settings.custom_inclination_resistance_table = defaultInclinationResistanceTable
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
