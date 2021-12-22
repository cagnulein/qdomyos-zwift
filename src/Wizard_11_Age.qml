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
        text: "Age"
        font.pointSize: 30
    }

    Label {
        id: question
        topPadding: 20
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: welcome.bottom
        text: "Simple question: how old are you?"
        font.pointSize: 18
    }

    TextField {
        anchors.top: question.bottom
        id: ageTextField
        text: settings.age
        horizontalAlignment: Text.AlignHCenter
        inputMethodHints: Qt.ImhFormattedNumbersOnly
        onTextChanged: settings.age = ageTextField.text
        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
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
            stackView.push("Wizard_20_MachineryType.qml")
        }
    }
}
