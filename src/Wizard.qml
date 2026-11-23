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
    // Modern look: the side margins of the page column and of the button bar
    readonly property int modernMargin: Math.max(16, window.contentSideMargin)
    // Modern look: the height of the bar with Back and Next at the bottom of a step
    readonly property int modernBarHeight: 72

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
        height: 4
        color: window.ui.surfaceHigh
        Rectangle {
            height: parent.height
            radius: 2
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
        anchors.topMargin: window.ui.modern ? 4 : 0

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
             // Modern look: an answer card with an icon and a second line. The moving-on buttons
             // (Back, Next, Finish...) are in the bar there, so every visible button is an answer
             readonly property bool rich: window.ui.modern && iconName !== ""
             // TalkBack reads the second line after the label. In the name, not in
             // Accessible.description: the Android node takes the description only when the name
             // is empty (main.cpp, accessibleDescription). Without it the name is the text, as
             // the button sets it by default (qquickabstractbutton.cpp v5.15.0:1136)
             Accessible.name: rich && subtitle !== "" ? text + ", " + subtitle : text
             // A translated label can be wider than the screen: cap the button and wrap the text.
             // Capped by the screen, not by parent.width: in a GridLayout the parent's width comes
             // from the buttons themselves, and the loop squeezed them on Android
             Layout.maximumWidth: window.ui.modern ? Math.min(480, stackViewLocal.width - 2 * wizardPage.modernMargin) : stackViewLocal.width * 0.8
             Layout.fillWidth: window.ui.modern
             // Modern look, answer cards (list items of the style guide): the card is the whole
             // button, no Material inset around it, 16 from the edges to the icon or the text,
             // at least 56 high (72 with the second line), the text plus 8 above and below
             Binding on topInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
             Binding on bottomInset { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
             Binding on horizontalPadding { when: window.ui.modern; value: 16; restoreMode: Binding.RestoreBindingOrValue }
             Binding on verticalPadding { when: window.ui.modern; value: 8; restoreMode: Binding.RestoreBindingOrValue }
             // Classic: the stock Button formula
             implicitHeight: window.ui.modern ? Math.max(rich ? 72 : 56, implicitContentHeight + 16)
                                              : Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)
             // UiFrame, not Rectangle.border: a thin border breaks up on Android
             background: UiFrame {
                 fill: !window.ui.modern ? wizardButton.backgroundColor
                      : (wizardButton.down ? window.ui.surfaceHighest : window.ui.surfaceHigh)
                 radius: window.ui.modern ? 16 : 5
                 strokeWidth: window.ui.modern ? 1 : 0
                 stroke: window.ui.alpha(window.ui.textMain, 0.08)
             }
             contentItem: Item {
                 implicitWidth: wizardButton.rich ? richRow.implicitWidth : plainText.implicitWidth
                 implicitHeight: wizardButton.rich ? richRow.implicitHeight : plainText.implicitHeight

                 Text {
                     id: plainText
                     anchors.fill: parent
                     visible: !wizardButton.rich
                     font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                     font.weight: window.ui.modern ? Font.DemiBold : Font.Normal
                     text: wizardButton.text
                     color: window.ui.modern ? window.ui.textMain : wizardButton.textColor
                     wrapMode: Text.WordWrap
                     horizontalAlignment: Text.AlignHCenter
                     verticalAlignment: Text.AlignVCenter
                 }

                 Row {
                     id: richRow
                     visible: wizardButton.rich
                     anchors.verticalCenter: parent.verticalCenter
                     width: parent.width
                     spacing: 16

                     Rectangle {
                         anchors.verticalCenter: parent.verticalCenter
                         width: 40
                         height: 40
                         radius: 20
                         color: window.ui.surfaceHighest
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
                         width: parent.width - 56
                         spacing: 2
                         Text {
                             width: parent.width
                             text: wizardButton.text
                             font.pixelSize: window.ui.fontBody
                             font.weight: Font.DemiBold
                             color: window.ui.textMain
                             wrapMode: Text.WordWrap
                         }
                         Text {
                             width: parent.width
                             visible: wizardButton.subtitle !== ""
                             text: wizardButton.subtitle
                             font.pixelSize: window.ui.fontCaption
                             color: window.ui.textMuted
                             wrapMode: Text.WordWrap
                         }
                     }
                 }
             }
     }

    // Modern look: the bar at the bottom of a step with Back on the left and the step's own
    // action (Next, Finish, Start, Close) on the right, each half of the page column; the page
    // content scrolls above it. The classic look keeps its buttons in the content and hides the bar
    component WizardBar: Rectangle {
        id: wizardBar
        property bool backVisible: true
        property string primaryText: ""
        // the Flickable of the step's ScrollView (its contentItem)
        property Flickable flick: null
        signal backClicked()
        signal primaryClicked()
        readonly property real buttonWidth: (width - 2 * wizardPage.modernMargin - 8) / 2

        visible: window.ui.modern
        height: wizardPage.modernBarHeight
        // On the page background while the whole content is seen; surfaceHigh while there is
        // more of it below (as an M3 app bar with content scrolled under it). The height check
        // first: atYEnd starts false before the first layout of a short step
        color: flick && flick.contentHeight > flick.height && !flick.atYEnd ? window.ui.surfaceHigh : "transparent"
        Behavior on color { ColorAnimation { duration: 150 } }

        // Modern look: a step drags only when its content is taller than the view. A ScrollView makes
        // a plain Flickable (qquickscrollview.cpp v5.15.0:183), and its default AutoFlickDirection
        // drags whenever the heights differ, a short page too (qquickflickable.cpp v5.15.0:2336)
        Binding {
            when: window.ui.modern && wizardBar.flick !== null
            target: wizardBar.flick
            property: "flickableDirection"
            value: Flickable.AutoFlickIfNeeded
            restoreMode: Binding.RestoreBindingOrValue
        }

        UiButton {
            visible: wizardBar.backVisible
            x: wizardPage.modernMargin
            anchors.verticalCenter: parent.verticalCenter
            width: wizardBar.buttonWidth
            // the tonal pill on a surfaceHigh surface
            cardSurface: true
            wrapsText: true
            text: qsTr("Back")
            onClicked: wizardBar.backClicked()
        }

        UiButton {
            visible: wizardBar.primaryText !== ""
            x: wizardBar.width - wizardPage.modernMargin - width
            anchors.verticalCenter: parent.verticalCenter
            width: wizardBar.buttonWidth
            highlighted: true
            wrapsText: true
            text: wizardBar.primaryText
            onClicked: wizardBar.primaryClicked()
        }
    }

    Component {
        id: welcomeComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

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
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(28)
                        font.bold: true
                        color: window.ui.ink("white")
                        Layout.topMargin: window.ui.modern ? 16 : 0
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Created by Roberto Viola")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(24)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.topMargin: window.ui.modern ? 4 : 0
                    }

                    Text {
                        Layout.topMargin: window.ui.modern ? 12 : 0
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("QZ is designed to maximize your workout experience on a range of fitness equipment, including indoor bikes, treadmills, ellipticals, and rower. By connecting seamlessly with your devices, QZ provides realtime data, personalized workout plans, and interactive elements to keep you motivated.\n\nThe following questions will customize QZ for your equipment and goals.")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Start")
                        onClicked: goNext()
                    }
                }
            }

            function goNext() {
                stackViewLocal.push(step1Component)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                backVisible: false
                primaryText: qsTr("Start")
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: step1Component

        Item {
            anchors.fill: parent
            // no button bar on this step: the drag fix of WizardBar is set here
            Binding {
                when: window.ui.modern
                target: stepScroll.contentItem
                property: "flickableDirection"
                value: Flickable.AutoFlickIfNeeded
                restoreMode: Binding.RestoreBindingOrValue
            }

            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content (this step has no button bar)
                bottomPadding: window.ui.modern ? 16 : 0
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
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("How can I help you?")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    WizardButton {
                        Layout.topMargin: window.ui.modern ? 12 : 0
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
                        Layout.topMargin: window.ui.modern ? 8 : 0
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
                        Layout.topMargin: window.ui.modern ? 8 : 0
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
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("What's your fitness device?")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Repeater {
                        model: [QT_TR_NOOP("Bike"), QT_TR_NOOP("Treadmill"), QT_TR_NOOP("Rower"), QT_TR_NOOP("Elliptical")]
                        delegate: WizardButton {
                            Layout.topMargin: window.ui.modern ? (index === 0 ? 12 : 8) : 0
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
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                onBackClicked: stackViewLocal.pop()
            }
        }
    }

    Component {
        id: step3Component

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Choose your preferred app")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.topMargin: window.ui.modern ? 4 : 0
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
                        // Modern look: two columns of answer cards across the page column, 8 apart.
                        // true and not window.ui.modern: a layout with fillWidth unset fills (as in
                        // the classic look), an explicit false would narrow it there (qquicklayout.cpp
                        // v5.15.0:1153)
                        Layout.fillWidth: true
                        Layout.topMargin: window.ui.modern ? 12 : 0
                        columns: 2
                        rowSpacing: window.ui.modern ? 8 : 20
                        columnSpacing: window.ui.modern ? 8 : 20
                        Repeater {
                            model: ["Peloton", "Zwift", "MyWhoosh", "Rouvy", "Kinomap", "Indievelo", "Fulgaz", "EXR", QT_TR_NOOP("Other app"), QT_TR_NOOP("QZ Standalone")]
                            delegate: WizardButton {
                                // a bigger text: wider buttons, two of them still fit the screen.
                                // Modern look: each column is half of the page column
                                Layout.preferredWidth: window.ui.modern ? (stackViewLocal.width - 2 * wizardPage.modernMargin - 8) / 2
                                                     : window.textScale !== 1 ? Math.min(window.fontPx(150), (stackViewLocal.width * 0.9 - 20) / 2) : 150
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
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                onBackClicked: stackViewLocal.pop()
            }
        }
    }

    Component {
        id: pelotonLoginComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Connect to Peloton")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.topMargin: window.ui.modern ? 4 : 0
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
                        Layout.topMargin: window.ui.modern ? 12 : 0
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
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                onBackClicked: stackViewLocal.pop()
            }
        }
    }

    Component {
        id: pelotonDifficultyComponent

        Item {
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Peloton Difficulty")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        text: qsTr("Typically, Peloton coaches call out a range for target incline, resistance and/or speed. Use this setting to choose the difficulty of the target QZ communicates. Difficulty level can be set to lower, upper or average")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.fillWidth: true
                        Layout.topMargin: window.ui.modern ? 4 : 0
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Difficulty")
                        // Modern look: a field label
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        font.bold: !window.ui.modern
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.topMargin: window.ui.modern ? 24 : 0
                    }

                    ValueComboBox {
                        id: pelotonDifficultyTextField
                        model: [ "lower", "upper", "average" ]
                        labels: ({ "lower": qsTr("lower"), "upper": qsTr("upper"), "average": qsTr("average") })
                        value: settings.peloton_difficulty
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        onActivated: {
                            console.log("combomodel activated" + pelotonDifficultyTextField.currentIndex)
                            settings.peloton_difficulty = pelotonDifficultyTextField.currentValue;
                        }
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: goNext()
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            function goNext() {
                settings.peloton_difficulty = pelotonDifficultyTextField.value;
                stackViewLocal.push(finalStepComponent)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Finish")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: zwiftComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Bike Resistance Level")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.topMargin: window.ui.modern ? 4 : 0
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
                        Layout.topMargin: window.ui.modern ? 12 : 0
                        Layout.alignment: Qt.AlignHCenter
                        from: 0
                        to: 100
                        value: 18
                        editable: true
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: goNext()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            function goNext() {
                settings.bike_resistance_offset = spinBoxResistanceOffset.value;
                settings.speed_power_based = true;
                stackViewLocal.push(finalStepComponent);
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Next")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: step4Component

        Item {
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Custom Configurations")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Here you will see custom configurations based on your previous choices.")
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.8
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: goNext()
                    }
                }
            }

            function goNext() {
                stackViewLocal.push(finalStepComponent)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Finish")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: step2HelpComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select a feature")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
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
                            Layout.topMargin: window.ui.modern ? (index === 0 ? 12 : 8) : 0
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
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                onBackClicked: stackViewLocal.pop()
            }
        }
    }

    Component {
        id: zwiftAutoInclination

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Zwift Credentials")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        // a plain Text does not inherit the window font: the text size while it is not 100 %
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontBody : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        text: qsTr("QZ will read the inclination in real time from the Zwift app and will adjust the inclination on your treadmill. It doesn't work on workout")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Username")
                        // Modern look: a field label
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        font.bold: !window.ui.modern
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.topMargin: window.ui.modern ? 24 : 0
                    }

                    UiTextField {
                        id: zwiftUsernameTextField
                        text: settings.zwift_username
                        // Modern look: the field takes the column, its text from the left
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        Layout.fillWidth: window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        Layout.fillHeight: false
                        onAccepted: settings.zwift_username = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Password")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        font.bold: !window.ui.modern
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.topMargin: window.ui.modern ? 24 : 0
                    }

                    UiTextField {
                        id: zwiftPasswordTextField
                        text: settings.zwift_password
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        Layout.fillWidth: window.ui.modern
                        Layout.fillHeight: false
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        inputMethodHints: Qt.ImhHiddenText
                        echoMode: TextInput.PasswordEchoOnEdit
                        onAccepted: settings.zwift_password = text
                        onActiveFocusChanged: if(this.focus) this.cursorPosition = this.text.length
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: goNext()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            function goNext() {
                settings.zwift_username = zwiftUsernameTextField.text;
                settings.zwift_password = zwiftPasswordTextField.text;
                settings.zwift_api_autoinclination = true;
                stackViewLocal.push(finalStepComponent);
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Next")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: zwiftPlayClick

        Item {
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 24
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Zwift Play and Click")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontBody : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        id: zwiftPlayClickDescription
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        text: qsTr("Enable the one that you would like to use directly with QZ. Remember to update their firmware before using it.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    UiSwitchDelegate {
                        Layout.topMargin: window.ui.modern ? 12 : 0
                        text: qsTr("Zwift Click")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        leftPadding: 0
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        clip: false
                        width: zwiftPlayClickDescription.width
                        checked: settings.zwift_click
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_click = checked;  }
                    }

                    UiSwitchDelegate {
                        Layout.topMargin: window.ui.modern ? 8 : 0
                        text: qsTr("Zwift Play")
                        spacing: 0
                        bottomPadding: 0
                        topPadding: 0
                        rightPadding: 0
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        width: zwiftPlayClickDescription.width
                        leftPadding: 0
                        clip: false
                        checked: settings.zwift_play
                        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                        Layout.fillWidth: true
                        onClicked: { settings.zwift_play = checked; }
                    }

                    Text {
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontBody : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 24 : 0
                        text: qsTr("Correct startup phase:\n\n1. close any app that can connect to your Zwift devices\n2. wake up your Zwift devices\n3. wake up your trainer\n4. open qz\n5. now if you change gear on your Zwift device you will see a reaction in the gear tile on qz and so on your trainer.")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        color: window.ui.ink("white")
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: goNext()
                    }
                }
            }

            function goNext() {
                settings.tile_gears_enabled = true;
                stackViewLocal.push(finalStepComponent);
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Finish")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: virtualShifting

        Item {
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Virtual Shifting")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontBody : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        text: qsTr("Virtual shifting enabled! You can change gears using the gears tile in QZ directly, or you can also add a bluetooth remote or a Zwift Play or a Zwift Click to control it!")
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.fillWidth: true
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: goNext()
                    }
                }
            }

            function goNext() {
                settings.tile_gears_enabled = true;
                stackViewLocal.push(finalStepComponent);
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Finish")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: step3HelpComponent

        Item {
            readonly property bool wizardLastStep: true
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Custom Configurations")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontBody : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        text: qsTr("Here you will see custom configurations based on the selected feature.")
                        wrapMode: Text.WordWrap
                        width: parent.width * 0.8
                        horizontalAlignment: Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Finish")
                        onClicked: goNext()
                    }
                }
            }

            function goNext() {
                stackViewLocal.push(finalStepComponent)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Finish")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: finalStepComponent

        Item {
            readonly property bool wizardDone: true
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true
                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Thank you for setting up QZ!")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(20)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Binding on font.pixelSize { when: window.ui.modern || window.scaledFontPx > 0; value: window.ui.modern ? window.ui.fontBody : window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        text: qsTr("If you have any questions or need further assistance, feel free to write to me at roberto.viola83@gmail.com. You can also restart this wizard from the left side bar menu. To apply some changes, you may need to restart the app.")
                        wrapMode: Text.WordWrap
                        width: stackViewLocal.width * 0.8
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        color: window.ui.modern ? window.ui.textMuted : "white"
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
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Close")
                        onClicked: goNext()
                    }
                }
            }

            function goNext() {
                stackView.pop();
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                backVisible: false
                primaryText: qsTr("Close")
                onPrimaryClicked: goNext()
            }
        }
    }
    Component {
        id: bluetoothDeviceSelectionComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select Your Fitness Device")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    RowLayout {
                        spacing: window.ui.modern ? 8 : 10
                        // The list across the column (as a layout with fillWidth unset, the classic
                        // look); modern look: Refresh beside it, as on the heart rate step
                        Layout.fillWidth: true
                        Layout.topMargin: window.ui.modern ? 12 : 0
                        DeviceComboBox {
                            id: filterDeviceTextField
                            labels: ({ "Disabled": qsTr("Disabled") })
                            value: settings.filter_device
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + filterDeviceTextField.currentIndex)
                            }
                        }

                        UiButton {
                            visible: window.ui.modern
                            // not the OK of the list before it: neither hidden nor pressed on a pick
                            applyOnSelect: false
                            Layout.alignment: Qt.AlignVCenter
                            text: qsTr("Refresh")
                            onClicked: refresh_bluetooth_devices_clicked();
                        }
                    }

                    WizardButton {
                        id: refreshFilterDeviceButton
                        visible: !window.ui.modern
                        text: qsTr("Refresh")
                        Layout.alignment: Qt.AlignHCenter
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: goNext()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            function goNext() {
                settings.filter_device = stripRssi(filterDeviceTextField.value);
                stackViewLocal.push(unitSelectionComponent)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Next")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: unitSelectionComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Unit System")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
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
                        Layout.topMargin: window.ui.modern ? 4 : 0
                    }

                    UiComboBox {
                        id: unitSystemComboBox
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 12 : 0
                        model: [qsTr("Metric"), qsTr("Imperial")]
                        currentIndex: settings.miles_unit ? 1 : 0
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: goNext()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            function goNext() {
                settings.miles_unit = unitSystemComboBox.currentIndex === 1
                stackViewLocal.push(userInfoComponent)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Next")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: userInfoComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("User Information")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
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
                        Layout.topMargin: window.ui.modern ? 12 : 0
                    }

                    UiSpinBox {
                        id: weightSpinBox
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
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
                        Layout.topMargin: window.ui.modern ? 24 : 0
                    }

                    UiSpinBox {
                        id: ageSpinBox
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
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
                        Layout.topMargin: window.ui.modern ? 24 : 0
                    }

                    ValueComboBox {
                        id: genderComboBox
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        // Modern look: as wide as the counters above it (-1: the classic implicit width)
                        Layout.preferredWidth: window.ui.modern ? 160 : -1
                        model: ["Male", "Female"]
                        labels: ({ "Male": qsTr("Male"), "Female": qsTr("Female") })
                        currentIndex: settings.sex === "Male" ? 0 : 1
                        value: settings.sex === "Male" ? "Male" : "Female"
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: goNext()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            function goNext() {
                settings.weight = weightSpinBox.realValue
                settings.age = ageSpinBox.value
                settings.sex = genderComboBox.value
                stackViewLocal.push(heartRateDeviceSelectionComponent)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Next")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }

    Component {
        id: heartRateDeviceSelectionComponent

        Item {
            anchors.fill: parent
            ScrollView {
                id: stepScroll
                contentWidth: availableWidth
                topPadding: 16
                // Modern look: 16 under the content, and the content ends above the button bar
                bottomPadding: window.ui.modern ? 16 : 0
                // Qt 5.15 keeps a stale implicit height after text wraps: size the content by the column itself
                contentHeight: contentChildren[0].height
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.fill: parent
                anchors.bottomMargin: window.ui.modern ? wizardPage.modernBarHeight : 0
                Layout.preferredHeight: parent.height
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 20
                    // Modern look: the gaps are the margins of the items (style guide)
                    Binding on spacing { when: window.ui.modern; value: 0; restoreMode: Binding.RestoreBindingOrValue }
                    width: window.ui.modern ? parent.width - 2 * wizardPage.modernMargin : parent.width * 0.9

                    Text {
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Select Your Heart Rate Device")
                        font.pixelSize: window.ui.modern ? window.ui.fontHeadline : window.fontPx(24)
                        font.bold: true
                        color: window.ui.ink("white")
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 4 : 0
                        text: qsTr("Choose your heart rate belt or select a smartwatch option:")
                        font.pixelSize: window.ui.modern ? window.ui.fontBody : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        // Modern look: centred under the title, as the hints of the other steps
                        horizontalAlignment: window.ui.modern ? Text.AlignHCenter : Text.AlignLeft
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: window.ui.modern ? 8 : 10
                        Layout.fillWidth: true
                        Layout.topMargin: window.ui.modern ? 12 : 0

                        DeviceComboBox {
                            id: heartBeltNameTextField
                            labels: ({ "Disabled": qsTr("Disabled") })
                            value: settings.heart_rate_belt_name
                            Layout.fillHeight: false
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onActivated: {
                                console.log("combomodel activated" + heartBeltNameTextField.currentIndex)
                            }
                        }

                        // Modern look: Refresh as a tonal button beside the list
                        UiButton {
                            visible: window.ui.modern
                            // not the OK of the list before it: neither hidden nor pressed on a pick
                            applyOnSelect: false
                            Layout.alignment: Qt.AlignVCenter
                            text: qsTr("Refresh")
                            onClicked: refresh_bluetooth_devices_clicked();
                        }
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        id: refreshHeartBeltNameButton
                        text: qsTr("Refresh")
                        onClicked: refresh_bluetooth_devices_clicked();
                    }

                    Text {
                        wrapMode: Text.WordWrap
                        // Modern look: the title of the group of watch cards
                        horizontalAlignment: window.ui.modern ? Text.AlignLeft : Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Or select a smartwatch option:")
                        font.pixelSize: window.ui.modern ? window.ui.fontLabel : window.fontPx(20)
                        color: window.ui.modern ? window.ui.textMuted : "white"
                        Layout.topMargin: window.ui.modern ? 24 : 20
                    }

                    // Modern look: the watches are answer cards with an icon, and the line under
                    // each button of the classic look is their second line
                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 12 : 0
                        iconName: "watch"
                        subtitle: qsTr("Download the QZ Companion App there")
                        text: qsTr("Apple Watch")
                        onClicked: {
                            settings.heart_rate_belt_name = "Disabled"
                            settings.garmin_companion = false
                            stackViewLocal.push(step3Component)
                        }
                    }

                    Text {
                        // Modern look: the second line of the watch card above
                        visible: !window.ui.modern
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: "white"
                        Layout.bottomMargin: 20
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 8 : 0
                        iconName: "watch"
                        subtitle: qsTr("Download the QZ Companion App there")
                        text: qsTr("Wear OS watch")
                        onClicked: {
                            settings.heart_rate_belt_name = "Disabled"
                            settings.garmin_companion = false
                            stackViewLocal.push(step3Component)
                        }
                    }

                    Text {
                        // Modern look: the second line of the watch card above
                        visible: !window.ui.modern
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: "white"
                        Layout.bottomMargin: 20
                    }

                    WizardButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: window.ui.modern ? 8 : 0
                        iconName: "watch"
                        subtitle: qsTr("Download the QZ Companion App there")
                        text: qsTr("Garmin watch")
                        onClicked: {
                            settings.heart_rate_belt_name = "Disabled"
                            settings.garmin_companion = true
                            stackViewLocal.push(step3Component)
                        }
                    }

                    Text {
                        // Modern look: the second line of the watch card above
                        visible: !window.ui.modern
                        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.plainFontPx; restoreMode: Binding.RestoreBindingOrValue }
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Download the QZ Companion App there")
                        color: "white"
                    }

                    Item {
                        // Modern look: the buttons are in the bar under the content
                        visible: !window.ui.modern
                        Layout.preferredHeight: 50
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Next")
                        onClicked: goNext()
                    }

                    WizardButton {
                        visible: !window.ui.modern
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Back")
                        onClicked: stackViewLocal.pop()
                    }
                }
            }

            function goNext() {
                settings.heart_rate_belt_name = stripRssi(heartBeltNameTextField.value);
                settings.garmin_companion = false
                stackViewLocal.push(step3Component)
            }

            WizardBar {
                flick: stepScroll.contentItem
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                primaryText: qsTr("Next")
                onBackClicked: stackViewLocal.pop()
                onPrimaryClicked: goNext()
            }
        }
    }
}
