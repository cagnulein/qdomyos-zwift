import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import Qt.labs.settings 1.0

ColumnLayout {
    property Settings settings: null
    id: rootElement
    property  bool linkedBoolSettingDefault: false
    property string linkedBoolSetting: "example_setting"
    property bool isOpen: false
    property string title: ""
    default property alias accordionContent: contentPlaceholder.data
    spacing: 0

    function convertValue(val) {
        let tpval = typeof(val);
        if (tpval==="undefined") return false;
        else if (tpval==="string") return val === "true";
        else if (tpval==="number") return val !== 0;
        else if (tpval==="boolean") return val;
        else return false;

    }

    Component.onCompleted: function() {
        if (typeof(settings[linkedBoolSetting])=="undefined")
            isOpen = convertValue(settings.value(linkedBoolSetting, linkedBoolSettingDefault));
        else
            isOpen = convertValue(settings[linkedBoolSetting]);
    }
 
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
    IndicatorOnlySwitch {
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
            rootElement.isOpen = checked;
            if (typeof(settings[rootElement.linkedBoolSetting])=="undefined") {
                settings.setValue(rootElement.linkedBoolSetting, rootElement.convertValue(checked));
            }
            else {
                settings[rootElement.linkedBoolSetting] = rootElement.convertValue(checked);
            }
        }
    }
 
    // This will get filled with the content
    ColumnLayout {
        id: contentPlaceholder
        visible: rootElement.isOpen
        Layout.fillWidth: true;
    }
}
