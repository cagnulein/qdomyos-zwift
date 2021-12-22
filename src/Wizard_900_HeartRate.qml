import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import QtQuick.Window 2.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1
import QtQuick.Layouts 1.3

ScrollView {
    contentWidth: -1
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent

    Settings {
        property string heart_rate_belt_name: "Disabled"
    }

    background: Material.color

    ColumnLayout {
        id: column1
        spacing: 0
        anchors.fill: parent

        Label {
            id: welcome
            horizontalAlignment: Text.AlignHCenter
            topPadding: 20
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            text: "Heart Rate Sensor"
            font.pointSize: 30
        }

        Label {
            id: question
            topPadding: 20
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: welcome.bottom
            text: "Do you have an heart rate sensor?"
            font.pointSize: 18
        }

        Column {
            anchors.top: question.bottom
            topPadding: 20
            RadioButton {
                id: builtin
                text: qsTr("Yes, it's built-in in the machinery")
                onClicked: {
                    heartBeltNameTextField.enabled = false;
                    okHeartBeltNameButton.enabled = false;
                    refreshHeartBeltNameButton.enabled = false;
                    settings.heart_rate_belt_name = "Disabled";
                }
            }
            RadioButton {
                id: applewatch
                text: qsTr("Yes, I have an Apple Watch")
                onClicked: {
                    heartBeltNameTextField.enabled = false;
                    okHeartBeltNameButton.enabled = false;
                    refreshHeartBeltNameButton.enabled = false;
                    settings.heart_rate_belt_name = "Disabled";
                }
            }
            RadioButton {
                id: yes
                checked: true
                text: qsTr("Yes, it's bluetooth")
                onClicked: {
                    heartBeltNameTextField.enabled = true;
                    okHeartBeltNameButton.enabled = true;
                    refreshHeartBeltNameButton.enabled = true;
                }
            }
            ComboBox {
                id: heartBeltNameTextField
                model: rootItem.bluetoothDevices
                displayText: settings.heart_rate_belt_name
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onActivated: {
                    console.log("combomodel activated" + heartBeltNameTextField.currentIndex)
                    displayText = heartBeltNameTextField.currentValue
                    settings.heart_rate_belt_name = heartBeltNameTextField.displayText;
                }

            }
            Button {
                id: refreshHeartBeltNameButton
                text: "Refresh Devices List"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                onClicked: refresh_bluetooth_devices_clicked();
            }
            RadioButton {
                id: no
                text: qsTr("No")
                onClicked: {
                    heartBeltNameTextField.enabled = false;
                    okHeartBeltNameButton.enabled = false;
                    refreshHeartBeltNameButton.enabled = false;
                    settings.heart_rate_belt_name = "Disabled";
                }
            }
        }
        Button {
            anchors.leftMargin: 20
            anchors.bottomMargin: 20
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            text: "Back"
            onClicked: stackView.pop();
        }

        Button {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            text: "Skip"
            onClicked:  stackView.pop(stackView.find(function(item) {
                return item.objectName === "home";
            }));
        }

        Button {
            id: next
            anchors.rightMargin: 20
            anchors.bottomMargin: 20
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            text: "Next"
            onClicked: {
                stackView.push("Wizard_999_Finish.qml")
            }
        }
    }
}
