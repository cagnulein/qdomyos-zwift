import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0

ScrollView {
    id: creditsPage
    contentWidth: -1
    // Two children: the content height is given, not taken from an only child
    contentHeight: window.ui.modern ? modernCredits.implicitHeight + 24 : lblHelp.implicitHeight
    focus: true
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.fill: parent

    readonly property var developers: ["ben75020", "d3m3vilurr", "lifof", "p3g4asus", "Roberto Viola"]
    // Modern look: side margin of the page
    readonly property int modernMargin: Math.max(16, window.contentSideMargin)

    Label {
        id: lblHelp
        visible: !window.ui.modern
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        anchors.top: row1.bottom
        anchors.topMargin: 30
        text: "<b>" + qsTr("Credits") + "</b><br><br>" + qsTr("A very big thanks to<br>all the developers<br>(alphabetical sorted):") + "<br><br>ben75020<br>d3m3vilurr<br>lifof<br>p3g4asus<br>Roberto Viola"
        wrapMode: Label.WordWrap
    }

    // Modern look: a page title, the thanks as one line and the developers as a card of rows
    ColumnLayout {
        id: modernCredits
        visible: window.ui.modern
        x: creditsPage.modernMargin
        width: creditsPage.availableWidth - 2 * creditsPage.modernMargin
        spacing: 12

        Label {
            Layout.fillWidth: true
            text: qsTr("Credits")
            color: window.ui.textMain
            font.pixelSize: window.ui.fontHeadline
            font.weight: Font.DemiBold
            topPadding: 12
            wrapMode: Text.WordWrap
        }

        Label {
            Layout.fillWidth: true
            text: qsTr("A very big thanks to<br>all the developers<br>(alphabetical sorted):").split("<br>").join(" ")
            color: window.ui.textMuted
            font.pixelSize: window.ui.fontCaption
            wrapMode: Text.WordWrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 4
            implicitHeight: developerList.implicitHeight + 16
            radius: 16
            color: window.ui.surface

            Column {
                id: developerList
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8

                Repeater {
                    model: creditsPage.developers
                    Item {
                        width: developerList.width
                        // Material 3 list item: one line at least 56, the initial 16 from the card
                        // edge (with the 8 of the list) and the name 16 after it; a bigger text
                        // grows the row, 8 above and below
                        height: Math.max(56, Math.max(40, developerName.implicitHeight) + 16)
                        Rectangle {
                            id: developerAvatar
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            width: 40
                            height: 40
                            radius: 20
                            color: window.ui.alpha(window.ui.accent, 0.18)
                            Label {
                                anchors.centerIn: parent
                                text: modelData.charAt(0).toUpperCase()
                                color: window.ui.accent
                                font.pixelSize: window.ui.fontTitleSmall
                                font.weight: Font.DemiBold
                            }
                        }
                        Label {
                            id: developerName
                            anchors.left: developerAvatar.right
                            anchors.leftMargin: 16
                            anchors.right: parent.right
                            anchors.rightMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            color: window.ui.textMain
                            font.pixelSize: window.ui.fontBody
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
