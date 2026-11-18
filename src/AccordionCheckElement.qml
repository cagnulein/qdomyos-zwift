import QtQuick 2.7
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0

ColumnLayout {
    property Settings settings: null
    id: rootElement
    property  bool linkedBoolSettingDefault: false
    property string linkedBoolSetting: "example_setting"
    property bool isOpen: false
    property string title: ""
    // A note under the element (what the setting does); empty - none
    property string description: ""
    // Modern look: the element as a card of its own (the tile page: one card per tile). Off for
    // the elements nested in the settings sections
    property bool card: false
    default property alias accordionContent: contentPlaceholder.data
    spacing: 0
    readonly property bool modernCard: window.ui.modern && card
    // A card keeps a little room from the cards around it
    Layout.topMargin: modernCard ? 4 : 0
    Layout.bottomMargin: modernCard ? 4 : 0

    // How deep it is among the settings sections (as AccordionElement): switched on inside a
    // section, it is framed as a subsection of it
    readonly property int depth: {
        var d = 1
        for (var p = parent; p; p = p.parent)
            if (p.isOpen !== undefined && p.title !== undefined && p.accordionContent !== undefined)
                d++
        return d
    }

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
    // Turn the element on or off and save it, as a tap on the classic switch does
    function toggle(on) {
        rootElement.isOpen = on;
        if (typeof(settings[rootElement.linkedBoolSetting])=="undefined") {
            settings.setValue(rootElement.linkedBoolSetting, rootElement.convertValue(on));
        }
        else {
            settings[rootElement.linkedBoolSetting] = rootElement.convertValue(on);
        }
    }

    // Modern look: a plain switch row with the switch of the modern look; a tap anywhere
    // on the row switches it. Made only while the look is on (as the header of AccordionElement):
    // the classic page of tiles does not carry a hidden switch in every section
    Loader {
        id: modernHeader
        active: window.ui.modern
        visible: active
        Layout.fillWidth: true
        // A tile card: the row inside its frame
        Layout.leftMargin: rootElement.modernCard ? 12 : 0
        Layout.rightMargin: rootElement.modernCard ? 8 : 0
        Layout.topMargin: rootElement.modernCard ? 4 : 0
        readonly property real radius: item ? item.radius : 0
        sourceComponent: Rectangle {
            implicitHeight: Math.max(48, modernTitle.implicitHeight + 16)
            radius: 12
            color: modernArea.pressed ? window.ui.surfaceHigh : "transparent"

            Accessible.role: Accessible.CheckBox
            Accessible.name: rootElement.title
            Accessible.checkable: true
            Accessible.checked: rootElement.isOpen
            Accessible.onPressAction: rootElement.toggle(!rootElement.isOpen)

            MouseArea {
                id: modernArea
                anchors.fill: parent
                onClicked: rootElement.toggle(!rootElement.isOpen)
            }

            Label {
                id: modernTitle
                anchors.left: parent.left
                // Open, the element is a block out to the edges of the setting frames: the title as
                // far in as the text of a setting in its frame. Closed, it is a setting in a frame
                // itself: no room of its own, as the other switches. A tile card: a little in from
                // the edge of the pressed shade
                anchors.leftMargin: rootElement.modernCard ? 4 : rootElement.isOpen ? settingFrames.framePadH : 0
                anchors.right: modernSwitch.left
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                text: rootElement.title
                wrapMode: Text.WordWrap
                // A tile card: its title stands out, in the accent while the tile is on
                color: rootElement.modernCard && rootElement.isOpen ? window.ui.accent : window.ui.textMain
                font.weight: rootElement.modernCard ? Font.DemiBold : Font.Normal
                // The size as well: in Qt 5 a write of the weight alone keeps the size the label
                // inherits at that moment, and a change of the text size missed it
                font.pixelSize: window.ui.fontBase
            }

            UiSwitch {
                id: modernSwitch
                anchors.right: parent.right
                anchors.rightMargin: rootElement.isOpen ? settingFrames.framePadH : 0
                anchors.verticalCenter: parent.verticalCenter
                checked: rootElement.isOpen
                onClicked: {
                    rootElement.toggle(checked)
                    // A tap set checked and broke the binding: follow isOpen again, so a later toggle
                    // by the row or the screen reader still moves the switch
                    checked = Qt.binding(function() { return rootElement.isOpen })
                }
                Accessible.ignored: true
            }
        }
    }

    SwitchDelegate {
        // on iOS the title of a switch kept the size of the style while the text around it grew: with a size other than 100 % it takes the one of the window font
        Binding on font.pixelSize { when: window.scaledFontPx > 0; value: window.scaledFontPx; restoreMode: Binding.RestoreBindingOrValue }
        visible: !window.ui.modern
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
        // Screen reader: Qt 5.15 gives a SwitchDelegate no press or toggle action, so on Android its
        // node is not clickable and TalkBack's swipe passed over the switch of every section.
        // The press action makes it a stop; it turns the switch and saves it as a tap does, as
        // Qt 6.11.2 does itself (QTBUG-144350). A TalkBack double tap presses it with the space
        // key instead, see CustomQtActivity
        Accessible.name: text
        Accessible.checkable: true
        Accessible.checked: checked
        Accessible.onPressAction: {
            checked = !checked
            clicked()
        }
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
 
    // A tile card: the note right under the title, before the options. Made only for a card
    Loader {
        active: rootElement.modernCard && rootElement.description.length > 0
        visible: active
        Layout.fillWidth: true
        Layout.leftMargin: 16
        Layout.rightMargin: 16
        Layout.bottomMargin: 4
        sourceComponent: Label {
            text: rootElement.description
            font.pixelSize: window.fontHint
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            color: window.ui.textMuted
        }
    }

    // This will get filled with the content. Held in a plain item: in a settings section the
    // modern look lays the frames of its settings beside it (a column would lay them out as
    // rows), the switch row on top of the block
    Item {
        id: contentBox
        // Framed as a section of settings; a tile card has a frame of its own
        readonly property bool framed: window.ui.modern && !rootElement.modernCard
        visible: rootElement.isOpen
        Layout.fillWidth: true
        // Takes the free height as the column it replaces did (a layout fills by default): a page
        // that gives the section the rest of its height (the list of profiles) keeps it
        Layout.fillHeight: true
        // Set on a change, not bound: a binding made a loop with the layout of the column
        implicitHeight: 0
        Connections {
            target: contentPlaceholder
            function onImplicitHeightChanged() { contentBox.implicitHeight = contentPlaceholder.implicitHeight }
        }
        Component.onCompleted: implicitHeight = contentPlaceholder.implicitHeight
        // Modern look: inside the block of the element and inside the frames of its settings. A tile
        // card: its options under the title, clear of the right line of its frame
        Layout.leftMargin: rootElement.modernCard ? 16 : settingFrames.insetLeft
        Layout.rightMargin: rootElement.modernCard ? 18 : settingFrames.insetRight
        Layout.topMargin: settingFrames.insetTop
        Layout.bottomMargin: settingFrames.insetBottom
        // Under the switch row: the block frame starts behind it
        z: -1

        // Modern look: made the first time the section is framed, and kept - back in the classic look it
        // gives the settings their own margins again (a tile card is never framed, so it gets none)
        Loader {
            id: settingFrames
            property bool used: false
            active: contentBox.framed || used
            onLoaded: used = true
            z: -1
            readonly property int insetLeft: item ? item.insetLeft : 0
            readonly property int insetRight: item ? item.insetRight : 0
            readonly property int insetTop: item ? item.insetTop : 0
            readonly property int insetBottom: item ? item.insetBottom : 0
            readonly property int framePadH: item ? item.framePadH : 0
            sourceComponent: UiSettingFrames {
                holder: contentBox
                content: contentPlaceholder.children.length > 0 ? contentPlaceholder.children[0] : null
                header: modernHeader
                depth: rootElement.depth
            }
        }

        ColumnLayout {
            id: contentPlaceholder
            width: parent.width
            height: parent.height
        }
    }

    // The note of the element, under it (description): the pages wrote it as a label of their own
    // after each element. A tile card shows it under its title instead
    Label {
        visible: !rootElement.modernCard && rootElement.description.length > 0
        text: rootElement.description
        font.bold: true
        font.italic: true
        font.pixelSize: window.fontHint
        bottomPadding: window.hintGap
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignVCenter
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        Layout.fillWidth: true
        color: Material.color(Material.Lime)
    }

    // A tile card: its frame around the whole element. Laid out as the last row, 12 high - the room
    // under the content of the card; the frame reaches up over the element and lies under the other
    // rows (z). A frame as a row of its own would push the rows, and one bound to the height of
    // the element as its child would make a binding loop with the layout. Made only for a card
    Loader {
        id: cardFrame
        active: rootElement.modernCard
        visible: active
        Layout.fillWidth: true
        Layout.preferredHeight: 12
        z: -2
        sourceComponent: Item {
            UiFrame {
                y: -cardFrame.y
                width: parent.width
                height: rootElement.height
                radius: 16
                strokeWidth: 1
                // A tile that is on (shown on the main page) in the toned-down accent: still marked,
                // not as bright as an open section
                stroke: rootElement.isOpen ? Qt.rgba(window.ui.accent.r, window.ui.accent.g, window.ui.accent.b, 0.4)
                                           : window.ui.outline
            }
        }
    }
}
