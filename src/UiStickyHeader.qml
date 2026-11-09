import QtQuick 2.7
import QtQuick.Layouts 1.3

// Modern look: an open top-level section keeps a short copy of its header at the top of the
// page while its content scrolls under it, so it stays clear which section this is. The copy
// lives on the flickable, not in its content, and goes up with the end of the section; a tap
// on it scrolls back to the real header. It appears as soon as the real header starts going up
// and shrinks with the scroll from the height of the real one to its compact height.
// Goes into the section (AccordionElement, StaticAccordionElement) as an invisible item: the
// column of the section leaves it out of the layout.
Item {
    id: sticky

    // The section: its isOpen, title, depth, visible and height
    property Item section: null
    // The real header of the section (UiSectionHeader)
    property Item header: null

    visible: false
    readonly property int compactHeight: 44
    // The rows fade out under the copy instead of being cut through a line of text by its edge
    readonly property int fadeHeight: Math.round(16 * window.ui.textScale)
    property Item copy: null
    readonly property Item pageFlickable: {
        for (var p = section ? section.parent : null; p; p = p.parent)
            if (p.contentY !== undefined && p.contentItem !== undefined && p.flickableDirection !== undefined)
                return p
        return null
    }

    function update() {
        var f = pageFlickable
        var want = f && section && header && window.ui.modern && section.depth === 1 && section.isOpen
                   && section.visible
        var top = 0, bottom = 0
        if (want) {
            top = header.mapToItem(f, 0, 0).y
            bottom = section.mapToItem(f, 0, 0).y + section.height
            // Until the very end of the section: the copy goes up with its last row
            want = top < 0 && bottom > 0
        }
        if (!want) {
            if (copy)
                copy.visible = false
            return
        }
        if (!copy)
            copy = copyComponent.createObject(f)
        // On the outline of the section block, which runs 1 px outside the real header
        copy.x = header.mapToItem(f, 0, 0).x - 1
        copy.width = header.width + 2
        // At the end its bottom line lies on the bottom line of the block (1 px below the section)
        var h = Math.max(compactHeight, Math.min(header.height, top + header.height))
        copy.height = h
        copy.y = Math.min(0, bottom + 1 - h)
        // The fade under the copy only while the section goes on below it: at the end it would
        // lie on the header of the next section
        copy.fadeShown = bottom + 1 - h > fadeHeight
        copy.visible = true
    }
    function scrollToHeader() {
        var f = pageFlickable
        if (!f)
            return
        var at = header.mapToItem(f.contentItem, 0, 0).y - header.Layout.topMargin
        f.contentY = Math.max(0, Math.min(at, f.contentHeight - f.height))
    }

    Connections {
        target: sticky.pageFlickable
        enabled: sticky.section !== null && sticky.section.isOpen && sticky.section.depth === 1
        function onContentYChanged() { sticky.update() }
        function onHeightChanged() { sticky.update() }
    }
    Connections {
        target: sticky.section
        function onIsOpenChanged() { sticky.update() }
        function onHeightChanged() { if (sticky.section.isOpen) sticky.update() }
        function onVisibleChanged() { sticky.update() }
    }
    Component.onDestruction: if (copy) copy.destroy()

    Component {
        id: copyComponent
        UiFrame {
            z: 10
            height: sticky.compactHeight
            // As the corners of the block it slides into at the end
            radius: sticky.header.radius + 1
            fill: window.ui.surfaceHigh
            stroke: window.ui.accent
            strokeWidth: 1
            property bool fadeShown: false

            Accessible.role: Accessible.Button
            Accessible.name: sticky.section.title
            Accessible.onPressAction: sticky.scrollToHeader()

            // Under the copy: the page above it and its upper half in the page colour, so the
            // side lines of the section block start at the middle of the copy, not above it
            Rectangle {
                z: -1
                x: -3
                y: -24
                width: parent.width + 6
                height: 24 + parent.height / 2
                color: window.ui.bg
            }
            // Under the lower half of the copy (its round corners) and below it: the rows fade
            // out in the page colour, inside the side lines of the section block
            Rectangle {
                id: copyFade
                z: -1
                visible: parent.fadeShown
                x: 2
                y: parent.height / 2
                width: parent.width - 4
                height: parent.height / 2 + sticky.fadeHeight
                gradient: Gradient {
                    GradientStop { position: 0; color: window.ui.bg }
                    GradientStop { position: (copyFade.height - sticky.fadeHeight) / Math.max(1, copyFade.height); color: window.ui.bg }
                    GradientStop { position: 1; color: window.ui.alpha(window.ui.bg, 0) }
                }
            }
            // The accent bar of the open header, on the outline like there
            Rectangle {
                width: 4
                // As long as on the real header at its height; shorter as the copy shrinks, so that
                // at the compact height it stays within the straight part of the side
                height: Math.min(parent.height - 24, 14 + 1.5 * (parent.height - sticky.compactHeight))
                anchors.verticalCenter: parent.verticalCenter
                radius: 2
                color: window.ui.accent
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 18
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                text: window.ui.plainTitle(sticky.section.title)
                elide: Text.ElideRight
                // From the size of the real header down to the compact one as the copy shrinks
                font.pixelSize: parent.height > sticky.compactHeight + 6 ? window.ui.fontBody : window.ui.fontLabel
                font.weight: Font.Medium
                color: window.ui.accent
            }
            // The chevron of the real header, fading out as the copy shrinks
            UiIcon {
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                width: 24
                height: 24
                name: "expand_more"
                rotation: 180
                color: window.ui.textMuted
                opacity: Math.max(0, Math.min(1, (parent.height - sticky.compactHeight)
                                                 / Math.max(1, sticky.header.height - sticky.compactHeight)))
                visible: opacity > 0
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: sticky.scrollToHeader()
            }
        }
    }
}
