import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0

ColumnLayout {
    property Settings settings: null
    id: rootElement
    property bool invert: false
    property  bool linkedBoolSettingDefault: false
    property string linkedBoolSetting: "example_setting"
    property bool isOpen: invert ? !settings[linkedBoolSetting]:settings[linkedBoolSetting]
    property string title: ""
    default property alias accordionContent: contentPlaceholder.data
    spacing: 0
 
    Layout.fillWidth: true;
 
    /*RowLayout {
        id: accordionHeader
        Layout.alignment: Qt.AlignTop
        Layout.fillWidth: true;
 
        Rectangle{
           id:indicatRect
           Layout.alignment: Qt.AlignLeft | Qt.AlignTop
           width: 8; height: 8
           radius: 8
           color: "white"
        }
 
        Text {
            id: accordionText
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            color: "#FFFFFF"
            text: rootElement.title
        }

    }*/
    SwitchDelegate {
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        Layout.fillWidth: true
        spacing: 0
        bottomPadding: 0
        topPadding: 0
        rightPadding: 0
        leftPadding: 0
        clip: false
        id: indicatCbx
        text: rootElement.title
        checked: rootElement.isOpen
        onClicked: {
            rootElement.isOpen = checked
            settings[rootElement.linkedBoolSetting] = rootElement.invert? !checked:checked
        }
    }
 
    // This will get filled with the content
    ColumnLayout {
        id: contentPlaceholder
        visible: rootElement.isOpen
        Layout.fillWidth: true;
    }
}
