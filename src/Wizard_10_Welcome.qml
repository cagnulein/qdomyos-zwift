import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import QtQuick.Window 2.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1

Page {
    background: Material.color

    Label {
        id: welcome
        horizontalAlignment: Text.AlignHCenter
        topPadding: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        text: "Welcome to QZ Fitness!"
        font.pointSize: 30
    }

    Label {
        topPadding: 20
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcome.bottom
        text: "I will help you in the\nsetup process of the app\n\nI just need to answer some\nquestions."
        font.pointSize: 18
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        text: "Skip"
        onClicked: stackView.pop();
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: "Next"
        onClicked: stackView.push("Wizard_20_MachineryType.qml");
    }
}
