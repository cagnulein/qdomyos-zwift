import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0

Page {
    id: wizardPage
    objectName: "wizardPage"

    property int currentStep: 0
    property var selectedOptions: ({})

    // Called by main.qml on the Android back button: go back one wizard step
    function handleBack() {
        if (stackViewLocal.depth > 1) {
            stackViewLocal.pop()
            return true
        }
        return false
    }

    // Strip the RSSI proximity suffix (e.g. " (75%)") before saving device names
    function stripRssi(deviceName) {
        return deviceName.replace(/ \(\d+%\)$/, "")
    }

    Settings {
        id: settings
        Component.onCompleted: window.retakeSettingsSnapshot(settings)
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
        property bool weight_kg_unit: false
    }

    // The heart rate sensor bar saves its Connect from C++, and this copy does not read QSettings again:
    // without this, Next on the heart rate step would save the old "Disabled" over it
    Connections {
        target: rootItem
        function onHeartRateSensorChosenFromOffer(name) { settings.heart_rate_belt_name = name }
    }

    background: Rectangle {
        anchors.fill: parent
        width: parent.fill
        height: parent.fill
        color: window.ui.modern ? window.ui.bg : settings.theme_background_color
    }

    // Modern look: progress along the wizard as a thin bar on top. The number of steps
    // depends on the answers: an estimate of ten steps while the branch is open, one step
    // left on a step with Finish (wizardLastStep), full on the last page (wizardDone). A
    // short branch (a feature, virtual shifting) ended at half the bar before
    Rectangle {
        visible: window.ui.modern
        z: 1
        width: parent.width
        height: 3
        color: window.ui.surfaceHigh
        Rectangle {
            height: parent.height
            radius: 1.5
            readonly property Item page: stackViewLocal.currentItem
            readonly property int depth: stackViewLocal.depth
            width: parent.width * (page && page.wizardDone ? 1
                                   : page && page.wizardLastStep ? depth / (depth + 1)
                                   : Math.min(0.9, depth / 10))
            color: window.ui.accent
            Behavior on width { NumberAnimation { duration: 200; easing.type: Easing.OutQuad } }
        }
    }

    StackView {
        id: stackViewLocal
        anchors.fill: parent
        anchors.topMargin: window.ui.modern ? 3 : 0

        initialItem: welcomeComponent
    }

    component WizardButton: Button {
             id: wizardButton
             property color textColor: "#800080"
             property color backgroundColor: "white"
             // Modern look, answers only: an icon on the left and a line of explanation
             // under the label; the classic look shows the label alone, as before
             property string iconName: ""
             property string subtitle: ""
             // Modern look: "primary" moves on (Next, Finish), "secondary" goes back,
             // "choice" is an answer drawn as a full-width card
             readonly property string kind: (text === qsTr("Back") || text === qsTr("Refresh")) ? "secondary"
                                          : (text === qsTr("Next") || text === qsTr("Finish") || text === qsTr("Start") || text === qsTr("Close")) ? "primary"
                                          : "choice"
             // A translated label can be wider than the screen: cap the button and wrap the text.
             // Capped by the screen, not by parent.width: in a GridLayout the parent's width comes
             // from the buttons themselves, and the loop squeezed them on Android
             Layout.maximumWidth: window.ui.modern ? Math.min(480, stackViewLocal.width * 0.9) : stackViewLocal.width * 0.8
             Layout.fillWidth: window.ui.modern && kind === "choice"
             Layout.minimumWidth: window.ui.modern && kind !== "choice" ? 200 : 0
             // Classic: the stock Button formula
             implicitHeight: window.ui.modern ? Math.max(kind === "choice" ? 60 : 52, implicitContentHeight + 24)
                                              : Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)
             // UiFrame, not Rectangle.border: a thin border breaks up on Android
             background: UiFrame {
                 fill: !window.ui.modern ? wizardButton.backgroundColor
                      : wizardButton.kind === "primary" ? (wizardButton.down ? Qt.darker(window.ui.accent, 1.15) : window.ui.accent)
                      : wizardButton.kind === "secondary" ? (wizardButton.down ? window.ui.surfaceHigh : "transparent")
                      : (wizardButton.down ? window.ui.surfaceHighest : window.ui.surfaceHigh)
                 radius: !window.ui.modern ? 5 : wizardButton.kind === "choice" ? 16 : height / 2
                 strokeWidth: window.ui.modern && wizardButton.kind !== "primary" ? 1 : 0
                 stroke: wizardButton.kind === "choice" ? window.ui.alpha(window.ui.textMain, 0.08) : window.ui.outline
             }
             contentItem: Item {
                 readonly property bool rich: window.ui.modern && wizardButton.kind === "choice" && wizardButton.iconName !== ""
                 implicitWidth: rich ? richRow.implicitWidth : plainText.implicitWidth
                 implicitHeight: rich ? richRow.implicitHeight : plainText.implicitHeight

                 Text {
                     id: plainText
                     anchors.fill: parent
                     visible: !parent.rich
                     font.pixelSize: window.ui.modern ? window.ui.fontTitleSmall : window.fontPx(20)
                     font.weight: window.ui.modern ? Font.Medium : Font.Normal
                     text: wizardButton.text
                     color: !window.ui.modern ? wizardButton.textColor
                          : wizardButton.kind === "primary" ? window.ui.accentInk : window.ui.textMain
                     wrapMode: Text.WordWrap
                     horizontalAlignment: Text.AlignHCenter
                     verticalAlignment: Text.AlignVCenter
                 }

                 Row {
                     id: richRow
                     visible: parent.rich
                     anchors.verticalCenter: parent.verticalCenter
                     width: parent.width
                     spacing: 14

                     Rectangle {
                         anchors.verticalCenter: parent.verticalCenter
                         width: 40
                         height: 40
                         radius: 20
                         color: window.ui.alpha(window.ui.accent, 0.14)
                         UiIcon {
                             anchors.centerIn: parent
                             width: 22
                             height: 22
                             name: wizardButton.iconName !== "" ? wizardButton.iconName : "help"
                             color: window.ui.accent
                         }
                     }

                     Column {
                         anchors.verticalCenter: parent.verticalCenter
                         width: parent.width - 54
                         spacing: 2
                         Text {
                             width: parent.width
                             text: wizardButton.text
                             font.pixelSize: window.ui.modern ? window.ui.fontTitleSmall : 17
                             font.weight: Font.Medium
                             color: window.ui.textMain
                             wrapMode: Text.WordWrap
                         }
                         Text {
                             width: parent.width
                             visible: wizardButton.subtitle !== ""
                             text: wizardButton.subtitle
                             font.pixelSize: window.ui.modern ? window.ui.fontLabel : 14
                             color: window.ui.textMuted
                             wrapMode: Text.WordWrap
                         }
                     }
                 }
             }
     }

    Component {
        id: welcomeComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        source: "qrc:/inner_templates/chartjs/qzlogo.png" // Replace with your logo path
                        width: 100
                        height: 100
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Welcome to QZ")
                        font.pixelSize: window.ui.fontPx(28)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Created by Roberto Viola")
                        font.pixelSize: window.ui.fontPx(24)
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("QZ is designed to maximize your workout experience on a range of fitness equipment, including indoor bikes, treadmills, ellipticals, and rower. By connecting seamlessly with your devices, QZ provides realtime data, personalized workout plans, and interactive elements to keep you motivated.\n\nThe following questions will customize QZ for your equipment and goals.")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("How can I help you?")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("First-time setup")
                        iconName: "rocket_launch"
                        subtitle: qsTr("Your equipment, units and profile, step by step")
                        onClicked: {
                            selectedOptions.step1 = "First-time setup"
                            stackViewLocal.push(step2Component)
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Help with a specific feature")
                        iconName: "help"
                        subtitle: qsTr("Pick a topic and go straight to its settings")
                        onClicked: {
                            selectedOptions.step1 = "Help with a specific feature"
                            stackViewLocal.push(step2HelpComponent)
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("I'm fine, thanks.")
                        iconName: "check_circle"
                        subtitle: qsTr("Close the wizard")
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("What's your fitness device?")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Repeater {
                        model: [QT_TR_NOOP("Bike"), QT_TR_NOOP("Treadmill"), QT_TR_NOOP("Rower"), QT_TR_NOOP("Elliptical")]
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Choose your preferred app")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("QZ allows you to connect to both of them, even simultaneously if you want!")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    GridLayout {
                        Layout.alignment: Qt.AlignHCenter
                        columns: 2
                        rowSpacing: 20
                        columnSpacing: 20
                        Repeater {
                            model: ["Peloton", "Zwift", "MyWhoosh", "Rouvy", "Kinomap", "Indievelo", "Fulgaz", "EXR", QT_TR_NOOP("Other app"), QT_TR_NOOP("QZ Standalone")]
                            delegate: WizardButton {
                                // a bigger text: wider buttons, two of them still fit the screen
                                Layout.preferredWidth: window.textScale !== 1 ? Math.min(window.fontPx(150), (stackViewLocal.width * 0.9 - 20) / 2) : 150
                                text: qsTr(modelData)
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Connect to Peloton")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Click the button below to connect your Peloton account")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
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

                    // Modern look: go on without signing in now (already connected, or later)
                    WizardButton {
                        visible: window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: stackViewLocal.push(pelotonDifficultyComponent)
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
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Peloton Difficulty")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        text: qsTr("Typically, Peloton coaches call out a range for target incline, resistance and/or speed. Use this setting to choose the difficulty of the target QZ communicates. Difficulty level can be set to lower, upper or average")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.fillWidth: true
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Difficulty")
                        font.pixelSize: window.ui.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    ValueComboBox {
                        id: pelotonDifficultyTextField
                        model: [ "lower", "upper", "average" ]
                        labels: ({ "lower": qsTr("lower"), "upper": qsTr("upper"), "average": qsTr("average") })
                        value: settings.peloton_difficulty
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignHCenter
                        onActivated: {
                            console.log("combomodel activated" + pelotonDifficultyTextField.currentIndex)
                            settings.peloton_difficulty = pelotonDifficultyTextField.currentValue;
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: {
                            settings.peloton_difficulty = pelotonDifficultyTextField.value;
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Bike Resistance Level")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("What resistance level feels like a flat road on your bike?")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.fillWidth: true
                    }

                    UiSpinBox {
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
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Custom Configurations")
                        font.pixelSize: window.ui.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Here you will see custom configurations based on your previous choices.")
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.8
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    // Classic: Back above Finish, as before; modern: the step goes on first, like
                    // Next on the other steps, and Back under it
                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: stackViewLocal.push(finalStepComponent)
                    }

                    WizardButton {
                        visible: window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select a feature")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Repeater {
                        // Untranslated keys: onClicked compares against them, so the model must not go through qsTr()
                        model: [
                            QT_TR_NOOP("Auto-incline with treadmill and Zwift"),
                            QT_TR_NOOP("Auto-resistance with Peloton"),
                            QT_TR_NOOP("Zwift Click or Zwift Play"),
                            QT_TR_NOOP("Virtual Shifting")
                        ]
                        delegate: WizardButton {
                            Layout.alignment: Qt.AlignHCenter
                            text: qsTr(modelData)
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Zwift Credentials")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        // a plain Text does not inherit the window font: the text size while it is not 100 %
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("QZ will read the inclination in real time from the Zwift app and will adjust the inclination on your treadmill. It doesn't work on workout")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.ink("white")
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Username")
                        font.pixelSize: window.ui.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    UiTextField {
                        id: zwiftUsernameTextField
                        text: settings.zwift_username
                        horizontalAlignment: Text.AlignHCenter
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillHeight: false
                        onAccepted: settings.zwift_username = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Password")
                        font.pixelSize: window.ui.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    UiTextField {
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
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 24
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Zwift Play and Click")
                        font.pixelSize: window.ui.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        id: zwiftPlayClickDescription
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Enable the one that you would like to use directly with QZ. Remember to update their firmware before using it.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.ink("white")
                    }

                    UiSwitchDelegate {
                        text: qsTr("Zwift Click")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        font.pixelSize: window.ui.fontPx(20)
                        clip: false
                        width: zwiftPlayClickDescription.width
                        checked: settings.zwift_click
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_click = checked;  }
                    }

                    UiSwitchDelegate {
                        text: qsTr("Zwift Play")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        font.pixelSize: window.ui.fontPx(20)
                        width: zwiftPlayClickDescription.width
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_play
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_play = checked; }
                    }

                    Text {
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Correct startup phase:\n\n1. close any app that can connect to your Zwift devices\n2. wake up your Zwift devices\n3. wake up your trainer\n4. open qz\n5. now if you change gear on your Zwift device you will see a reaction in the gear tile on qz and so on your trainer.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.ink("white")
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    // Classic: Back above Finish, as before; modern: the step goes on first, like
                    // Next on the other steps, and Back under it
                    WizardButton {
                        visible: !window.ui.modern
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

                    WizardButton {
                        visible: window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: virtualShifting

        Item {
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Virtual Shifting")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Virtual shifting enabled! You can change gears using the gears tile in QZ directly, or you can also add a bluetooth remote or a Zwift Play or a Zwift Click to control it!")
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.ink("white")
                        Layout.fillWidth: true
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    // Classic: Back above Finish, as before; modern: the step goes on first, like
                    // Next on the other steps, and Back under it
                    WizardButton {
                        visible: !window.ui.modern
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

                    WizardButton {
                        visible: window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: step3HelpComponent

        Item {
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Custom Configurations")
                        font.pixelSize: window.ui.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Here you will see custom configurations based on the selected feature.")
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.ink("white")
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    // Classic: Back above Finish, as before; modern: the step goes on first, like
                    // Next on the other steps, and Back under it
                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: stackViewLocal.push(finalStepComponent)
                    }

                    WizardButton {
                        visible: window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }
        }
    }

    Component {
        id: finalStepComponent

        Item {
            readonly property bool wizardDone: true
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Thank you for setting up QZ!")
                        font.pixelSize: window.ui.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("If you have any questions or need further assistance, feel free to write to me at roberto.viola83@gmail.com. You can also restart this wizard from the left side bar menu. To apply some changes, you may need to restart the app.")
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.ink("white")
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
    Component {
        id: bluetoothDeviceSelectionComponent

        Item {
            anchors.fill: parent
            ScrollView {
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select Your Fitness Device")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    RowLayout {
                        spacing: 10
                        ValueComboBox {
                            id: filterDeviceTextField
                            model: rootItem.bluetoothDevices
                            labels: ({ "Disabled": qsTr("Disabled") })
                            value: settings.filter_device
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + filterDeviceTextField.currentIndex)
                            }
                        }
                    }

                    WizardButton {
                        id: refreshFilterDeviceButton
                        text: qsTr("Refresh")
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
                            settings.filter_device = stripRssi(filterDeviceTextField.value);
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Unit System")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select your preferred unit system")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    UiComboBox {
                        id: unitSystemComboBox
                        Layout.alignment: Qt.AlignHCenter
                        model: [qsTr("Metric"), qsTr("Imperial")]
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("User Information")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Weight (%1)").arg((settings.miles_unit && !settings.weight_kg_unit) ? qsTr("lbs") : qsTr("kg"))
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    UiSpinBox {
                        id: weightSpinBox
                        Layout.alignment: Qt.AlignHCenter
                        from: (settings.miles_unit && !settings.weight_kg_unit) ? 660 : 300  // 66.0 lbs or 30.0 kg
                        to: (settings.miles_unit && !settings.weight_kg_unit) ? 4400 : 2000  // 440.0 lbs or 200.0 kg
                        value: (settings.miles_unit && !settings.weight_kg_unit) ? (settings.weight * 2.20462 * 10).toFixed(0) : (settings.weight * 10)
                        stepSize: 1
                        editable: true

                        property real realValue: weightOf(value)

                        function weightOf(v) {
                            return (settings.miles_unit && !settings.weight_kg_unit) ? v / 22.0462 : v / 10
                        }

                        textFromValue: function(value, locale) {
                            return Number(value / 10).toLocaleString(locale, 'f', 1)
                        }

                        valueFromText: function(text, locale) {
                            return Number.fromLocaleString(locale, text) * 10
                        }

                        // From the new value itself: realValue may not be updated yet when this runs,
                        // the old weight went to the settings and the binding of value took the field
                        // back, so every second tap of +/- was lost
                        onValueChanged: {
                            settings.weight = weightOf(value)
                        }
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Age")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    UiSpinBox {
                        id: ageSpinBox
                        Layout.alignment: Qt.AlignHCenter
                        from: 1
                        to: 120
                        value: settings.age
                        editable: true
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Gender")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    ValueComboBox {
                        id: genderComboBox
                        Layout.alignment: Qt.AlignHCenter
                        model: ["Male", "Female"]
                        labels: ({ "Male": qsTr("Male"), "Female": qsTr("Female") })
                        currentIndex: settings.sex === "Male" ? 0 : 1
                        value: settings.sex === "Male" ? "Male" : "Female"
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
                            settings.sex = genderComboBox.value
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
                contentWidth: availableWidth
                topPadding: 16
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    width: parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select Your Heart Rate Device")
                        font.pixelSize: window.ui.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Choose your heart rate belt or select a smartwatch option:")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 10
                        Layout.fillWidth: true

                        ValueComboBox {
                            id: heartBeltNameTextField
                            model: rootItem.bluetoothDevices
                            labels: ({ "Disabled": qsTr("Disabled") })
                            value: settings.heart_rate_belt_name
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + heartBeltNameTextField.currentIndex)
                            }
                        }
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        id: refreshHeartBeltNameButton
                        text: qsTr("Refresh")
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Or select a smartwatch option:")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
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
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: window.ui.ink("white")
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
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: window.ui.ink("white")
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
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: window.ui.ink("white")
                    }

                    Item {
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: {
                            settings.heart_rate_belt_name = stripRssi(heartBeltNameTextField.value);
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
