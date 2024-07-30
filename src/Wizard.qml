import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0

Page {
    id: wizardPage

    property int currentStep: 0
    property var selectedOptions: ({})

    Settings {
        id: settings
        property string peloton_username: "username"
        property string peloton_password: "password"
        property string peloton_difficulty: "lower"
        property int bike_resistance_offset: 4
        property string zwift_username: ""
        property string zwift_password: ""
        property bool speed_power_based: false
        property bool zwift_api_autoinclination: true
        property bool zwift_click: false
        property bool zwift_play: false
        property double gears_gain: 1.0
        property bool tile_gears_enabled: false
        property string theme_background_color: "#303030"
    }

    background: Rectangle {
        anchors.fill: parent
        width: parent.fill
        height: parent.fill
        color: settings.theme_background_color
    }

    StackView {
        id: stackViewLocal
        anchors.fill: parent

        initialItem: welcomeComponent
    }

    component WizardButton: Button {
             property color textColor: "#800080"
             property color backgroundColor: "white"
             background: Rectangle {
                 color: parent.backgroundColor
                 radius: 5
             }
             contentItem: Text {
                 font.pixelSize: 20
                 text: parent.text
                 color: parent.textColor
                 horizontalAlignment: Text.AlignHCenter
                 verticalAlignment: Text.AlignVCenter
             }
     }

    Component {
        id: welcomeComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        source: "qrc:/inner_templates/chartjs/qzlogo.png" // Replace with your logo path
                        width: 100
                        height: 100
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Welcome to QZ")
                        font.pixelSize: 28
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Created by Roberto Viola")
                        font.pixelSize: 24
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("QZ is a versatile app designed to maximize your workout experience on a range of fitness equipment, including indoor bikes, treadmills, ellipticals, and rowers. It offers comprehensive features that help you track your progress, enhance your training routines, and achieve your fitness goals more efficiently. By connecting seamlessly with your devices, QZ provides real-time data, personalized workout plans, and interactive elements to keep you motivated. Whether you're aiming to improve your endurance, build strength, or simply stay active, QZ ensures that you get the most out of every session on your indoor fitness equipment.")
                        font.pixelSize: 20
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Start")
                        onClicked: stackViewLocal.push(step1Component)
                    }
                }
            }
        }
    }

    Component {
        id: step1Component

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("How can I help you?")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("First-time setup")
                        onClicked: {
                            selectedOptions.step1 = "First-time setup"
                            stackViewLocal.push(step2Component)
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Help with a specific feature")
                        onClicked: {
                            selectedOptions.step1 = "Help with a specific feature"
                            stackViewLocal.push(step2HelpComponent)
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("I'm fine, thanks.")
                        onClicked: {
                                stackView.pop();
                        }
                    }
                }
            }
        }
    }

    Component {
        id: step2Component

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("What's your fitness device?")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Repeater {
                        model: ["Bike", "Treadmill", "Rower", "Elliptical"]
                        delegate: WizardButton {
                            Layout.alignment: Qt.AlignHCenter
                            text: qsTr(modelData)
                            onClicked: {
                                selectedOptions.step2 = modelData
                                stackViewLocal.push(step3Component)
                            }
                        }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: step3Component

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Choose your preferred app")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Repeater {
                        model: ["Peloton", "Zwift", "MyWhoosh"]
                        delegate: WizardButton {
                            Layout.alignment: Qt.AlignHCenter
                            text: modelData
                            onClicked: {
                                selectedOptions.step3 = modelData
                                if (modelData === "Peloton") {
                                    stackViewLocal.push(pelotonLoginComponent)
                                } else {
                                    stackViewLocal.push(zwiftComponent)
                                }
                            }
                        }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: pelotonLoginComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Peloton Login")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Username")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    TextField {
                        id: pelotonUsernameTextField
                        text: settings.peloton_username
                        horizontalAlignment: Text.AlignHCenter
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillHeight: false
                        onAccepted: settings.peloton_username = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Password")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    TextField {
                        id: pelotonPasswordTextField
                        text: settings.peloton_password
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignHCenter
                        inputMethodHints: Qt.ImhHiddenText
                        echoMode: TextInput.PasswordEchoOnEdit
                        onAccepted: settings.peloton_password = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.peloton_username = pelotonUsernameTextField.text;
                            settings.peloton_password = pelotonPasswordTextField.text;
                            stackViewLocal.push(pelotonDifficultyComponent)
                        }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: pelotonDifficultyComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Peloton Difficulty")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Typically, Peloton coaches call out a range for target incline, resistance and/or speed. Use this setting to choose the difficulty of the target QZ communicates. Difficulty level can be set to lower, upper or average")
                        font.pixelSize: 16
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Difficulty")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    ComboBox {
                        id: pelotonDifficultyTextField
                        model: [ "lower", "upper", "average" ]
                        displayText: settings.peloton_difficulty
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignHCenter
                        onActivated: {
                            console.log("combomodel activated" + pelotonDifficultyTextField.currentIndex)
                            displayText = pelotonDifficultyTextField.currentValue
                            settings.peloton_difficulty = pelotonDifficultyTextField.displayText;
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: {
                            settings.peloton_difficulty = pelotonDifficultyTextField.displayText;
                            stackViewLocal.push(finalStepComponent)
                        }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: zwiftComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Bike Resistance Level")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("What resistance level feels like a flat road on your bike?")
                        font.pixelSize: 20
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        Layout.fillWidth: true
                    }

                    SpinBox {
                        id: spinBoxResistanceOffset
                        Layout.alignment: Qt.AlignHCenter
                        from: 0
                        to: 100
                        value: 18
                        editable: true
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.bike_resistance_offset = spinBoxResistanceOffset.value;
                            settings.speed_power_based = true;
                            stackViewLocal.push(finalStepComponent);
                        }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: step4Component

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Custom Configurations")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Here you will see custom configurations based on your previous choices.")
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.8
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: stackViewLocal.push(finalStepComponent)
                    }
                }
            }
        }
    }

    Component {
        id: step2HelpComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select a feature")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Repeater {
                        model: [
                            qsTr("Auto-incline with treadmill and Zwift"),
                            qsTr("Auto-resistance with Peloton"),
                            qsTr("Zwift Click or Zwift Play"),
                            qsTr("Virtual Shifting")
                        ]
                        delegate: WizardButton {
                            Layout.alignment: Qt.AlignHCenter
                            text: modelData
                            onClicked: {
                                selectedOptions.step2Help = modelData
                                if(modelData === "Auto-incline with treadmill and Zwift")
                                    stackViewLocal.push(zwiftAutoInclination)
                                else if(modelData === "Auto-resistance with Peloton")
                                    stackViewLocal.push(pelotonLoginComponent)
                                else if(modelData === "Zwift Click or Zwift Play")
                                    stackViewLocal.push(zwiftPlayClick)
                                else if(modelData === "Virtual Shifting")
                                    stackViewLocal.push(virtualShifting)
                                else
                                    stackViewLocal.push(step3HelpComponent)
                            }
                        }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: zwiftAutoInclination

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Zwift Credentials")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Username")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    TextField {
                        id: zwiftUsernameTextField
                        text: settings.zwift_username
                        horizontalAlignment: Text.AlignHCenter
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillHeight: false
                        onAccepted: settings.zwift_username = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Password")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    TextField {
                        id: zwiftPasswordTextField
                        text: settings.zwift_password
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignHCenter
                        inputMethodHints: Qt.ImhHiddenText
                        echoMode: TextInput.PasswordEchoOnEdit
                        onAccepted: settings.zwift_password = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.zwift_username = zwiftUsernameTextField.text;
                            settings.zwift_password = zwiftPasswordTextField.text;
                            settings.zwift_api_autoinclination = true;
                            // Here you would typically handle the login process
                            // For now, we'll just move to the next step
                            stackViewLocal.push(finalStepComponent);
                        }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: zwiftPlayClick

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 24
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Zwift Play and Click")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        id: zwiftPlayClickDescription
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Enable the one that you would like to use directly with QZ. Remember to update their firmware before using it.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                    }

                    SwitchDelegate {
                        text: qsTr("Zwift Click")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        font.pixelSize: 20
                        clip: false
                        width: zwiftPlayClickDescription.width
                        checked: settings.zwift_click
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_click = checked;  }
                    }

                    SwitchDelegate {
                        text: qsTr("Zwift Play")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        font.pixelSize: 20
                        width: zwiftPlayClickDescription.width
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_play
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_play = checked; }
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: {
                            settings.tile_gears_enabled = true;
                            settings.gears_gain = 0.5;
                            stackViewLocal.push(finalStepComponent);
                        }
                    }
                }
            }
        }
    }

    Component {
        id: virtualShifting

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Virtual Shifting")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Virtual shifting enabled! You can change gears using the gears tile in QZ directly, or you can also add a bluetooth remote or a Zwift Play or a Zwift Click to control it!")
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        Layout.fillWidth: true
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: {
                            settings.tile_gears_enabled = true;
                            settings.gears_gain = 1;
                            stackViewLocal.push(finalStepComponent);
                        }
                    }
                }
            }
        }
    }

    Component {
        id: step3HelpComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Custom Configurations")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Here you will see custom configurations based on the selected feature.")
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: stackViewLocal.push(finalStepComponent)
                    }
                }
            }
        }
    }

    Component {
        id: finalStepComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: -1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Thank you for setting up QZ!")
                        font.pixelSize: 20
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("If you have any questions or need further assistance, feel free to write to me at roberto.viola83@gmail.com. You can also restart this wizard from the left side bar menu. To apply some changes, you may need to restart the app.")
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        Layout.fillWidth: true
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                Qt.openUrlExternally("mailto:roberto.viola83@gmail.com")
                            }
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Close")
                        onClicked: {
                            stackView.pop();
                        }
                    }
                }
            }
        }
    }
}
