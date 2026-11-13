#!/usr/bin/env python3
"""Check that QML changed by a pull request stays usable with TalkBack/VoiceOver.

A control the screen reader cannot name or press is also out of reach for voice control
("tap Settings" finds a control by its name). For every QML element the pull request adds or
changes, the check reports:

Errors (exit code 1):
  no-name       a control without a name: a button without `text` and `Accessible.name` (only an
                icon or its own contentItem, or `text` empty in one branch of a condition:
                `text: modern ? "" : "<"`); a field, list, spin box or slider without
                `Accessible.name` - the reader then says only the value ("0", "Disabled") and
                not which setting it is
  symbol-text   the name of a button (`text` without `Accessible.name`, or `Accessible.name`
                itself) is a symbol with no letters or digits ("<", "+", "x" sign, an emoji): the
                reader reads out the symbol
  no-qstr       `Accessible.name`/`Accessible.description` has a string outside qsTr(): it stays
                English in every language
  no-action     MenuItem and the delegates (ItemDelegate, SwipeDelegate, Switch/Check/
                RadioDelegate) without `Accessible.onPressAction`: a double tap of the reader
                does not emit clicked() - in Qt 5.15 the role of MenuItem and Item/Swipe/
                SwitchDelegate has no press action, and Check/RadioDelegate only flip `checked`;
                the same for any item with `Accessible.role: Accessible.Button` and no
                `onPressAction`

Warnings (printed, but do not fail the check):
  no-action     the same for an instance of an own component (e.g. a settings switch based on
                SwitchDelegate): the component is old code, and one fix there covers all its
                instances; a component that loses its action is still an error on its root
  tap-only      a MouseArea/TapHandler with a click handler on an item without
                `Accessible.role` and `onPressAction`: only a finger can press it. Sometimes this
                is fine (tap outside to close), so it is a warning
  placeholder   a field with `placeholderText` but no `Accessible.name`: Qt reads the
                placeholder as a description, not as the name
  name-no-value a field, list or spin box whose `Accessible.name` does not include its value:
                Qt 5.15 on Android reads the value only when the name is empty, so the typed
                value is not read (fix: `Accessible.name: qsTr("Weight") + ": " + text`); not
                for a password field (`echoMode: TextInput.Password`), whose value must stay
                silent
  glyph-text    a Text/Label of symbols only (no letters or digits) outside a button, without
                `Accessible.ignored: true`: the reader stops on it
  not-parsed    a QML file the checker could not parse, so it was not checked

Only new findings are reported. An element is checked when one of its own lines (or the line of
its closing brace) is changed by the diff from the merge base, as on GitHub. Each changed element
is paired with the same element of the base version (by its unchanged lines, then by id, text or
icon, then by its place); a finding its pair already had there is old and stays silent. So
changing the text or the handler of an old control does not report its old problems.

Also checked: a MouseArea/TapHandler up to 3 levels below a changed element (a removed
Accessible.role of a parent leaves it without an action) and, when the root of a component is
changed (the file or `component X: ...`), its instances in all files. The type of own components
(e.g. a button based on Button) is taken from the root of their .qml, with its Accessible.*
properties: a component that names itself does not need a name on each instance.

Not reported:
  - a missing name when the name is set from script: `id.text = ...`, `id.Accessible.name = ...`,
    a `text = ...` in the element's own handler, `Binding { target: id; property: "text" }`,
    `Binding on text { ... }` (a symbol in the literal `text` is still reported);
  - names from an expression (`text: model.label`, `Accessible.name: title + qsTr(" settings")`);
    only plain strings are checked for qsTr, and a string compared with `===`, a format or a key
    (`Qt.formatDate(d, "dddd")`, `names["home"]`) is not a text;
  - elements with `visible: false` (and their children) or `Accessible.ignored: true`;
  - QML tests in tst/.
A false finding goes into tools/check_accessibility_exceptions.json with the reason.

What it cannot see (only a device shows it): the reading order, a Popup placed at the end of the
tree, and names that are set but not updated.

Usage (from the repository root):
  python3 tools/check_accessibility.py [base] [--head REV] [--all] [--no-baseline] [--github]
                                       [--repo DIR] [--exceptions FILE] [--files src/a.qml ...]
  base           compare with this revision (default origin/master); the diff starts at the
                 merge base of `base` and the checked version
  --head REV     check REV instead of the working tree
  --all          every element of every QML file (or of --files), old ones too
  --no-baseline  every finding of the changed elements, old ones too
  --github       also print GitHub annotations, so findings show in the "Files changed" tab
Tests: python3 -m unittest tests/test_check_accessibility.py

No regular expressions: the QML is read by a short tokenizer.
"""
import json
import subprocess
import sys
from pathlib import Path

# Pressable: the name comes from `text`
TEXT_CONTROLS = {
    "AbstractButton", "Button", "ToolButton", "RoundButton", "DelayButton", "TabButton",
    "CheckBox", "RadioButton", "Switch", "MenuItem", "MenuBarItem", "ItemDelegate",
    "SwipeDelegate", "CheckDelegate", "RadioDelegate", "SwitchDelegate",
}
# Qt 5.15 gives them the role MenuItem/ListItem, which has no press action
NEED_ACTION = {"MenuItem", "ItemDelegate", "SwipeDelegate", "CheckDelegate", "RadioDelegate",
               "SwitchDelegate"}
# Editable: `text` is the value, not the name
INPUT_CONTROLS = {"TextField", "TextArea", "ComboBox", "SpinBox", "Slider", "RangeSlider", "Dial",
                  "Tumbler", "TextInput", "TextEdit"}
VALUE_REFS = {"text", "displayText", "currentText", "currentIndex", "currentValue", "value",
              "textFromValue", "textAt", "first", "second"}
# A long press does not count: TalkBack passes it to the app (double tap and hold)
TAP_HANDLERS = {"onClicked", "onDoubleClicked", "onTapped", "onSingleTapped", "onDoubleTapped"}
# onPressed/onReleased without onClicked is a press, unless it is a drag
PRESS_HANDLERS = {"onPressed", "onReleased"}
DRAG_SIGNS = {"drag.target", "onPositionChanged", "onMouseXChanged", "onMouseYChanged"}
TRANSLATE = {"qsTr", "qsTrId", "qsTranslate", "QT_TR_NOOP", "QT_TRANSLATE_NOOP"}
# After these a "/" starts a JavaScript regular expression literal, not a division
REGEX_AFTER = {"(", ",", "=", ":", "[", "!", "&", "|", "?", "{", "}", ";", "&&", "||", "=>",
               "==", "===", "!=", "!==", "+", "-", "*", "%", "<", ">", "~", "^", "??"}
REGEX_AFTER_WORDS = {"return", "else", "typeof", "case", "in", "of", "void", "delete", "instanceof",
                     "new", "throw", "yield", "await", "do"}
PUNCT3 = {"===", "!==", "...", ">>>", "**="}
PUNCT2 = {"==", "!=", "<=", ">=", "&&", "||", "=>", "++", "--", "+=", "-=", "*=", "/=", "%=",
          "<<", ">>", "?.", "??", "**", "|=", "&=", "^="}
# A value goes on to the next line when a line ends with one of these, or the next starts with one
CONTINUE = {"?", ":", "+", "-", "*", "/", "%", "&&", "||", "??", ".", "?.", "==", "===", "!=",
            "!==", "<", ">", "<=", ">=", "|", "&", "^", "=>", ","}

EXCEPTIONS = Path(__file__).with_name("check_accessibility_exceptions.json")


class ParseError(Exception):
    pass


class Tok:
    __slots__ = ("kind", "text", "line")

    def __init__(self, kind, text, line):
        self.kind, self.text, self.line = kind, text, line

    def __repr__(self):
        return f"{self.kind}:{self.text}@{self.line}"


def tokenize(src):
    """Tokens str, regex, ident, num, punct with their line; comments and spaces are dropped."""
    toks, i, line, n = [], 0, 1, len(src)
    while i < n:
        c = src[i]
        if c == "\n":
            line += 1
            i += 1
        elif c in " \t\r﻿":
            i += 1
        elif src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            if j < 0:
                raise ParseError(f"line {line}: comment not closed")
            line += src.count("\n", i, j)
            i = j + 2
        elif c in "\"'`":
            j, start = i + 1, line
            while j < n and src[j] != c:
                if src[j] == "\\":
                    j += 1
                if j < n and src[j] == "\n":   # also after a "\" at the end of a line
                    line += 1
                j += 1
            if j >= n:
                raise ParseError(f"line {start}: string not closed")
            toks.append(Tok("str", src[i + 1:j], start))
            i = j + 1
        elif c == "/" and (not toks or (toks[-1].kind == "punct" and toks[-1].text in REGEX_AFTER)
                           or (toks[-1].kind == "ident" and toks[-1].text in REGEX_AFTER_WORDS
                               and not (len(toks) > 1 and toks[-2].text == "."))):
            # JavaScript regular expression literal /.../flags; a "/" inside [] does not end it
            j, in_class = i + 1, False
            while j < n and src[j] != "\n":
                if src[j] == "\\" and src[j + 1:j + 2] != "\n":
                    j += 1
                elif src[j] == "[":
                    in_class = True
                elif src[j] == "]":
                    in_class = False
                elif src[j] == "/" and not in_class:
                    break
                j += 1
            if j >= n or src[j] != "/":
                # not closed on this line: it was a division
                toks.append(Tok("punct", "/", line))
                i += 1
                continue
            j += 1
            while j < n and src[j].isalnum():
                j += 1
            toks.append(Tok("regex", src[i:j], line))
            i = j
        elif c.isalpha() or c in "_$":
            j = i + 1
            while j < n and (src[j].isalnum() or src[j] in "_$"):
                j += 1
            toks.append(Tok("ident", src[i:j], line))
            i = j
        elif c.isdigit():
            j = i + 1
            while j < n and (src[j].isalnum() or src[j] == "."):
                j += 1
            toks.append(Tok("num", src[i:j], line))
            i = j
        else:
            for size, group in ((3, PUNCT3), (2, PUNCT2)):
                if src[i:i + size] in group:
                    toks.append(Tok("punct", src[i:i + size], line))
                    i += size
                    break
            else:
                toks.append(Tok("punct", c, line))
                i += 1
    return toks


class Elem:
    def __init__(self, type_name, line, parent, slot):
        self.type = type_name.split(".")[-1]
        self.full_type = type_name
        self.line = line
        self.end = line
        self.parent = parent
        self.slot = slot          # property the element is assigned to (contentItem, delegate...)
        self.props = {}           # name -> (value tokens, line)
        self.own_lines = {line}
        self.children = []
        self.id = None
        self.component = None     # name of an inline component (component X: Button { ... })
        self.script_text = False  # `text` is assigned from script
        self.script_name = False  # `Accessible.name` is assigned from script
        self.binding_on = None    # target property of `Binding on <property> { ... }`

    def value(self, name):
        p = self.props.get(name)
        return p[0] if p else None

    def text_of(self, name):
        toks = self.value(name)
        return None if toks is None else " ".join(t.text if t.kind != "str" else '"' + t.text + '"' for t in toks)

    def lines_text(self, src_lines):
        return {src_lines[ln - 1].strip() for ln in self.own_lines if 0 < ln <= len(src_lines)}

    def walk(self):
        yield self
        for ch in self.children:
            yield from ch.walk()


def is_type_name(name):
    return name.split(".")[-1][:1].isupper()


class Parser:
    def __init__(self, toks):
        self.t, self.i = toks, 0

    def peek(self, k=0):
        j = self.i + k
        return self.t[j] if j < len(self.t) else None

    def next(self):
        tok = self.peek()
        if tok is None:
            raise ParseError("unexpected end of file")
        self.i += 1
        return tok

    def expect(self, text):
        tok = self.next()
        if tok.text != text:
            raise ParseError(f"line {tok.line}: expected '{text}', found '{tok.text}'")
        return tok

    def dotted(self):
        first = self.next()
        if first.kind != "ident":
            raise ParseError(f"line {first.line}: expected a name, found '{first.text}'")
        name = first.text
        while self.peek() and self.peek().text == "." and self.peek(1) and self.peek(1).kind == "ident":
            self.i += 1
            name += "." + self.next().text
        return name, first.line

    def skip_balanced(self):
        """Tokens from an opening bracket to its closing one."""
        pairs = {"{": "}", "(": ")", "[": "]"}
        stack, out = [], []
        while True:
            tok = self.next()
            out.append(tok)
            if tok.kind == "punct" and tok.text in pairs:
                stack.append(pairs[tok.text])
            elif tok.kind == "punct" and tok.text in ")]}":
                if not stack or stack.pop() != tok.text:
                    raise ParseError(f"line {tok.line}: unbalanced '{tok.text}'")
                if not stack:
                    return out

    def document(self):
        while self.peek() and self.peek().text in ("import", "pragma"):
            line = self.next().line
            while self.peek() and self.peek().line == line:
                self.i += 1
        if self.peek() is None:
            raise ParseError("no element in the file")
        name, line = self.dotted()
        root = self.obj(name, line, None, None)
        if self.peek() is not None:
            raise ParseError(f"line {self.peek().line}: text after the root element")
        return root

    def obj(self, name, line, parent, slot):
        el = Elem(name, line, parent, slot)
        self.expect("{")
        self.members(el, "")
        el.end = self.expect("}").line
        # the closing brace too: properties removed after the children sit next to it
        el.own_lines.add(el.end)
        return el

    def members(self, el, prefix):
        while True:
            tok = self.peek()
            if tok is None:
                raise ParseError("unexpected end of file inside an element")
            if tok.text == "}":
                return
            if tok.text in (";", ","):
                self.i += 1
                continue
            if tok.kind != "ident":
                raise ParseError(f"line {tok.line}: unexpected '{tok.text}'")
            if tok.text in ("readonly", "default", "required") and self.peek(1) and self.peek(1).text in ("property", "readonly", "default", "required"):
                self.i += 1
                continue
            if tok.text == "property" and self.peek(1) and self.peek(1).kind == "ident" and self.peek(1).text != ":":
                self.property_decl(el)
                continue
            if tok.text == "signal" and self.peek(1) and self.peek(1).kind == "ident":
                self.i += 2
                if self.peek() and self.peek().text == "(":
                    self.skip_balanced()
                continue
            if tok.text == "function" and self.peek(1) and self.peek(1).kind == "ident":
                self.i += 2
                self.skip_balanced()             # (arguments)
                while self.peek().text != "{":   # : return type
                    self.i += 1
                self.skip_balanced()
                continue
            if tok.text == "enum" and self.peek(1) and self.peek(1).kind == "ident":
                self.i += 2
                self.skip_balanced()
                continue
            if tok.text == "component" and self.peek(1) and self.peek(1).kind == "ident" and self.peek(2) and self.peek(2).text == ":":
                comp = self.peek(1).text
                self.i += 3
                name, line = self.dotted()
                child = self.obj(name, line, el, None)
                child.component = comp
                el.children.append(child)
                continue
            name, line = self.dotted()
            nxt = self.peek()
            if nxt is None:
                raise ParseError(f"line {line}: file ends after '{name}'")
            if nxt.text == "{":
                if is_type_name(name):
                    el.children.append(self.obj(name, line, el, None))
                else:                            # font { bold: true }
                    el.own_lines.add(line)
                    self.i += 1
                    self.members(el, prefix + name + ".")
                    el.own_lines.add(self.expect("}").line)
                continue
            if nxt.text == "on" and is_type_name(name):  # Behavior on x { }
                self.i += 1
                target, _ = self.dotted()
                child = self.obj(name, line, el, None)
                child.binding_on = target
                el.children.append(child)
                continue
            if nxt.text == ":":
                self.i += 1
                self.assign(el, prefix + name, line)
                continue
            raise ParseError(f"line {line}: unexpected '{nxt.text}' after '{name}'")

    def property_decl(self, el):
        self.i += 1                              # property
        self.next()                              # type
        if self.peek().text == "<":              # list<Item>
            while self.next().text != ">":
                pass
        name_tok = self.next()
        el.own_lines.add(name_tok.line)
        if self.peek() and self.peek().text == ":":
            self.i += 1
            self.assign(el, name_tok.text, name_tok.line)

    def assign(self, el, name, line):
        el.own_lines.add(line)
        tok = self.peek()
        if tok.kind == "ident" and is_type_name(tok.text):
            # Rectangle { ... } as a value; Qt.binding(...) and Accessible.Button are expressions
            j = self.i
            while self.t[j + 1].text == "." and self.t[j + 2].kind == "ident":
                j += 2
            if self.t[j + 1].text == "{":
                tname, tline = self.dotted()
                el.children.append(self.obj(tname, tline, el, name))
                return
        if tok.text == "[" and self.list_of_objects():
            self.i += 1
            while self.peek().text != "]":
                if self.peek().text == ",":
                    self.i += 1
                    continue
                tname, tline = self.dotted()
                el.children.append(self.obj(tname, tline, el, name))
            self.i += 1
            return
        toks = self.expression()
        for t in toks:
            el.own_lines.add(t.line)
        if name == "id" and toks:
            el.id = toks[0].text
        el.props[name] = (toks, line)

    def list_of_objects(self):
        a = self.peek(1)
        if a and a.kind == "ident" and is_type_name(a.text):
            j = self.i + 1
            while self.t[j + 1].text == "." and self.t[j + 2].kind == "ident":
                j += 2
            return self.t[j + 1].text == "{"
        return False

    def expression(self):
        """A property value: to the end of the line once the brackets are closed, or on to the
        next line when this one ends with an operator or the next one starts with it."""
        out = []
        depth = 0
        pairs = {"{": 1, "(": 1, "[": 1, "}": -1, ")": -1, "]": -1}
        if self.peek().text == "{":
            return self.skip_balanced()
        while True:
            tok = self.peek()
            if tok is None:
                return out
            if depth == 0 and out:
                if tok.text in ("}", ";"):
                    return out
                if tok.line != out[-1].line and not continues(out[-1]) and not continues(tok):
                    return out
            if tok.kind == "punct" and tok.text in pairs:
                depth += pairs[tok.text]
                if depth < 0:
                    return out
            out.append(tok)
            self.i += 1


def continues(tok):
    return tok.kind == "punct" and tok.text in CONTINUE


def parse(src):
    try:
        toks = tokenize(src)
        root = Parser(toks).document()
    except (IndexError, AttributeError, ValueError) as e:
        # a file cut in the middle of an element: "not parsed", not a traceback
        raise ParseError(f"parsing stopped ({type(e).__name__})") from e
    mark_script_names(root, toks)
    return root


def assigns(toks, k, names):
    """The tokens from k are `<one of names> =`, a name given as a list of words joined by dots."""
    for name in names:
        words = []
        for w in name:
            words += [w, "."]
        words[-1] = "="
        if [t.text for t in toks[k:k + len(words)]] == words:
            return name
    return None


def mark_script_names(root, toks):
    """Mark the elements whose `text` or `Accessible.name` is assigned from script, so a name
    given at run time is not reported missing."""
    # the same id may be used in separate scopes (delegates, Components): mark them all, so a
    # name given to one of them is never reported missing
    ids = {}
    for el in root.walk():
        if el.id:
            ids.setdefault(el.id, []).append(el)
    text, name = ("text",), ("Accessible", "name")

    def mark(els, which):
        for el in els:
            if which == text:
                el.script_text = True
            else:
                el.script_name = True

    # anywhere in the file: myButton.text = ..., myField.Accessible.name = ...
    for k, t in enumerate(toks):
        if t.kind == "ident" and t.text in ids and toks[k + 1:k + 2] and toks[k + 1].text == "." and (k == 0 or toks[k - 1].text != "."):
            which = assigns(toks, k + 2, (text, name))
            if which:
                mark(ids[t.text], which)
    for el in root.walk():
        # Binding { target: myButton; property: "Accessible.name"; value: ... }
        if el.type == "Binding":
            target, prop = el.value("target"), literal(el.value("property"))
            if target and len(target) == 1 and target[0].text in ids and prop in ("text", "Accessible.name"):
                mark(ids[target[0].text], text if prop == "text" else name)
        # Binding on text { value: ... } inside the element
        for child in el.children:
            if child.type == "Binding" and child.binding_on in ("text", "Accessible.name"):
                mark([el], text if child.binding_on == "text" else name)
        # text = ... in the element's own handler (onCompleted, onLanguageChanged...); not a local
        # variable `var text = ...`
        for prop, (vt, _) in el.props.items():
            if not prop.split(".")[-1].startswith("on"):
                continue
            for k in range(len(vt)):
                if k and vt[k - 1].text in (".", "var", "let", "const"):
                    continue
                which = assigns(vt, k, (text, name))
                if which:
                    mark([el], which)


# --- values ---------------------------------------------------------------------------------

def has_letters(s):
    return any(ch.isalpha() for ch in s)


def symbol_only(s):
    s = s.strip()
    return bool(s) and not any(ch.isalnum() for ch in s)


def unescape(s):
    """The text of a JavaScript string literal: \\uD83D\\uDD0D is a magnifier emoji, \\n, \\x41;
    surrogate pairs are joined."""
    out, i = [], 0
    while i < len(s):
        ch = s[i]
        if ch != "\\" or i + 1 >= len(s):
            out.append(ch)
            i += 1
            continue
        nxt = s[i + 1]
        try:
            if nxt == "u" and s[i + 2:i + 3] == "{":
                end = s.find("}", i)
                out.append(chr(int(s[i + 3:end], 16)))
                i = end + 1
            elif nxt == "u":
                out.append(chr(int(s[i + 2:i + 6], 16)))
                i += 6
            elif nxt == "x":
                out.append(chr(int(s[i + 2:i + 4], 16)))
                i += 4
            else:
                out.append({"n": "\n", "t": "\t", "r": "\r"}.get(nxt, nxt))
                i += 2
        except ValueError:                       # \u without digits: as is
            out.append(nxt)
            i += 2
    # half of a surrogate pair becomes U+FFFD, not an exception
    return "".join(out).encode("utf-16", "surrogatepass").decode("utf-16", "replace")


def literal(toks):
    """The text of a value that is one string (or qsTr("...")), otherwise None."""
    if toks and len(toks) == 1 and toks[0].kind == "str":
        return unescape(toks[0].text)
    if toks and len(toks) == 4 and toks[0].text in TRANSLATE and toks[1].text == "(" and toks[2].kind == "str" and toks[3].text == ")":
        return unescape(toks[2].text)
    return None


def untranslated(toks):
    """Strings with letters outside qsTr(...) in an expression."""
    # The bracket around a string decides: qsTr(...) is translated; an argument of another call
    # (Qt.formatDate(d, "dddd")) and an index (names["home"]) are not a text for the reader;
    # plain brackets are a text
    bad, stack = [], []
    for k, t in enumerate(toks):
        prev = toks[k - 1] if k else None
        after_name = prev is not None and (prev.kind == "ident" or prev.text in (")", "]"))
        if t.kind == "punct" and t.text == "(":
            stack.append("tr" if prev is not None and prev.text in TRANSLATE else "call" if after_name else "group")
        elif t.kind == "punct" and t.text == "[":
            stack.append("index" if after_name else "array")
        elif t.kind == "punct" and t.text in ")]":
            if stack:
                stack.pop()
        elif t.kind == "str" and has_letters(t.text) and not set(stack) & {"tr", "call", "index"}:
            # chevron === "expand_more" is a comparison, not a text for the reader
            near = {prev.text if prev else "", toks[k + 1].text if k + 1 < len(toks) else ""}
            if not near & {"==", "===", "!=", "!=="}:
                bad.append(t.text)
    return bad


def refs_value(toks, own_id):
    for k, t in enumerate(toks):
        if t.kind != "ident" or t.text not in VALUE_REFS:
            continue
        prev = toks[k - 1] if k else None
        if prev is None or prev.text != ".":
            return True
        if k >= 2 and toks[k - 2].text in (own_id, "control", "root"):
            return True
    return False


def is_true(toks):
    return toks is not None and len(toks) == 1 and toks[0].text == "true"


def is_false(toks):
    return toks is not None and len(toks) == 1 and toks[0].text == "false"


# --- components -----------------------------------------------------------------------------

class Types:
    """Own component -> its Qt base type and the Accessible.* of its root."""

    def __init__(self, sources):
        self.roots = {}
        self.files = {}           # path -> parsed root (or ParseError)
        for path, src in sources.items():
            try:
                root = parse(src)
            except ParseError as e:
                self.files[path] = e
                continue
            self.files[path] = root
            self.roots.setdefault(Path(path).stem, root)
            for el in root.walk():
                if el.component:
                    self.roots.setdefault(el.component, el)

    def chain(self, el):
        """Type names from the element down its components: ValueComboBox, MyComboBox, ComboBox"""
        out, t = [el.type], el.type
        while t in self.roots and self.roots[t].type not in out:
            t = self.roots[t].type
            out.append(t)
        return out

    def resolve(self, el):
        """(Qt base type, properties of the roots of its components)"""
        props, seen, t = {}, set(), el.type
        while t in self.roots and t not in seen and t not in TEXT_CONTROLS | INPUT_CONTROLS:
            seen.add(t)
            root = self.roots[t]
            for k, v in root.props.items():
                props.setdefault(k, v)
            t = root.type
        return t, props


# --- rules ----------------------------------------------------------------------------------

def control_ancestor(el, types):
    p = el.parent
    while p is not None:
        base, _ = types.resolve(p)
        if base in TEXT_CONTROLS | INPUT_CONTROLS:
            return p
        p = p.parent
    return None


def check_elem(el, types):
    """[(rule, level, message)] for one element; level is "error" or "warning"."""
    base, inherited = types.resolve(el)
    props = dict(inherited)
    props.update(el.props)

    def val(name):
        p = props.get(name)
        return p[0] if p else None

    out = []
    if is_true(val("Accessible.ignored")) or is_false(val("visible")):
        return out
    name = val("Accessible.name")
    for key in ("Accessible.name", "Accessible.description"):
        if key in el.props:
            for s in untranslated(el.props[key][0]):
                out.append(("no-qstr", "error", f'{key}: "{s}" is not in qsTr(), so the screen reader reads it in English in every language'))
    named = el.script_name or (name is not None and literal(name) != "")
    if name is not None and literal(name) is not None and symbol_only(literal(name)):
        out.append(("symbol-text", "error", f'Accessible.name is the symbol "{literal(name)}": the screen reader reads out the symbol; use a word in qsTr()'))
    # The root of a component (a file or component X: ...) gets its name from each instance
    definition = el.parent is None or el.component is not None
    if base in TEXT_CONTROLS:
        text = val("text")
        action = val("action")
        lit = literal(text) if text is not None else None
        # a text set from script names the control; a symbol in its literal text still counts
        unnamed = not named and not el.script_text
        if definition:
            pass
        elif unnamed and text is None and action is None:
            out.append(("no-name", "error", f"{base} without text or Accessible.name: the screen reader says just \"button\", and voice control cannot press it"))
        elif name is None and lit is not None and symbol_only(lit):
            out.append(("symbol-text", "error", f'the name of the {base} is the symbol "{lit}": add Accessible.name: qsTr("...")'))
        elif unnamed and lit == "":
            out.append(("no-name", "error", f"{base} with an empty text and no Accessible.name"))
        elif unnamed and text is not None and empty_branch(text):
            # text: modern ? "" : "<" - in one of the states the button has no name
            out.append(("no-name", "error", f'{base}: text is "" in one branch of the condition and there is no Accessible.name, so in that state the control has no name'))
        if base in NEED_ACTION and "Accessible.onPressAction" not in props:
            if el.type != base and not definition:
                # an instance of an own component (a settings switch) without the action: the
                # component is old code, and one fix there covers every instance; a change of
                # the component itself is checked on its root
                out.append(("no-action", "warning", f"{el.type} ({base}) without Accessible.onPressAction: in Qt 5.15 a double tap of the screen reader does not emit clicked(); a fix in {el.type}.qml covers all its instances"))
            else:
                out.append(("no-action", "error", f"{base} without Accessible.onPressAction: in Qt 5.15 a double tap of the screen reader does not emit clicked()"))
    elif base in INPUT_CONTROLS:
        # a field in contentItem is a part of its control (ComboBox, SpinBox), named by the control
        if el.slot == "contentItem" or (base in ("TextInput", "TextEdit") and is_true(val("readOnly"))):
            return out
        # a password field must not put its value into the name: the reader would say it aloud
        echo = val("echoMode")
        password = echo is not None and any("Password" in t.text for t in echo)
        value = text_of_value(base)
        fix = 'add Accessible.name: qsTr("...")' if password or not value else f'add Accessible.name: qsTr("...") + ": " + {value}'
        if definition:
            pass
        elif not named and "placeholderText" in props:
            out.append(("placeholder", "warning", f"{base} without Accessible.name: Qt reads placeholderText as a description, not as the name; {fix}"))
        elif not named:
            out.append(("no-name", "error", f"{base} without Accessible.name: the screen reader reads only the value, not which setting it is; {fix}"))
        elif not password and not el.script_name and not refs_value(name, el.id):
            out.append(("name-no-value", "warning", f"{base}: Accessible.name does not include the value, and Qt 5.15 on Android then does not read the value"))
    else:
        role = val("Accessible.role")
        if role is not None and [t.text for t in role][-1:] == ["Button"] and "Accessible.onPressAction" not in props:
            out.append(("no-action", "error", "Accessible.role: Accessible.Button without Accessible.onPressAction: a double tap of the screen reader does nothing"))
        if el.type in ("Text", "Label") and not named and "Accessible.role" not in props:
            lit = literal(val("text"))
            if lit is not None and symbol_only(lit) and control_ancestor(el, types) is None:
                out.append(("glyph-text", "warning", f'{el.type} "{lit}" without Accessible.ignored: true: the screen reader stops on the symbol'))
        if el.type in ("MouseArea", "TapHandler") and "Accessible.role" not in props:
            live = [h for h in sorted(el.props) if not empty_block(el.props[h][0])]
            handlers = [h for h in live if h in TAP_HANDLERS]
            if not handlers and not any(h in el.props for h in DRAG_SIGNS):
                handlers = [h for h in live if h in PRESS_HANDLERS]
            if handlers and not is_false(val("enabled")) and not accessible_press(el, types):
                out.append(("tap-only", "warning", f"{el.type} with {handlers[0]} on an item without Accessible.role and Accessible.onPressAction: the screen reader cannot press it"))
    return out


def text_of_value(base):
    """The property holding the value of an editable control, for the hint in a message."""
    if base in ("TextField", "TextArea", "TextInput", "TextEdit"):
        return "text"
    if base == "ComboBox":
        return "displayText"
    if base in ("SpinBox", "Slider", "Dial"):
        return "value"
    return None


def empty_branch(toks):
    """A branch of a condition is an empty string: a ? "" : b, a ? b : "". With a concatenation
    (qsTr("A") + (b ? "" : "!")) the whole text is never empty."""
    if any(t.kind == "punct" and t.text == "+" for t in toks):
        return False
    for k, t in enumerate(toks):
        if t.kind == "str" and t.text == "" and k > 0 and toks[k - 1].kind == "punct" and toks[k - 1].text in ("?", ":"):
            return True
    return False


def empty_block(toks):
    return [t.text for t in toks] in (["{", "}"], [])


def accessible_press(area, types):
    """The press is reachable for the screen reader: on the area itself, or on a control or an
    item with onPressAction up to 3 levels above (a MouseArea may sit in an Item inside the
    Rectangle that has the action)."""
    if "Accessible.onPressAction" in area.props:
        return True
    p, level = area.parent, 0
    while p is not None and level < 3:
        base, inherited = types.resolve(p)
        if base in TEXT_CONTROLS | INPUT_CONTROLS:
            return True
        props = dict(inherited)
        props.update(p.props)
        if "Accessible.onPressAction" in props or is_true(props.get("Accessible.ignored", (None,))[0]):
            return True
        if "Accessible.role" in props:           # a role without an action is no-action there
            return False
        p, level = p.parent, level + 1
    return False


# --- git ------------------------------------------------------------------------------------

def git(repo, *args):
    return subprocess.run(["git", "-C", str(repo), *args], capture_output=True, text=True,
                          encoding="utf-8", check=True).stdout


class FileDiff:
    """Changes of one file: the changed lines of the new version, the hunks, the old path"""

    def __init__(self, path):
        self.path = path
        self.old_path = path
        self.lines = set()        # None: the whole file is new (untracked)
        self.hunks = []           # (old start, count, new start, count)

    def base_line(self, line, approx=False):
        """A line of the new version outside the hunks -> the line of the base; in a hunk None,
        or with approx the line at the same place of the old part when the hunk replaces lines
        one for one (text: "Open" -> "Open file"); where the hunk adds or removes lines, the
        places do not match - None"""
        delta = 0
        for os_, oc, ns, nc in self.hunks:
            if nc and ns <= line < ns + nc:
                return os_ + (line - ns) if approx and oc == nc else None
            if (nc and ns + nc <= line) or (not nc and ns < line):
                delta += oc - nc
        return line + delta


def parse_range(part):
    start, _, count = part[1:].partition(",")
    return int(start), int(count) if count else 1


def changed_lines(repo, base, head):
    """path -> FileDiff. A renamed file keeps its old path (-M); the paths are unquoted and have
    the a/ b/ prefixes whatever the git settings are."""
    # the diff algorithm is fixed: elements are paired by how it aligns the lines, and the
    # user's diff.algorithm and diff.indentHeuristic would silently change that
    args = ["-c", "core.quotePath=false", "diff", "-U0", "--no-color", "-M", "--diff-algorithm=myers",
            "--indent-heuristic", "--src-prefix=a/", "--dst-prefix=b/", base] + ([head] if head else []) + ["--", "*.qml"]
    out, cur, old = {}, None, None
    for line in git(repo, *args).splitlines():
        if line.startswith("diff --git "):
            cur, old = None, None
        elif line.startswith("rename from "):
            old = line[len("rename from "):]
        elif line.startswith("rename to "):
            # a rename without changed lines has no ---/+++: the file is checked only for the
            # instances of changed components, but its base is under the old path
            cur = out.setdefault(line[len("rename to "):], FileDiff(line[len("rename to "):]))
            cur.old_path = old
        elif line.startswith("--- "):
            path = line[4:].rstrip("\t")
            old = None if path == "/dev/null" else path[2:]
        elif line.startswith("+++ "):
            path = line[4:].rstrip("\t")
            cur = None if path == "/dev/null" else out.setdefault(path[2:], FileDiff(path[2:]))
            if cur is not None and old:
                cur.old_path = old
        elif line.startswith("@@") and cur is not None:
            parts = line.split(" ")
            os_, oc = parse_range([p for p in parts if p.startswith("-")][0])
            ns, nc = parse_range([p for p in parts if p.startswith("+")][0])
            cur.hunks.append((os_, oc, ns, nc))
            if nc:
                cur.lines.update(range(ns, ns + nc))
            else:
                # lines only removed (an Accessible.name dropped): check the lines around
                cur.lines.update((ns, ns + 1))
    if head is None:
        # new files not yet added to git: git diff does not see them
        for path in git(repo, "-c", "core.quotePath=false", "ls-files", "--others",
                        "--exclude-standard", "--", "*.qml").splitlines():
            out.setdefault(path, FileDiff(path)).lines = None
    return out


def read_tree(repo, head, paths):
    if head:
        return {p: git(repo, "show", f"{head}:{p}") for p in paths}
    # a file missing on disk (sparse checkout) is as in HEAD
    return {p: (Path(repo) / p).read_text(encoding="utf-8") if (Path(repo) / p).exists()
            else git(repo, "show", f"HEAD:{p}") for p in paths}


def list_qml(repo, head):
    q = ["-c", "core.quotePath=false"]
    if head:
        names = git(repo, *q, "ls-tree", "-r", "--name-only", head).splitlines()
    else:
        # untracked too: a new component may not be in git yet
        names = git(repo, *q, "ls-files", "--cached", "--others", "--exclude-standard", "--", "*.qml").splitlines()
    return sorted({n for n in names if n.endswith(".qml")})


def load_exceptions(path):
    if not path.exists():
        return {}
    data = json.loads(path.read_text(encoding="utf-8"))
    data.pop("_", None)
    return data


# --- old findings ---------------------------------------------------------------------------

def elem_key(el, types):
    """How an element is found in the other version of the file when not by place (all its lines
    changed): the Qt base type (TextField and a component based on it are both TextField) and
    the id, otherwise its text, otherwise only the type."""
    base = types.resolve(el)[0]
    if el.id:
        return base + "#" + el.id
    # Accessible.name last: a fix adds it, and the key must not change with the fix
    for prop in ("text", "placeholderText", "icon.source", "onClicked", "onTriggered", "Accessible.name"):
        if prop in el.props:
            return base + " " + prop + "=" + el.text_of(prop)
    return base


def hidden(el):
    """The element or one of its parents has visible: false - the screen reader does not see it"""
    while el is not None:
        if is_false(el.props.get("visible", (None,))[0]):
            return True
        el = el.parent
    return False


def in_scope(el, lines, extra):
    """The element is changed (one of its lines is in the diff), it is an instance of a changed
    component (extra), or it is a MouseArea/TapHandler below a changed element: tap-only depends
    on the same 3 levels of parents that accessible_press looks at."""
    if lines is None or el in extra or el.own_lines & lines:
        return True
    if el.type not in ("MouseArea", "TapHandler"):
        return False
    p, level = el.parent, 0
    while p is not None and level < 3:
        if p.own_lines & lines:
            return True
        p, level = p.parent, level + 1
    return False


def findings(root, types, lines=None, extra=()):
    """[(element, rule, level, message)]; lines - only the changed elements (None - all)"""
    out = []
    for el in root.walk():
        if not in_scope(el, lines, extra) or hidden(el):
            continue
        for rule, level, msg in check_elem(el, types):
            out.append((el, rule, level, msg))
    return out


def match_elems(head_root, base_root, diff, types, base_types):
    """Element of the new version -> the same element of the base, one to one and only with the
    same Qt base type (a new SpinBox in place of a fixed ComboBox is another element). In turn:
    1. unchanged lines: the base element holding the opening line of the element (`Button {`)
       mapped through the diff, then the one with most of its other lines (the text and the
       handler may change, the rest does not); the closing brace does not count - the diff
       readily aligns it with the next element;
    2. the key elem_key when it is more than the bare type (id, text, icon): a hunk may hold
       another element at the same place, and the key is more exact than the place;
    3. the place in a hunk that replaces lines one for one (a one-line element rewritten);
    4. the key elem_key, the bare type too, in file order (all lines rewritten: TextField ->
       a component based on it).
    A pair takes the base element: no other element gets it, so a new button next to a fixed
    old one does not hide behind the old finding."""
    head_els, base_els = list(head_root.walk()), list(base_root.walk())
    head_type = [types.resolve(el)[0] for el in head_els]
    base_type = [base_types.resolve(el)[0] for el in base_els]
    at = {}
    for b, el in enumerate(base_els):
        for ln in el.own_lines:
            at.setdefault(ln, []).append(b)
    match, used = {}, set()
    changed = diff.lines or set()

    def by_key(strong):
        keys = {}
        for b, el in enumerate(base_els):
            if b not in used:
                keys.setdefault(elem_key(el, base_types), []).append(b)
        for h, el in enumerate(head_els):
            key = elem_key(el, types)
            if h not in match and (not strong or key != head_type[h]):
                free = keys.get(key)
                if free:
                    match[h] = free.pop(0)
                    used.add(match[h])

    for approx in (False, True):
        if approx:
            by_key(True)
        score = {}
        for h, el in enumerate(head_els):
            if h in match:
                continue
            own = el.own_lines - ({el.end} if el.end != el.line else set())
            for ln in (own & changed if approx else own - changed):
                for b in at.get(diff.base_line(ln, approx), ()):
                    if b not in used and base_type[b] == head_type[h]:
                        opening, other = score.get((h, b), (0, 0))
                        if ln == el.line and diff.base_line(ln, approx) == base_els[b].line:
                            opening = 1
                        else:
                            other += 1
                        score[(h, b)] = (opening, other)
        # the opening line at the same place first, then more common lines, then file order
        for (h, b), _ in sorted(score.items(), key=lambda x: (-x[1][0], -x[1][1], x[0])):
            if h not in match and b not in used:
                match[h] = b
                used.add(b)
    by_key(False)
    return {head_els[h]: base_els[b] for h, b in match.items()}


def new_findings(found, head_root, base_root, diff, types, base_types):
    """The findings of `found` that the same element (match_elems) did not have in the base.
    Returns (new findings, how many were there before the change).
    Cannot tell apart: an element replaced by another of the same type at the same place in one
    line (MenuItem "Open GPX" -> MenuItem "Import") - it looks like a changed text, so the
    finding counts as old; a copy of an old unnamed element next to the fixed original (the diff
    gives the copy the lines of the original, and the key is the same)."""
    if base_root is None or diff.lines is None:
        return list(found), 0
    match = match_elems(head_root, base_root, diff, types, base_types)
    before = {}
    for el, rule, _, _ in findings(base_root, base_types):
        before[(el, rule)] = before.get((el, rule), 0) + 1
    new = []
    for f in found:
        key = (match.get(f[0]), f[1])
        if before.get(key):
            before[key] -= 1
        else:
            new.append(f)
    return new, len(found) - len(new)


def component_instances(types, changed):
    """The root of a component (a file or component X: ...) is changed - its instances in all
    files, which may have lost their name or action with it. path -> {elements}"""
    names = set()
    for path, d in changed.items():
        root = types.files.get(path)
        if not isinstance(root, Elem):
            continue
        for el in root.walk():
            if (el.parent is None or el.component) and (d.lines is None or el.own_lines & d.lines):
                names.add(el.component or Path(path).stem)
    out = {}
    if not names:
        return out
    for path, root in types.files.items():
        if isinstance(root, Elem):
            for el in root.walk():
                if el.parent is not None and not el.component and names & set(types.chain(el)):
                    out.setdefault(path, set()).add(el)
    return out


# --- output ---------------------------------------------------------------------------------

def annotation(level, path, line, title, msg):
    """A GitHub workflow command that shows the finding next to the line in the pull request."""
    def escape(s, prop=False):
        s = s.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")
        return s.replace(":", "%3A").replace(",", "%2C") if prop else s
    props = f"file={escape(path, True)}" + (f",line={line}" if line else "") + f",title={escape(title, True)}"
    return f"::{level} {props}::{escape(msg)}"


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    if hasattr(sys.stdout, "reconfigure"):
        # a symbol in a message must not stop the check on a console without UTF-8
        sys.stdout.reconfigure(errors="replace")
    if argv and argv[0] in ("-h", "--help"):
        print(__doc__)
        return 0
    repo, head, exceptions_path = Path("."), None, EXCEPTIONS
    flags, files, pos = set(), [], []
    k = 0
    while k < len(argv):
        a = argv[k]
        if a in ("--head", "--exceptions", "--repo"):
            if k + 1 >= len(argv):
                print(f"{a} needs a value")
                return 2
            if a == "--head":
                head = argv[k + 1]
            elif a == "--repo":
                repo = Path(argv[k + 1])
            else:
                exceptions_path = Path(argv[k + 1])
            k += 2
            continue
        if a in ("--all", "--no-baseline", "--github"):
            flags.add(a)
        elif a == "--files":
            files = [f.replace("\\", "/") for f in argv[k + 1:]]
            break
        elif a.startswith("--"):
            print(f"unknown option {a}; see --help")
            return 2
        else:
            pos.append(a)
        k += 1
    base = pos[0] if pos else "origin/master"
    if "--all" not in flags:
        # like the diff of a pull request on GitHub: from the merge base, so a branch behind
        # master does not get the new code of master as its own change
        base = git(repo, "merge-base", base, head or "HEAD").strip()

    all_qml = list_qml(repo, head)
    sources = read_tree(repo, head, all_qml)
    types = Types(sources)
    if "--all" in flags:
        diffs = {f: FileDiff(f) for f in (files or all_qml)}
        for d in diffs.values():
            d.lines = None
        extra = {}
    else:
        diffs = changed_lines(repo, base, head)
        extra = component_instances(types, diffs)
        for path in extra:
            diffs.setdefault(path, FileDiff(path))
        if files:
            diffs = {f: v for f, v in diffs.items() if f in files}
    # QML tests (tst/) are not a part of the app
    diffs = {f: v for f, v in diffs.items() if f in sources and not (f.startswith("tst/") or "/tst/" in f)}
    # Base: a finding the same element already had before the change is old, not new
    use_base = "--all" not in flags and "--no-baseline" not in flags
    base_types = None
    if use_base and diffs:
        base_types = Types(read_tree(repo, base, list_qml(repo, base)))
    exceptions = load_exceptions(exceptions_path)
    github = "--github" in flags

    errors = warnings = skipped = old = 0

    def report(level, path, line, rule, label, msg):
        print(f"{path}:{line}: {level}: {rule}: {label}{msg}")
        if github:
            print(annotation(level, path, line, f"Accessibility: {rule}", label + msg))

    for path in sorted(diffs):
        d = diffs[path]
        root = types.files[path]
        if not isinstance(root, Elem):
            warnings += 1
            report("warning", path, 0, "not-parsed", "", f"the file was not checked: {root}")
            continue
        found = findings(root, types, d.lines, extra.get(path, ()))
        if base_types is not None:
            base_root = base_types.files.get(d.old_path)
            if isinstance(base_root, ParseError):
                # without the base every finding would look new: check nothing rather than all
                warnings += 1
                report("warning", path, 0, "not-parsed", "", f"the base version {d.old_path} was not parsed, so this file was not checked: {base_root}")
                continue
            found, n_old = new_findings(found, root, base_root, d, types, base_types)
            old += n_old
        lines = sources[path].splitlines()
        for el, rule, level, msg in found:
            if any(x["rule"] == rule and x["anchor"] in el.lines_text(lines) for x in exceptions.get(path, [])):
                skipped += 1
                continue
            if level == "error":
                errors += 1
            else:
                warnings += 1
            label = el.full_type + (f" #{el.id}" if el.id else "") + ": "
            report(level, path, el.line, rule, label, msg)
    tail = f", already there before the change: {old}" if use_base else ""
    print(f"errors: {errors}, warnings: {warnings}, skipped by exceptions: {skipped}{tail}, files: {len(diffs)}")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
