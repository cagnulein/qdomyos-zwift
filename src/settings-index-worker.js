// Where every setting lives on the settings pages: the page and the chain of section titles
// around the control bound to it. Built from the
// QML sources in the resources, so it always matches the pages as they are, whatever is
// added to them later. Runs in a WorkerScript: the sources are about 1.3 MB, parsing them
// on the GUI thread would freeze the page for a moment.
//
// In:  { pages: [{ file: "settings.qml", source: "..." }, ...] }
// Out: { map: { key: { file, chain: ["Section", "Subsection"], pos: "005367.000072", own, card } }, keys, ms }
// own (only when true): the setting is the switch in the header of the last section of the chain
// card (only when inside one): the index in the chain of the innermost section shown as a card
// (card: true - a tile on the Tiles page), a section about one thing rather than a group
// The titles are the English sources of qsTr(); the page translates them with its context.
// pos is where the setting stands as the pages are read, top to bottom: its line on the main
// page, or the line of the accordionContent that opens its page there, then its line on that
// page. Fixed-width numbers, so comparing the strings gives the order of the pages.

// AccordionElement, AccordionCheckElement, StaticAccordionElement
var ELEMENT = /\b\w*Accordion\w*Element\s*\{/
// title: qsTr("...") or a plain title: "..."
var TITLE = /^\s*title\s*:\s*(?:qsTr\()?"((?:[^"\\]|\\.)*)"/
var LINKED = /^\s*linkedBoolSetting\s*:\s*"([A-Za-z0-9_]+)"/
var KEY = /\bsettings\.([A-Za-z_][A-Za-z0-9_]*)/g
// The line that shows the value is the control itself; an assignment comes next (a handler of
// the control); any other mention (a visible: of some other row, say) is the weakest hint
var SHOWS = /^\s*(checked|text|savedText|savedIndex|currentIndex|value|displayText)\s*:/
var WRITES = /\bsettings\.[A-Za-z0-9_]+\s*=[^=]/

// The look of the modern interface is written through main.qml, not through settings.<key>:
// settingsPane.setUiOption("ui_theme", ...). The key of such a line, or "" when it has none
function uiOptionKey(line) {
    var call = "setUiOption("
    var at = line.indexOf(call)
    if (at < 0)
        return ""
    var i = at + call.length
    while (i < line.length && (line.charAt(i) === " " || line.charAt(i) === "\t"))
        i++
    if (line.charAt(i) !== "\"")
        return ""
    var end = line.indexOf("\"", i + 1)
    if (end < 0)
        return ""
    var key = line.substring(i + 1, end)
    for (var k = 0; k < key.length; k++) {
        var c = key.charAt(k)
        var ok = (c >= "a" && c <= "z") || (c >= "A" && c <= "Z") || (c >= "0" && c <= "9") || c === "_"
        if (!ok)
            return ""
    }
    return key
}

// Braces of a line outside string literals and // comments
function braceDelta(line) {
    if (line.indexOf("{") < 0 && line.indexOf("}") < 0)
        return 0
    var delta = 0
    var quote = ""
    for (var i = 0; i < line.length; i++) {
        var c = line.charAt(i)
        if (quote) {
            if (c === "\\")
                i++
            else if (c === quote)
                quote = ""
        } else if (c === "\"" || c === "'") {
            quote = c
        } else if (c === "/" && line.charAt(i + 1) === "/") {
            break
        } else if (c === "{") {
            delta++
        } else if (c === "}") {
            delta--
        }
    }
    return delta
}

function unescapeLiteral(s) {
    return s.replace(/\\(.)/g, "$1")
}

function titlesOf(stack) {
    var chain = []
    for (var i = 0; i < stack.length; i++)
        if (stack[i].title)
            chain.push(stack[i].title)
    return chain
}

// card: true of a section, on its own line ("card: true" or "card: true // ...")
function isCardLine(line) {
    var text = line.trim()
    if (text.indexOf("card") !== 0)
        return false
    text = text.substring(4).trim()
    if (text.charAt(0) !== ":")
        return false
    text = text.substring(1).trim()
    return text === "true" || text.indexOf("true ") === 0 || text.indexOf("true//") === 0
}

// own: the setting is the switch in the header of the last section of its chain (linkedBoolSetting
// of an AccordionCheckElement - a tile, Wahoo direct connect): the card of a tile shows its switch
// under the tile's own title (settings.qml modernTitle). sections: the open sections themselves,
// their card: true may come on a line after the setting
function record(map, key, file, stack, strength, line, own) {
    var known = map[key]
    if (!known || strength > known.strength) {
        map[key] = { file: file, chain: titlesOf(stack), strength: strength, line: line,
                     sections: stack.slice() }
        if (own)
            map[key].own = true
    }
}

// The index in the chain of the innermost card section, -1 outside one
function cardLevel(sections) {
    var level = -1
    var titled = 0
    for (var i = 0; i < sections.length; i++) {
        if (!sections[i].title)
            continue
        if (sections[i].card)
            level = titled
        titled++
    }
    return level
}

// The key ending at `end` is a condition: "(settings.miles_unit && ...) ? lbs : kg" in the label of
// Player Weight shows the weight, not the unit switch further down the page
function usedAsCondition(line, end) {
    var k = end
    while (k < line.length && (line.charAt(k) === " " || line.charAt(k) === ")"))
        k++
    var next = line.substring(k, k + 2)
    return next.charAt(0) === "?" || next === "&&" || next === "||"
}

var MAIN_PAGE = "settings.qml"
// The pages that are on no page come after all of them
var NO_ANCHOR = "999999"

function padLine(line) {
    var s = String(line)
    while (s.length < 6)
        s = "0" + s
    return s
}

// The pages opened from another page: { "settings-tiles.qml": { file: "settings.qml", line: 7689 } }
function pageAnchors(pages) {
    var anchors = {}
    for (var i = 0; i < pages.length; i++) {
        var lines = withoutBlockComments(pages[i].source || "").split("\n")
        for (var n = 0; n < lines.length; n++) {
            var at = lines[n].indexOf("accordionContent:")
            if (at < 0)
                continue
            var open = lines[n].indexOf("\"", at)
            var close = open < 0 ? -1 : lines[n].indexOf("\"", open + 1)
            if (close < 0)
                continue
            var target = lines[n].substring(open + 1, close)
            if (target !== MAIN_PAGE && !anchors[target])
                anchors[target] = { file: pages[i].file, line: n }
        }
    }
    return anchors
}

// "005367." for a page opened at line 5367 of the main page, "" for the main page itself
function pagePrefix(file, anchors, depth) {
    if (file === MAIN_PAGE)
        return ""
    var anchor = anchors[file]
    if (!anchor || depth > 8)
        return NO_ANCHOR + "."
    return pagePrefix(anchor.file, anchors, depth + 1) + padLine(anchor.line) + "."
}

// Block comments out, line breaks kept: commented-out controls are not where a setting lives
function withoutBlockComments(source) {
    return source.replace(/\/\*[\s\S]*?\*\//g, function (comment) {
        return comment.replace(/[^\n]/g, " ")
    })
}

function parse(file, source, map) {
    var lines = withoutBlockComments(source).split("\n")
    var stack = []
    var depth = 0
    for (var i = 0; i < lines.length; i++) {
        var line = lines[i]
        if (ELEMENT.test(line))
            stack.push({ depth: depth, title: "", card: false })
        if (stack.length > 0) {
            var top = stack[stack.length - 1]
            var title = TITLE.exec(line)
            if (title && !top.title && depth === top.depth + 1)
                top.title = unescapeLiteral(title[1])
            if (depth === top.depth + 1 && isCardLine(line))
                top.card = true
        }
        // Outside any section too: a page without sections (the voice page) has its controls
        // at the top level, with an empty chain
        {
            var linked = LINKED.exec(line)
            // own: a property of the innermost section, whose title is already in the chain
            // (title: above it)
            if (linked)
                record(map, linked[1], file, stack, 3, i,
                       stack.length > 0 && depth === stack[stack.length - 1].depth + 1 &&
                       !!stack[stack.length - 1].title)
            var uiOption = uiOptionKey(line)
            if (uiOption)
                record(map, uiOption, file, stack, 1, i)
            if (line.indexOf("settings.") >= 0) {
                var strength = SHOWS.test(line) ? 2 : (WRITES.test(line) ? 1 : 0)
                var m
                KEY.lastIndex = 0
                // A key that only decides what the line shows is no more than a mention there
                while ((m = KEY.exec(line)) !== null)
                    record(map, m[1], file, stack, strength === 2 && usedAsCondition(line, KEY.lastIndex) ? 0 : strength, i)
            }
        }
        depth += braceDelta(line)
        while (stack.length > 0 && depth <= stack[stack.length - 1].depth)
            stack.pop()
    }
}

WorkerScript.onMessage = function (message) {
    var started = Date.now()
    var map = {}
    // The pages arrive in the order their requests finished; read in a fixed order, the main
    // page first, a setting mentioned equally on two pages gets the same place every time
    var pages = (message.pages || []).slice().sort(function (a, b) {
        if ((a.file === MAIN_PAGE) !== (b.file === MAIN_PAGE))
            return a.file === MAIN_PAGE ? -1 : 1
        return a.file < b.file ? -1 : (a.file > b.file ? 1 : 0)
    })
    // Always answer: without a map the page keeps the catalog paths, but it must stop waiting
    try {
        for (var i = 0; i < pages.length; i++)
            parse(pages[i].file, pages[i].source || "", map)
    } catch (e) {
        map = {}
    }
    var anchors = {}
    try {
        anchors = pageAnchors(pages)
    } catch (e) {
        anchors = {}
    }
    var keys = 0
    for (var key in map) {
        // Without a place the setting goes after the rest, in the catalog order
        try {
            map[key].pos = pagePrefix(map[key].file, anchors, 0) + padLine(map[key].line)
        } catch (e) {
            delete map[key].pos
        }
        var card = cardLevel(map[key].sections || [])
        if (card >= 0)
            map[key].card = card
        delete map[key].sections
        delete map[key].strength
        delete map[key].line
        keys++
    }
    WorkerScript.sendMessage({ map: map, keys: keys, ms: Date.now() - started })
}
