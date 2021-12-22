import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.12
import QtQuick.Window 2.12
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1

Page {
    Settings{
        property string wizard_machinery_type: "bike"
    }

    background: Material.color

    Label {
        id: welcome
        horizontalAlignment: Text.AlignHCenter
        topPadding: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        text: "Fitness Machine"
        font.pointSize: 30
    }

    Label {
        id: question
        topPadding: 20
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcome.bottom
        text: "Which Fitness Machine do you have\ndo you use most?"
        font.pointSize: 18
    }

    Column {
        anchors.top: question.bottom
        topPadding: 20
        RadioButton {
            id: bike
            checked: true
            text: qsTr("Spin bike")
        }
        RadioButton {
            id: treadmill
            text: qsTr("Treadmill")
        }
        RadioButton {
            id: rower
            text: qsTr("Rower")
        }
        RadioButton {
            id: elliptical
            text: qsTr("Elliptical")
        }
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        text: "Skip"
        onClicked: stackView.pop();
    }

    Button {
        id: next
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        text: "Next"
        onClicked: {
            if(bike.checked == false) {
                stackView.push("Wizard_999_Finish.qml")
            }
        }
    }
}
