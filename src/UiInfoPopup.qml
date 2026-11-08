import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12

// A notice with nothing to answer ("Your Strava account is now connected!", "Trial time
// expired!" ...). Classic look: the popup as before - fixed size, white text,
// the line breaks of the text; with a bigger text it wraps and grows. Modern look: the card of UiMessageDialog with an OK button,
// the text in the theme colours and wrapped by the card width.
UiPopup {
    id: control

    property string title: ""
    property string text: ""
    // Classic look: the sizes of the popup and of its label as they were (0 = implicit)
    property int classicWidth: 380
    property int classicHeight: 130
    property int classicLabelWidth: 0
    property int classicLabelHeight: 0
    // Modern look: the addresses in the text open the browser. Off where the address is only
    // named ("not affiliated with https://whatsonzwift.com/")
    property bool links: true

    // The texts break their lines by hand (\n, <br>) for the fixed classic width: in the card
    // the single breaks become spaces and only the empty lines between paragraphs stay
    function flowText(s) {
        // <br>, <br/>, <br /> -> line break
        var t = ""
        var i = 0
        while (i < s.length) {
            if (s.substr(i, 3) === "<br") {
                var j = i + 3
                while (j < s.length && (s[j] === " " || s[j] === "\t" || s[j] === "\n" || s[j] === "\r"))
                    j++
                if (s[j] === "/")
                    j++
                if (s[j] === ">") {
                    t += "\n"
                    i = j + 1
                    continue
                }
            }
            t += s[i]
            i++
        }
        // a single line break between two lines -> space; empty lines stay; runs of spaces -> one
        var out = ""
        for (var k = 0; k < t.length; k++) {
            var c = t[k]
            if (c === "\n" && k > 0 && t[k - 1] !== "\n" && t[k + 1] !== "\n")
                c = " "
            if (c === " " && out.length > 0 && out[out.length - 1] === " ")
                continue
            out += c
        }
        return out.trim()
    }

    // The card text as StyledText: the markup characters escaped, the addresses made links
    function linkText(s) {
        var t = flowText(s).split("&").join("&amp;").split("<").join("&lt;").split(">").join("&gt;")
        if (links)
            t = linkAddresses(t)
        return t.split("\n").join("<br>")
    }

    // http: and https: addresses -> links; an address ends at a space or a line break, without
    // the . , ! ? that close the sentence after it
    function linkAddresses(t) {
        var out = ""
        var i = 0
        while (i < t.length) {
            var start = t.indexOf("http", i)
            if (start < 0)
                break
            var prefix = t.substr(start, 6) === "https:" ? 6 : t.substr(start, 5) === "http:" ? 5 : 0
            var end = start + prefix
            if (prefix > 0) {
                while (end < t.length && t[end] !== " " && t[end] !== "\n")
                    end++
                while (end > start + prefix && ".,!?".indexOf(t[end - 1]) >= 0)
                    end--
            }
            // at least two characters after "http(s):", the last one not . , ! ?
            if (prefix === 0 || end - start - prefix < 2) {
                out += t.substring(i, start + 4)
                i = start + 4
                continue
            }
            var address = t.substring(start, end)
            out += t.substring(i, start) + '<a href="' + address + '">' + address + "</a>"
            i = end
        }
        return out + t.substring(i)
    }

    parent: Overlay.overlay
    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    width: modern ? Math.min(parent.width - 48, 440)
                  : window.textScale !== 1 ? Math.min(classicWidth, parent.width - 32) : classicWidth
    height: modern ? implicitHeight
                   : window.textScale !== 1 ? Math.max(classicHeight, classicColumn.implicitHeight + topPadding + bottomPadding)
                   : classicHeight
    modal: true
    focus: true
    palette.text: "white"
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    padding: modern ? 24 : 12
    bottomPadding: 12

    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
    }

    contentItem: Item {
        implicitHeight: control.modern ? modernColumn.implicitHeight : classicColumn.implicitHeight

        Column {
            id: classicColumn
            visible: !control.modern
            anchors.horizontalCenter: parent.horizontalCenter
            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                width: window.textScale !== 1 ? control.availableWidth
                       : control.classicLabelWidth > 0 ? control.classicLabelWidth : implicitWidth
                height: window.textScale === 1 && control.classicLabelHeight > 0 ? control.classicLabelHeight : implicitHeight
                text: control.text
                wrapMode: window.labelWrap
            }
        }

        Column {
            id: modernColumn
            visible: control.modern
            width: parent.width
            spacing: 12

            Label {
                width: parent.width
                visible: control.title.length > 0
                text: control.title
                wrapMode: Text.WordWrap
                font.pixelSize: window.ui.fontTitle
                font.weight: Font.DemiBold
                color: window.ui.textMain
            }

            Flickable {
                id: bodyFlick
                width: parent.width
                height: Math.min(body.implicitHeight, control.parent.height * 0.55)
                contentHeight: body.implicitHeight
                clip: contentHeight > height
                boundsBehavior: Flickable.StopAtBounds
                Label {
                    id: body
                    width: parent.width
                    text: control.linkText(control.text)
                    textFormat: Text.StyledText
                    linkColor: Material.accent
                    onLinkActivated: Qt.openUrlExternally(link)
                    wrapMode: Text.Wrap
                    font.pixelSize: window.ui.fontBody
                    lineHeight: 1.15
                    color: control.title.length > 0 ? window.ui.textMuted : window.ui.textMain
                }
                // A long text: the bar stays to show there is more below
                ScrollIndicator.vertical: ScrollIndicator {
                    active: bodyFlick.contentHeight > bodyFlick.height
                }
            }

            UiButton {
                anchors.right: parent.right
                // the OK of the settings pages, already translated
                text: qsTranslate("settings", "OK")
                flat: true
                onClicked: control.close()
            }
        }
    }
}
