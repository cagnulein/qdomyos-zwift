// Where every setting lives on the settings pages: the page and the chain of section titles
// around the control bound to it. Built from the
// QML sources in the resources, so it always matches the pages as they are, whatever is
// added to them later. Runs in a WorkerScript: the sources are about 1.3 MB, parsing them
// on the GUI thread would freeze the page for a moment.
//
// In:  { pages: [{ file: "settings.qml", source: "..." }, ...] }
// Out: { map: { key: { file, chain: ["Section", "Subsection"] } }, keys, ms }
// The titles are the English sources of qsTr(); the page translates them with its context.

// AccordionElement, AccordionCheckElement, StaticAccordionElement
var ELEMENT = /\b\w*Accordion\w*Element\s*\{/
// title: qsTr("...") or a plain title: "..."
var TITLE = /^\s*title\s*:\s*(?:qsTr\()?"((?:[^"\\]|\\.)*)"/
var LINKED = /^\s*linkedBoolSetting\s*:\s*"([A-Za-z0-9_]+)"/
var KEY = /\bsettings\.([A-Za-z_][A-Za-z0-9_]*)/g
// The line that shows the value is the control itself; an assignment comes next (a handler of
// the control); any other mention (a visible: of some other row, say) is the weakest hint
var SHOWS = /^\s*(checked|text|currentIndex|value|displayText)\s*:/
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

function record(map, key, file, chain, strength) {
    var known = map[key]
    if (!known || strength > known.strength)
        map[key] = { file: file, chain: chain, strength: strength }
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
                record(map, linked[1], file, titlesOf(stack), 3)
            if (line.indexOf("settings.") >= 0) {
                var strength = SHOWS.test(line) ? 2 : (WRITES.test(line) ? 1 : 0)
                var chain = null
                var m
                KEY.lastIndex = 0
                while ((m = KEY.exec(line)) !== null) {
                    if (chain === null)
                        chain = titlesOf(stack)
                    record(map, m[1], file, chain, strength)
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
    var pages = message.pages || []
    // Always answer: without a map the page keeps the catalog paths, but it must stop waiting
    try {
        for (var i = 0; i < pages.length; i++)
            parse(pages[i].file, pages[i].source || "", map)
    } catch (e) {
        map = {}
    }
    var keys = 0
    for (var key in map) {
        delete map[key].strength
        keys++
    }
    WorkerScript.sendMessage({ map: map, keys: keys, ms: Date.now() - started })
}
