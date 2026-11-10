import QtQuick 2.7
import QtQuick.Layouts 1.3

// Modern look: the frames of an open settings section, laid under its content.
// A setting is not an item of its own in settings.qml but loose neighbours in the column of the
// section (a switch, then its description), so the column is cut into groups - a control and the
// descriptions after it - and each group gets the space around it by its layout margins and a
// frame drawn by the geometry of its first and last item. Sections inside are left out: they draw
// their own block. The block of the section itself (the header on top of it) is drawn here too.
// Goes into the item that holds the content at its (0, 0) - the Loader of AccordionElement, the
// box of StaticAccordionElement and AccordionCheckElement - which is laid out in the column of
// the section under its header; made by a Loader of its own there, it is told that item by holder.
Item {
    id: frames

    // The column of the settings
    property Item content: null
    // The header of the section: the top of its block
    property Item header: null
    // 1 - a top-level section, 2 - inside it, and so on; deeper ones are drawn as the second
    property int depth: 1
    // The item that holds the content and these frames at its (0, 0)
    property Item holder: parent

    readonly property int framePadH: 10
    readonly property int framePadV: 8
    readonly property int frameGap: 6
    readonly property int blockPad: 6
    // The margins of the holder in the column of the section: the content inside the block and
    // inside the frames of its settings
    readonly property int insetLeft: !window.ui.modern ? 0
                                   : depth === 1 ? (header ? header.Layout.leftMargin : 0) + blockPad + framePadH
                                   : blockPad + framePadH
    readonly property int insetRight: insetLeft
    // The content is one control, not a column of settings (a section of a single "Enable support"
    // switch): it gets one frame around itself, and the room for it is left by the holder margins
    // A row is one setting as well ("GR7 Cadence Multiplier" with its field and OK): cut into
    // groups, the OK button got the margins of a setting of its own and sat lower than the field
    readonly property bool single: content !== null && (content.checked !== undefined || content.text !== undefined
                                                         || String(content).indexOf("QQuickRowLayout") === 0)
    readonly property int insetTop: !window.ui.modern ? 0 : blockPad + (single ? framePadV : 0)
    readonly property int insetBottom: insetTop

    property var itemFrames: []
    // The layout margins of the content as settings.qml set them, to add to and to give back
    property var baseMargins: []
    // Some margin of the content differs from its own: set by the modern look, to give back once
    // the look is classic again. While false, the classic look has nothing to do here
    property bool touched: false

    // A section inside draws its own block; a switch section (AccordionCheckElement) only while
    // it is on - switched off it is a switch row like the others
    function isSectionLike(c) {
        if (c.title === undefined || (c.nested === undefined && c.accordionContent === undefined))
            return false
        return c.linkedBoolSetting === undefined || c.isOpen
    }
    // A label of its own: the name of the control below it
    function isTitle(c) {
        return c.text !== undefined && c.wrapMode !== undefined && c.checked === undefined && c.readOnly === undefined
    }
    // A note in bold italic, which the modern look keeps in its own colour ("Apple Watch users:
    // leave it disabled!", "0.005 = Clinchers", "IMPORTANT: ... UNIT ID")
    function isNote(c) {
        return c.text !== undefined && c.wrapMode !== undefined && c.font !== undefined && c.font.italic
    }
    // A UiButton on its own in the column
    function isButton(c) {
        return c.applyOnSelect !== undefined && c.clicked !== undefined
    }
    // A drop-down list, or a row with one in it
    function hasPicker(c) {
        if (c.popup !== undefined && c.activated !== undefined)
            return true
        for (var i = 0; i < c.children.length; i++)
            if (c.children[i].popup !== undefined && c.children[i].activated !== undefined)
                return true
        return false
    }
    function baseOf(c) {
        for (var i = 0; i < baseMargins.length; i++)
            if (baseMargins[i].item === c)
                return baseMargins[i]
        var b = { item: c, top: c.Layout.topMargin, bottom: c.Layout.bottomMargin,
                  left: c.Layout.leftMargin, right: c.Layout.rightMargin }
        baseMargins.push(b)
        return b
    }
    function setMargin(c, key, v) {
        if (c.Layout[key] !== v)
            c.Layout[key] = v
    }
    function regroup() {
        // Classic look with the margins as they came: no frames, nothing to give back
        if (!content || (!window.ui.modern && !touched)) {
            if (itemFrames.length)
                itemFrames = []
            return
        }
        if (single) {
            var one = window.ui.modern ? [{ first: content, last: content }] : []
            if (one.length !== itemFrames.length || (one.length && itemFrames[0].first !== content))
                itemFrames = one
            return
        }
        var kids = content.children
        var entries = []
        var cur = null
        for (var i = 0; i < kids.length; i++) {
            var c = kids[i]
            var b = baseOf(c)
            var want = { item: c, top: b.top, bottom: b.bottom, left: b.left, right: b.right }
            entries.push(want)
            // A list (the profiles, the settings files) is not a setting: it fills its section
            // by its anchors, and layout margins of a frame moved it off its place
            if (!window.ui.modern || !c.visible || (c.height <= 0 && c.implicitHeight <= 0)
                    || c.contentY !== undefined)
                continue
            if (isSectionLike(c)) {
                // Out to the edges of the frames around it
                want.left = b.left - framePadH
                want.right = b.right - framePadH
                want.section = true
                want.groupStart = true
                cur = null
            } else if (cur && ((cur.titleOnly && c.checked === undefined)
                               || isNote(c)
                               || (isButton(c) && (hasPicker(cur.last.item) || cur.pickerNote)))) {
                // A description or a note joins the setting above it; a lone title ("FTMS
                // Treadmill:") takes the picker below it into its frame, but not a switch: a
                // warning line above a switch is a setting of its own. A note at the top of a
                // section (no setting above) is one too. A button right under a picker, or
                // under the note of a picker ("Heart Belt Name", the Apple Watch line,
                // "Refresh Devices List"), belongs to that picker
                cur.pickerNote = isNote(c) && hasPicker(cur.last.item)
                cur.last = want
                cur.titleOnly = false
            } else {
                cur = { first: want, last: want, titleOnly: isTitle(c) }
                want.group = cur
                want.groupStart = true
            }
        }
        var list = []
        var seen = false
        for (i = 0; i < entries.length; i++) {
            var e = entries[i]
            if (!e.groupStart)
                continue
            if (e.section) {
                e.top += seen ? frameGap : 0
            } else {
                e.group.first.top += framePadV + (seen ? frameGap : 0)
                e.group.last.bottom += framePadV
                list.push({ first: e.group.first.item, last: e.group.last.item })
            }
            seen = true
        }
        for (i = 0; i < entries.length; i++) {
            setMargin(entries[i].item, "topMargin", entries[i].top)
            setMargin(entries[i].item, "bottomMargin", entries[i].bottom)
            setMargin(entries[i].item, "leftMargin", entries[i].left)
            setMargin(entries[i].item, "rightMargin", entries[i].right)
        }
        // Classic: every margin is its own again
        touched = window.ui.modern
        var same = list.length === itemFrames.length
        for (i = 0; same && i < list.length; i++)
            same = list[i].first === itemFrames[i].first && list[i].last === itemFrames[i].last
        if (!same)
            itemFrames = list
    }

    // A new content (a Loader loads it again on every opening): its margins as it comes
    // On the next turn: the content is often set while the section is still being laid out, and
    // margins changed in the middle of it made a binding loop on the height of the section
    onContentChanged: {
        baseMargins = []
        Qt.callLater(frames.regroup)
    }
    // Made by a Loader once the look turns modern, the content is often there already: the frames
    // are laid out when made as well, not only on a change of it (a second call on the same turn
    // is merged into one)
    Component.onCompleted: Qt.callLater(frames.regroup)
    // A section opened again (StaticAccordionElement keeps its content while closed): the
    // settings are visible again, the groups are made anew
    onVisibleChanged: if (visible) Qt.callLater(frames.regroup)
    Connections {
        target: frames.content
        ignoreUnknownSignals: true
        function onImplicitHeightChanged() { Qt.callLater(frames.regroup) }
    }
    Connections {
        target: window.ui
        function onModernChanged() { Qt.callLater(frames.regroup) }
    }

    // The block of an open section: its header is the top of it
    UiFrame {
        readonly property Item holder: frames.holder
        visible: window.ui.modern && frames.header !== null && holder !== null
        z: -2
        x: frames.header ? frames.header.x - holder.x - 1 : 0
        y: frames.header ? frames.header.y - holder.y - 1 : 0
        width: frames.header ? frames.header.width + 2 : 0
        height: frames.header ? holder.y + holder.height + frames.insetBottom - frames.header.y + 2 : 0
        radius: frames.header ? frames.header.radius + 1 : 0
        // A subsection: the accent toned down - clearly a block of its own, still under the section
        stroke: frames.depth === 1 ? window.ui.accent
                : Qt.rgba(window.ui.accent.r, window.ui.accent.g, window.ui.accent.b, 0.4)
        strokeWidth: 1
    }

    // A frame for every setting
    Repeater {
        model: frames.itemFrames
        UiFrame {
            z: -1
            x: -frames.framePadH
            width: (frames.holder ? frames.holder.width : 0) + 2 * frames.framePadH
            y: modelData.first.y - frames.framePadV
            height: modelData.last.y + modelData.last.height - modelData.first.y + 2 * frames.framePadV
            radius: 14
            stroke: window.ui.outline
            strokeWidth: 1
        }
    }
}
