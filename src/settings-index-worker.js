// Where every setting lives on the settings pages: the page and the chain of section titles
// around the control bound to it. Built from the
// QML sources in the resources, so it always matches the pages as they are, whatever is
// added to them later. Runs in a WorkerScript: the sources are about 1.3 MB, parsing them
// on the GUI thread would freeze the page for a moment.
//
// In:  { pages: [{ file: "settings.qml", source: "..." }, ...] }
// Out: { map: { key: { file, chain: ["Section", "Subsection"], pos: "005367.000072" } }, keys, ms }
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

function record(map, key, file, chain, strength, line) {
    var known = map[key]
    if (!known || strength > known.strength)
        map[key] = { file: file, chain: chain, strength: strength, line: line }
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
            stack.push({ depth: depth, title: "" })
        if (stack.length > 0) {
            var top = stack[stack.length - 1]
            var title = TITLE.exec(line)
            if (title && !top.title && depth === top.depth + 1)
                top.title = unescapeLiteral(title[1])
        }
        // Outside any section too: a page without sections (the voice page) has its controls
        // at the top level, with an empty chain
        {
            var linked = LINKED.exec(line)
            if (linked)
                record(map, linked[1], file, titlesOf(stack), 3, i)
            if (line.indexOf("settings.") >= 0) {
                var strength = SHOWS.test(line) ? 2 : (WRITES.test(line) ? 1 : 0)
                var chain = null
                var m
                KEY.lastIndex = 0
                while ((m = KEY.exec(line)) !== null) {
                    if (chain === null)
                        chain = titlesOf(stack)
                    // A key that only decides what the line shows is no more than a mention there
                    record(map, m[1], file, chain, strength === 2 && usedAsCondition(line, KEY.lastIndex) ? 0 : strength, i)
                }
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
        delete map[key].strength
        delete map[key].line
        keys++
    }
    WorkerScript.sendMessage({ map: map, keys: keys, ms: Date.now() - started })
}
