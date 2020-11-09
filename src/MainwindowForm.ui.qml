import QtQuick 2.4

Item {
    width: 400
    height: 400
    property alias textInput: textInput

    Grid {
        id: grid
        x: 0
        y: 0
        width: 400
        height: 400

        Text {
            id: text1
            text: qsTr("Speed")
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        TextInput {
            id: textInput
            width: 80
            height: 20
            text: qsTr("0.0")
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
