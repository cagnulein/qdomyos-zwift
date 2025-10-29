import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0

Page {
    id: wizardPage
    objectName: "wizardPage"

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
        property bool wahoo_rgt_dircon: false
        property bool virtual_device_rower: false
        property real weight: 75.0
        property int age: 35
        property string sex: "Male"
        property bool miles_unit: false
        property string heart_rate_belt_name: "Disabled"
        property bool garmin_companion: false
        property string filter_device: "Disabled"
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
                        text: qsTr("QZ is designed to maximize your workout experience on a range of fitness equipment, including indoor bikes, treadmills, ellipticals, and rower. By connecting seamlessly with your devices, QZ provides realtime data, personalized workout plans, and interactive elements to keep you motivated.\n\nThe following questions will customize QZ for your equipment and goals.")
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
                                if (typeof window !== 'undefined' && window.goBack) { window.goBack(); }
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
                                if (modelData === "Rower") {
                                    settings.virtual_device_rower = true
                                } else {
                                    settings.virtual_device_rower = false
                                }

                                selectedOptions.step2 = modelData
                                stackViewLocal.push(bluetoothDeviceSelectionComponent)
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

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("QZ allows you to connect to both of them, even simultaneously if you want!")
                        font.pixelSize: 20
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                    }

                    GridLayout {
                        Layout.alignment: Qt.AlignHCenter
                        columns: 2
                        rowSpacing: 20
                        columnSpacing: 20
                        Repeater {
                            model: ["Peloton", "Zwift", "MyWhoosh", "Rouvy", "Kinomap", "Indievelo", "Fulgaz", "EXR", "Other app", "QZ Standalone"]
                            delegate: WizardButton {
                                Layout.preferredWidth: 150
                                text: modelData
                                onClicked: {
                                    selectedOptions.step3 = modelData
                                    if (modelData === "Peloton") {
                                        stackViewLocal.push(pelotonLoginComponent)
                                    } else {
                                        if(modelData === "Zwift") {
                                            settings.wahoo_rgt_dircon = false
                                        } else {
                                            settings.wahoo_rgt_dircon = true
                                        }
                                        stackViewLocal.push(zwiftComponent)
                                    }
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
                        text: qsTr("Connect to Peloton")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Click the button below to connect your Peloton account")
                        font.pixelSize: 20
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                    }

                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        source: "icons/icons/Button_Connect_Rect_DarkMode.png"
                        fillMode: Image.PreserveAspectFit
                        width: parent.width * 0.8

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                stackViewLocal.push("WebPelotonAuth.qml")
                                stackViewLocal.currentItem.goBack.connect(function() {
                                            stackViewLocal.pop();
                                            stackViewLocal.push(pelotonDifficultyComponent)
                                        })
                                peloton_connect_clicked()
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
                        text: qsTr("Typically, Peloton coaches call out a range for target incline, resistance and/or speed. Use this setting to choose the difficulty of the target QZ communicates. Difficulty level can be set to lower, upper or average")
                        font.pixelSize: 20
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: "white"
                        Layout.fillWidth: true
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

                    Item {
                        Layout.preferredHeight: 50
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
                        text: qsTr("QZ will read the inclination in real time from the Zwift app and will adjust the inclination on your treadmill. It doesn't work on workout")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
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

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.zwift_username = zwiftUsernameTextField.text;
                            settings.zwift_password = zwiftPasswordTextField.text;
                            settings.zwift_api_autoinclination = true;
                            stackViewLocal.push(finalStepComponent);
                        }
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

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Correct startup phase:\n\n1. close any app that can connect to your Zwift devices\n2. wake up your Zwift devices\n3. wake up your trainer\n4. open qz\n5. now if you change gear on your Zwift device you will see a reaction in the gear tile on qz and so on your trainer.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
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
                        onClicked: {
                            settings.tile_gears_enabled = true;
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
                            if (typeof window !== 'undefined' && window.goBack) { window.goBack(); }
                        }
                    }
                }
            }
        }
    }
    Component {
        id: bluetoothDeviceSelectionComponent

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
                        text: qsTr("Select Your Fitness Device")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    RowLayout {
                        spacing: 10
                        ComboBox {
                            id: filterDeviceTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.filter_device
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + filterDeviceTextField.currentIndex)
                                displayText = filterDeviceTextField.currentValue
                            }
                        }
                    }

                    WizardButton {
                        id: refreshFilterDeviceButton
                        text: "Refresh"
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.filter_device = filterDeviceTextField.displayText;
                            stackViewLocal.push(unitSelectionComponent)
                        }
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
        id: unitSelectionComponent

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
                        text: qsTr("Unit System")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select your preferred unit system")
                        font.pixelSize: 20
                        color: "white"
                    }

                    ComboBox {
                        id: unitSystemComboBox
                        Layout.alignment: Qt.AlignHCenter
                        model: ["Metric", "Imperial"]
                        currentIndex: settings.miles_unit ? 1 : 0
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.miles_unit = unitSystemComboBox.currentIndex === 1
                            stackViewLocal.push(userInfoComponent)
                        }
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
        id: userInfoComponent

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
                        text: qsTr("User Information")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Weight (" + (settings.miles_unit ? "lbs" : "kg") + ")")
                        font.pixelSize: 20
                        color: "white"
                    }

                    SpinBox {
                        id: weightSpinBox
                        Layout.alignment: Qt.AlignHCenter
                        from: settings.miles_unit ? 660 : 300  // 66.0 lbs or 30.0 kg
                        to: settings.miles_unit ? 4400 : 2000  // 440.0 lbs or 200.0 kg
                        value: settings.miles_unit ? (settings.weight * 2.20462 * 10).toFixed(0) : (settings.weight * 10)
                        stepSize: 1
                        editable: true

                        property real realValue: settings.miles_unit ? value / 22.0462 : value / 10

                        textFromValue: function(value, locale) {
                            return Number(value / 10).toLocaleString(locale, 'f', 1)
                        }

                        valueFromText: function(text, locale) {
                            return Number.fromLocaleString(locale, text) * 10
                        }

                        onValueChanged: {
                            settings.weight = realValue
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Age")
                        font.pixelSize: 20
                        color: "white"
                    }

                    SpinBox {
                        id: ageSpinBox
                        Layout.alignment: Qt.AlignHCenter
                        from: 1
                        to: 120
                        value: settings.age
                        editable: true
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Gender")
                        font.pixelSize: 20
                        color: "white"
                    }

                    ComboBox {
                        id: genderComboBox
                        Layout.alignment: Qt.AlignHCenter
                        model: ["Male", "Female"]
                        currentIndex: settings.sex === "Male" ? 0 : 1
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.weight = weightSpinBox.realValue
                            settings.age = ageSpinBox.value
                            settings.sex = genderComboBox.currentText
                            stackViewLocal.push(heartRateDeviceSelectionComponent)
                        }
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
        id: heartRateDeviceSelectionComponent

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
                        text: qsTr("Select Your Heart Rate Device")
                        font.pixelSize: 24
                        font.bold: true
                        color: "white"
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Choose your heart rate belt or select a smartwatch option:")
                        font.pixelSize: 20
                        color: "white"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 10
                        Layout.fillWidth: true

                        ComboBox {
                            id: heartBeltNameTextField
                            model: rootItem.bluetoothDevices
                            displayText: settings.heart_rate_belt_name
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + heartBeltNameTextField.currentIndex)
                                displayText = heartBeltNameTextField.currentValue
                            }
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        id: refreshHeartBeltNameButton
                        text: "Refresh"
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Or select a smartwatch option:")
                        font.pixelSize: 20
                        color: "white"
                        Layout.topMargin: 20
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Apple Watch")
                        onClicked: {
                            settings.heart_rate_belt_name = "Disabled"
                            settings.garmin_companion = false
                            stackViewLocal.push(step3Component)
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: "white"
                        Layout.bottomMargin: 20
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Wear OS watch")
                        onClicked: {
                            settings.heart_rate_belt_name = "Disabled"
                            settings.garmin_companion = false
                            stackViewLocal.push(step3Component)
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: "white"
                        Layout.bottomMargin: 20
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Garmin watch")
                        onClicked: {
                            settings.heart_rate_belt_name = "Disabled"
                            settings.garmin_companion = true
                            stackViewLocal.push(step3Component)
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: "white"
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.heart_rate_belt_name = heartBeltNameTextField.displayText;
                            settings.garmin_companion = false
                            stackViewLocal.push(step3Component)
                        }
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
}
