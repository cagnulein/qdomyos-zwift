import QtQuick 2.12
import QtQuick.Layouts 1.12
// after QtQuick: Binding.restoreMode (lineHeight of the label)
import QtQml 2.15

// A row of a settings page: a label, a list and what follows it (OK). In place of a RowLayout:
// with everything in one row it lays out as one (one column per shown item, columnSpacing =
// spacing). Modern look: the list keeps the width of its text, at most half the window
// (UiComboBox Layout.minimumWidth), and when the label then has less room than its longest word,
// or the value of the list is wider than half the window, the row is stacked - the label across
// the row, the list and the rest one line lower, the list across the free width
// (UiComboBox Layout.fillWidth). A field for a long text (UiTextField.wideText: an e-mail, a
// password, an address) is always stacked, across the free width as well. The label spans
// the row with "Layout.columnSpan: parent.stackedRow ? parent.columns : 1".
// Items with visible: false take no cell (Qt 5.15.0 QQuickLayout::shouldIgnoreItem,
// qquicklayout.cpp:771-793; QQuickGridLayout::insertLayoutItems, qquicklinearlayout.cpp:675).
GridLayout {
    id: row

    // As the "spacing" of a RowLayout. Not set - the one of the style, as a RowLayout: both take
    // QGridLayoutEngine::spacing with the style info (8 x DPI/96 on Android and iOS, 5 elsewhere;
    // Qt 5.15.0 qquicklinearlayout.cpp:529 and 838, qquicklayoutstyleinfo.cpp:51-66), and the setter
    // skips NaN (qquicklinearlayout.cpp:535)
    property real spacing: NaN
    columnSpacing: spacing
    // Stacked: the items right under the label, ~6 dp from its glyphs (~3.5 dp of the line under them)
    // to what is drawn. A field, a list and OK keep a top inset over their background (6 dp:
    // UiTextField.modernInset, UiComboBox, UiButton) - -4 with them; items with no inset - 2. A user
    // row spacing is taken as it is, a negative one too: Qt 5.15.0 qgridlayoutengine.cpp:1349-1351
    // (no clamp), :94-100 (the total height), :466-477 (the position of the next row); the same in
    // 6.8 (:1348-1349, :62-68, :446)
    readonly property real stackInset: {
        var m = -1
        for (var i = 0; i < others.length; i++) {
            var t = others[i].topInset !== undefined ? others[i].topInset : 0
            m = m < 0 ? t : Math.min(m, t)
        }
        return Math.max(0, m)
    }
    // plus what the closer lines of the label take off under its last line (labelCut, below)
    rowSpacing: stackedRow ? 2 - stackInset + labelCut : 4

    // A label in two lines or more: the lines closer, x1.15 of the font (~x1.28 by default). Qt
    // 5.15.0 sets the proportional height on every line, the first and only one too
    // (qquicktext.cpp:1263-1264), and draws from the top (:2469): beside a field a label in one line
    // would sit lower, so in a row it keeps 1. Stacked, nothing is beside it - always the closer
    // lines, and what they take off is the same for one line or more: the spacing does not read the
    // line count. It must not: setting rowSpacing asks the layout for its size at once
    // (qquicklinearlayout.cpp:557-561, 386-388), a Text then lays itself out (qquicktext.cpp:139-147)
    // and emits lineCountChanged (:1187-1189) while the spacing is still being set - Qt drops that
    // second update ("Binding loop detected" on opening General Options at 170 %), and the spacing
    // could stay the one of the other line count
    readonly property real stackedLeading: 0.9
    readonly property real labelLeading: window.ui.modern && label !== null
                                         && (stackedRow || label.lineCount > 1) ? stackedLeading : 1
    Binding {
        target: row.label
        property: "lineHeight"
        value: row.labelLeading
        when: row.label !== null
        // a label hidden or replaced (a collapsed section: label null) gets its 1 back, without the
        // warning of the default mode on every collapse (Qt 5.15.0 qqmlbind.cpp:494-505)
        restoreMode: Binding.RestoreBindingOrValue
    }
    // what the closer lines take off under the last baseline: (1 - lineHeight) x the line height,
    // which Qt rounds up (qquicktext.cpp:1273); stacked, given back to keep the item ~6 dp under the
    // glyphs. Only the stacked leading, not labelLeading: that one reads the line count (above)
    readonly property real labelCut: (1 - stackedLeading) * Math.ceil(metrics.height)

    // the first shown label (a text that wraps) and the other shown items of the row
    readonly property Item label: {
        for (var i = 0; i < children.length; i++) {
            var c = children[i]
            if (c.visible && c.wrapMode !== undefined && c.text !== undefined)
                return c
        }
        return null
    }
    readonly property var others: {
        var list = []
        for (var i = 0; i < children.length; i++)
            if (children[i].visible && children[i] !== label)
                list.push(children[i])
        return list
    }

    FontMetrics {
        id: metrics
        font: row.label ? row.label.font : Qt.application.font
    }

    // the width of the longest word of the label: less room than that cuts the word
    readonly property real wordWidth: {
        // advanceWidth() is a call: the binding does not see the font it measures with. Read here,
        // the font makes a change of the text size on the open page measure again (the word kept its
        // width at 100 % after 130 % was set, and the items beside it ran over the label)
        var font = metrics.font
        if (!label || !font)
            return 0
        // a word ends at a space; a CJK character (from U+2E80) is a word of its own - the line
        // can break after any of them, so a Chinese or Japanese label with no spaces is not one word
        var text = String(label.text)
        var w = 0
        var word = ""
        for (var i = 0; i <= text.length; i++) {
            var c = i < text.length ? text.charAt(i) : " "
            var cjk = c.charCodeAt(0) >= 0x2E80
            if (c !== " " && !cjk) {
                word += c
                continue
            }
            w = Math.max(w, metrics.advanceWidth(word))
            if (cjk)
                w = Math.max(w, metrics.advanceWidth(c))
            word = ""
        }
        return Math.ceil(w) + (label.leftPadding || 0) + (label.rightPadding || 0)
    }
    // a list of the modern look on a settings page (UiComboBox): in a row it takes at most half the
    // window (its Layout.minimumWidth); the other items (OK) keep their own width. UiButton has
    // inTypeScalePage too: an OK is narrow, and a button wider than half the window is better a line
    // lower as well
    function isList(item) {
        return item.inTypeScalePage !== undefined
    }
    // what the other items take in one row
    readonly property real othersWidth: {
        var w = 0
        for (var i = 0; i < others.length; i++)
            w += (isList(others[i]) ? Math.min(others[i].implicitWidth, window.width / 2)
                                    : others[i].implicitWidth) + columnSpacing
        return w
    }
    // a list whose value is wider than half the window: in a row it was cut; stacked it gets the row
    readonly property bool listCut: {
        for (var i = 0; i < others.length; i++)
            if (isList(others[i]) && others[i].implicitWidth > window.width / 2)
                return true
        return false
    }
    // a field for a long text (UiTextField.wideText: an e-mail, a login, a password, an address) -
    // always under the label, across the row
    readonly property bool wideField: {
        for (var i = 0; i < others.length; i++)
            if (others[i].wideText === true)
                return true
        return false
    }
    // the width the row gets: the one of its column (its own follows the stacking) less its side
    // margins, when the row has them (Layout.left/rightMargin; the rows of a card have 0 - the column
    // is already the inside of the card)
    readonly property real roomWidth: (parent ? parent.width : width) - Layout.leftMargin - Layout.rightMargin

    readonly property bool stackedRow: window.ui.modern && label !== null && others.length > 0
                                       && (wordWidth + othersWidth > roomWidth || listCut || wideField)
    columns: Math.max(1, stackedRow ? others.length : others.length + (label ? 1 : 0))
}
