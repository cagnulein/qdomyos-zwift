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
        text: "Everything is done!"
        font.pointSize: 30
    }

    Label {
        topPadding: 20
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcome.bottom
        text: "Just click finish and start to use QZ"
        font.pointSize: 18
    }

    Button {
        anchors.leftMargin: 20
        anchors.bottomMargin: 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        text: "Skip"
        onClicked: stackView.pop();
    }

    Button {
        anchors.rightMargin: 20
        anchors.bottomMargin: 20
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: "Finish"
        onClicked:  stackView.pop(stackView.find(function(item) {
            return item.objectName === "home";
        }));
    }
}
