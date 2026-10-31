import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).parents[1] / "tools"))
import check_accessibility as c


def findings(qml, components=None):
    """[(rule, level, line)] of every element of a QML snippet"""
    sources = {"Page.qml": qml}
    sources.update(components or {})
    types = c.Types(sources)
    return sorted((r, level, el.line) for el, r, level, _ in c.findings(types.files["Page.qml"], types))


def rules(qml, components=None):
    """[(rule, line)] of every element of a QML snippet"""
    return [(r, line) for r, _, line in findings(qml, components)]


def wrap(body):
    """The snippet inside a root Item; its first line is line 3"""
    return "import QtQuick 2.15\nItem {\n" + body + "\n}\n"


class ValidPatterns(unittest.TestCase):
    """Accessible QML: no finding"""

    def test_named_controls(self):
        for qml in (
            'Button { text: qsTr("OK") }',
            'Button { text: "×"; Accessible.name: qsTr("Close") }',
            'ToolButton { icon.source: "back.png"; Accessible.name: qsTr("Back") }',
            'ToolButton { action: backAction }',
            'Button { text: qsTr("A") + (b ? "" : "!") }',
            'ToolButton { text: modern ? "" : "x"; Accessible.name: qsTr("Menu") }',
            'Button { text: qsTr("A"); contentItem: Text { text: "→" } }',
        ):
            self.assertEqual(rules(wrap(qml)), [], qml)

    def test_dynamic_names(self):
        # names from an expression or a model are not checked for qsTr
        for qml in (
            'Button { text: modelData }',
            'Button { text: model.label }',
            'ToolButton { icon.source: "a.png"; Accessible.name: root.title }',
            'Rectangle { Accessible.name: title + ", " + qsTr("has a workout") }',
            'Rectangle { Accessible.name: qsTr("Speed %1").arg(speed) }',
            'Rectangle { Accessible.name: Qt.formatDate(d, "dddd d MMMM") + names["home"] }',
            'Rectangle { Accessible.description: chevron === "expand_more" ? qsTr("Open") : "" }',
            'Rectangle { Accessible.name: qsTranslate("homeform", "Speed") }',
        ):
            self.assertEqual(rules(wrap(qml)), [], qml)

    def test_names_set_from_script(self):
        for qml in (
            'ToolButton { id: b; icon.source: "a.png" }\nComponent.onCompleted: b.Accessible.name = qsTr("Back")',
            'ToolButton { id: b; icon.source: "a.png" }\nfunction setUp() { b.text = qsTr("Back") }',
            'ToolButton { icon.source: "a.png"; Component.onCompleted: text = qsTr("Back") }',
            'ToolButton { id: b; icon.source: "a.png" }\nBinding { target: b; property: "Accessible.name"; value: qsTr("Back") }',
            'TextField { id: f }\nComponent.onCompleted: f.Accessible.name = qsTr("Weight")',
            'ToolButton { icon.source: "a.png"; Binding on text { value: qsTr("Back") } }',
            'ToolButton { icon.source: "a.png"; Binding on Accessible.name { value: qsTr("Back") } }',
            # the same id in two delegates: the name given to one of them is not reported missing
            'Repeater { delegate: ToolButton { id: b; icon.source: "c"; Component.onCompleted: b.text = qsTr("C") } }\n'
            'Repeater { delegate: ToolButton { id: b; icon.source: "a"; Accessible.name: qsTr("A") } }',
        ):
            self.assertEqual(rules(wrap(qml)), [], qml)

    def test_fields_with_value(self):
        for qml in (
            'TextField { id: f; Accessible.name: qsTr("Weight") + " " + f.text }',
            'ComboBox { Accessible.name: qsTr("Mode") + ": " + displayText }',
            'ComboBox { Accessible.name: qsTr("A") + displayText; contentItem: TextField { } }',
            'SpinBox { Accessible.name: qsTr("Age") + ": " + value }',
            'TextInput { readOnly: true; text: "1" }',
            # a password field is named without its value, so the reader does not say it aloud
            'TextField { echoMode: TextInput.Password; Accessible.name: qsTr("Password") }',
        ):
            self.assertEqual(rules(wrap(qml)), [], qml)

    def test_actions(self):
        for qml in (
            'MenuItem { text: qsTr("Import"); Accessible.onPressAction: triggered() }',
            'Rectangle {\n Accessible.role: Accessible.Button\n Accessible.name: qsTr("D")\n Accessible.onPressAction: f()\n'
            ' MouseArea { onClicked: f() }\n}',
            # the action is on the Rectangle above an Item
            'Rectangle {\n Accessible.role: Accessible.Button\n Accessible.name: qsTr("D")\n'
            ' Accessible.onPressAction: f()\n Item {\n  MouseArea { onClicked: f() }\n }\n}',
            'Button {\n text: qsTr("A")\n MouseArea { onClicked: f() }\n}',
            'Rectangle {\n MouseArea { onClicked: {} }\n}',
            'Rectangle {\n MouseArea { onPressed: s(); onPositionChanged: m() }\n}',
            'Rectangle {\n MouseArea { onPressAndHold: menu() }\n}',
            'Rectangle {\n MouseArea { enabled: false; onClicked: f() }\n}',
        ):
            self.assertEqual(rules(wrap(qml)), [], qml)

    def test_skipped(self):
        for qml in (
            'Button { visible: false }',
            'Item {\n visible: false\n Button { icon.source: "x.png" }\n}',
            'Button { Accessible.ignored: true }',
            'Text { text: "🔍"; Accessible.ignored: true }',
        ):
            self.assertEqual(rules(wrap(qml)), [], qml)

    def test_components(self):
        comps = {"NamedField.qml": 'TextField {\n Accessible.name: label + ": " + text\n property string label\n}\n'}
        self.assertEqual(rules(wrap('NamedField { label: qsTr("Age") }'), comps), [])
        # the root of a component gets its name from its instances
        self.assertEqual(rules("import QtQuick 2.15\nT.Button {\n}\n"), [])
        self.assertEqual(rules(wrap("component Big: Button { font.bold: true }")), [])


class Violations(unittest.TestCase):
    """QML the screen reader cannot name or press"""

    def test_button_names(self):
        self.assertEqual(findings(wrap('Button { icon.source: "a.png" }')), [("no-name", "error", 3)])
        self.assertEqual(rules(wrap('Button { text: "" }')), [("no-name", 3)])
        self.assertEqual(rules(wrap('ToolButton { contentItem: Text { text: "🔍" } }')), [("no-name", 3)])
        self.assertEqual(rules(wrap('ToolButton { text: modern ? "" : qsTr("Menu") }')), [("no-name", 3)])
        self.assertEqual(rules(wrap('ToolButton { text: a ? qsTr("A") : "" }')), [("no-name", 3)])
        # a name given to another button does not count
        self.assertEqual(rules(wrap('ToolButton { id: a; icon.source: "a.png" }\nToolButton { id: b; Component.onCompleted: a.width = 3 }')),
                         [("no-name", 3), ("no-name", 4)])
        # a local variable named text is not the name of the control
        for handler in ('{ var text = clip(); paste(text) }', '{ let text = clip() }', '{ const text = clip() }'):
            self.assertEqual(rules(wrap('ToolButton { icon.source: "x.png"; onClicked: ' + handler + ' }')),
                             [("no-name", 3)], handler)

    def test_symbol_names(self):
        self.assertEqual(findings(wrap('Button { text: "<" }')), [("symbol-text", "error", 3)])
        # a glyph as an escape sequence
        self.assertEqual(rules(wrap('ToolButton { text: "\\uD83D\\uDD0D" }')), [("symbol-text", 3)])
        self.assertEqual(rules(wrap('ToolButton { text: qsTr("+") }')), [("symbol-text", 3)])
        # a text changed from script names the button, but its symbol is still read out
        self.assertEqual(rules(wrap('ToolButton { text: "+"; onClicked: text = "-" }')), [("symbol-text", 3)])
        self.assertEqual(rules(wrap('Item {\n MouseArea { onClicked: f(); Accessible.role: Accessible.Button; Accessible.name: "-" }\n}')),
                         [("no-action", 4), ("symbol-text", 4)])

    def test_fields(self):
        self.assertEqual(findings(wrap('TextField { }')), [("no-name", "error", 3)])
        self.assertEqual(findings(wrap('SpinBox { contentItem: TextInput { text: "1" } }')), [("no-name", "error", 3)])
        self.assertEqual(findings(wrap('ComboBox { model: m }')), [("no-name", "error", 3)])
        # uncertain: warnings
        self.assertEqual(findings(wrap('TextField { placeholderText: qsTr("Name") }')), [("placeholder", "warning", 3)])
        self.assertEqual(findings(wrap('TextField { Accessible.name: qsTr("Weight") }')), [("name-no-value", "warning", 3)])
        self.assertEqual(findings(wrap('TextField { Accessible.name: label.text }')), [("name-no-value", "warning", 3)])
        # the hint names the value of the control, and never for a password field
        self.assertIn('+ ": " + displayText', c.check_elem(c.parse(wrap("ComboBox { }")).children[0], c.Types({}))[0][2])
        pw = c.parse(wrap("TextField { echoMode: TextInput.Password }")).children[0]
        self.assertNotIn("+ text", c.check_elem(pw, c.Types({}))[0][2])
        # `text = ...` of a field is its value, not its name
        self.assertEqual(rules(wrap('TextField { Component.onCompleted: text = "5" }')), [("no-name", 3)])

    def test_no_qstr(self):
        self.assertEqual(findings(wrap('Rectangle { Accessible.description: open ? "Expanded" : "Collapsed" }')),
                         [("no-qstr", "error", 3), ("no-qstr", "error", 3)])
        self.assertEqual(rules(wrap('Rectangle { Accessible.name: (a ? "Open" : x) }')), [("no-qstr", 3)])
        self.assertEqual(rules(wrap('Button { text: qsTr("A"); Accessible.name: "Apply" }')), [("no-qstr", 3)])

    def test_actions(self):
        self.assertEqual(findings(wrap('MenuItem { text: qsTr("Import") }')), [("no-action", "error", 3)])
        self.assertEqual(rules(wrap('SwitchDelegate { text: qsTr("On") }')), [("no-action", 3)])
        self.assertEqual(rules(wrap('Rectangle { Accessible.role: Accessible.Button; Accessible.name: qsTr("Day") }')),
                         [("no-action", 3)])

    def test_tap_only(self):
        self.assertEqual(findings(wrap('Rectangle {\n MouseArea { onClicked: f() }\n}')), [("tap-only", "warning", 4)])
        self.assertEqual(rules(wrap('Rectangle {\n MouseArea { onPressed: f() }\n}')), [("tap-only", 4)])
        self.assertEqual(rules(wrap('Rectangle {\n TapHandler { onTapped: f() }\n}')), [("tap-only", 4)])

    def test_glyph(self):
        self.assertEqual(findings(wrap('Text { text: "🔍" }')), [("glyph-text", "warning", 3)])

    def test_components(self):
        comps = {"UiButton.qml": "import QtQuick.Templates 2.15 as T\nT.Button {\n}\n",
                 "Sw.qml": "SwitchDelegate {\n}\n"}
        self.assertEqual(rules(wrap("UiButton { }"), comps), [("no-name", 3)])
        # an instance of an old component without the action: a warning, the fix is in the component
        self.assertEqual(findings(wrap('Sw { text: qsTr("On") }'), comps), [("no-action", "warning", 3)])
        # the component itself without the action: an error on its root
        self.assertEqual(findings("SwitchDelegate {\n}\n"), [("no-action", "error", 1)])


class Parse(unittest.TestCase):
    def test_string_that_looks_like_operator(self):
        # "<" is a string, not an operator: onClicked on the next line is a property of its own
        btn = c.parse(wrap('Button {\n text: "<"\n onClicked: f()\n}')).children[0]
        self.assertEqual(set(btn.props), {"text", "onClicked"})

    def test_multiline_ternary(self):
        root = c.parse(wrap('Text {\n text: a\n   ? "x"\n   : "y"\n color: "red"\n}'))
        self.assertEqual(set(root.children[0].props), {"text", "color"})

    def test_regex_comment_template(self):
        qml = wrap('TextField {\n validator: RegExpValidator { regExp: /^[0-9"\']{1,3}$/ }\n'
                   ' // "not closed in a comment\n property string t: `a ${b} "c`\n'
                   ' onTextChanged: { var x = a / 2; if (x) { y() } }\n Accessible.name: qsTr("A") + text\n}')
        f = c.parse(qml).children[0]
        self.assertIn("Accessible.name", f.props)
        self.assertEqual(f.children[0].type, "RegExpValidator")

    def test_inline_component_group_behavior_list(self):
        qml = wrap('component Big: Button { font.bold: true }\n font { pixelSize: 3 }\n'
                   ' Behavior on x { NumberAnimation {} }\n states: [ State { name: "a" }, State {} ]\n'
                   ' function f(a, b) { return {k: 1} }\n signal s(int a)\n property list<Item> items\n'
                   ' Big { text: qsTr("Go") }')
        root = c.parse(qml)
        self.assertEqual([e.type for e in root.children], ["Button", "Behavior", "State", "State", "Big"])
        self.assertEqual(root.children[0].component, "Big")

    def test_broken_file(self):
        # a file cut anywhere is a ParseError, not an IndexError with a traceback
        for bad in ("Button {", "Item {\n x: Foo", "Item {\n function f()", "Item {\n x: [ A"):
            with self.assertRaises(c.ParseError, msg=bad):
                c.parse(wrap(bad))
        self.assertEqual(c.unescape("\\uD83D"), "�")
        self.assertEqual(c.unescape("\\uZZ"), "uZZ")
        self.assertEqual(c.unescape("\\uD83D\\uDD0D \\u25C0\\x41\\n"), "🔍 ◀A\n")

    def test_line_numbers_after_continued_string(self):
        root = c.parse(wrap('Text { text: "a\\\n b" }\nButton { text: qsTr("B") }'))
        self.assertEqual(root.children[1].line, 5)

    def test_regex_after_arrow_and_words(self):
        root = c.parse(wrap('Item {\n property var f: list.filter(s => /"/.test(s))\n'
                            ' function g(x) { if (x) a(); else /\'/.test(x); return typeof /"/ }\n}\nButton { text: "+" }'))
        self.assertEqual(root.children[1].type, "Button")

    def test_division_not_regex(self):
        # "/" after a property name, and "/" without a pair on its line, is a division
        root = c.parse(wrap('Item {\n width: obj.in / 2\n height: (a) /\n  2\n}\nButton { text: "/" }'))
        self.assertEqual(root.children[1].line, 8)
        self.assertEqual(set(root.children[0].props), {"width", "height"})


class Diff(unittest.TestCase):
    def run_diff(self, diff, head):
        orig = c.git
        c.git = lambda repo, *a: diff if "diff" in a else ""
        try:
            return c.changed_lines(".", "base", head)
        finally:
            c.git = orig

    def test_hunks(self):
        got = self.run_diff("diff --git a/src/a.qml b/src/a.qml\n--- a/src/a.qml\n+++ b/src/a.qml\n"
                            "@@ -10,0 +11,2 @@\n+x\n+y\n@@ -20 +22 @@\n-a\n+b\n@@ -30,2 +33,0 @@\n-c\n-d\n"
                            "diff --git a/src/b.qml b/src/b.qml\n--- a/src/b.qml\n+++ /dev/null\n@@ -1 +0,0 @@\n-z\n", None)
        self.assertEqual(set(got), {"src/a.qml"})
        d = got["src/a.qml"]
        self.assertEqual(d.lines, {11, 12, 22, 33, 34})
        # lines outside the hunks map to the base: +2 after an insert, 0 after a change, -2 after a removal
        self.assertEqual([d.base_line(x) for x in (5, 11, 15, 22, 25, 40)], [5, None, 13, None, 23, 40])

    def test_rename_and_space(self):
        got = self.run_diff("diff --git a/src/Old.qml b/src/New page.qml\nsimilarity index 90%\nrename from src/Old.qml\n"
                            "rename to src/New page.qml\n--- a/src/Old.qml\n+++ b/src/New page.qml\t\n@@ -3 +3 @@\n-a\n+b\n", "head")
        self.assertEqual(set(got), {"src/New page.qml"})
        self.assertEqual(got["src/New page.qml"].old_path, "src/Old.qml")

    def test_annotation(self):
        self.assertEqual(c.annotation("error", "src/a,b.qml", 3, "Accessibility: no-name", "50% done\nnext"),
                         "::error file=src/a%2Cb.qml,line=3,title=Accessibility%3A no-name::50%25 done%0Anext")


class Baseline(unittest.TestCase):
    """main() on a real git repository in a temporary folder: only new findings"""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.repo = Path(self.tmp.name)
        for args in (["init", "-q"], ["config", "user.email", "t@t"], ["config", "user.name", "t"],
                     ["config", "core.autocrlf", "false"]):
            c.git(self.repo, *args)
        (self.repo / "src").mkdir()

    def tearDown(self):
        self.tmp.cleanup()

    def write(self, name, qml):
        (self.repo / "src" / name).write_text(qml, encoding="utf-8")

    def commit(self, qml=None):
        if qml is not None:
            self.write("Page.qml", qml)
            self.write("SettingsTextField.qml", "TextField {\n}\n")
        c.git(self.repo, "add", "-A")
        c.git(self.repo, "commit", "-q", "-m", "x")
        return c.git(self.repo, "rev-parse", "HEAD").strip()

    def run_main(self, *args):
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            code = c.main(["--repo", str(self.repo), *args])
        return code, buf.getvalue()

    def test_old_debt_and_new(self):
        base = self.commit(wrap('TextField { id: a }\nTextField { id: b; Accessible.name: qsTr("B") + " " + text }\n'
                                'MouseArea { onClicked: f() }'))
        # the old unnamed field changed its type (old debt), the second lost its name, new ones added
        head = self.commit(wrap('SettingsTextField { id: a }\nTextField { id: b }\n'
                                'MouseArea { onClicked: f() }\nMouseArea { onClicked: g() }\nButton { icon.source: "x.png" }'))
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 1)
        self.assertNotIn("#a", out)
        self.assertIn("src/Page.qml:4: error: no-name: TextField #b", out)
        self.assertIn("src/Page.qml:6: warning: tap-only: MouseArea", out)
        self.assertEqual(out.count("tap-only"), 1)
        self.assertIn("error: no-name: Button", out)
        self.assertIn("already there before the change: 1", out)
        code, out = self.run_main(base, "--head", head, "--no-baseline")
        self.assertIn("#a", out)

    def test_warnings_do_not_fail(self):
        base = self.commit(wrap('Button { text: qsTr("A") }'))
        head = self.commit(wrap('Button { text: qsTr("A") }\nRectangle {\n MouseArea { onClicked: f() }\n}\n'
                                'TextField { placeholderText: qsTr("Name") }'))
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 0, out)
        self.assertIn("errors: 0, warnings: 2", out)

    def test_github_annotations(self):
        base = self.commit(wrap('Button { text: qsTr("A") }'))
        head = self.commit(wrap('Button { text: qsTr("A") }\nButton { icon.source: "x.png" }'))
        code, out = self.run_main(base, "--head", head, "--github")
        self.assertEqual(code, 1)
        self.assertIn("::error file=src/Page.qml,line=4,title=Accessibility%3A no-name::Button: ", out)

    def test_exception(self):
        base = self.commit(wrap('Button { text: qsTr("A") }'))
        head = self.commit(wrap('Button { text: qsTr("A") }\nMenuItem {\n text: qsTr("Close")\n onTriggered: popup.close()\n}'))
        exceptions = self.repo / "exceptions.json"
        exceptions.write_text(json.dumps({"src/Page.qml": [
            {"rule": "no-action", "anchor": "onTriggered: popup.close()", "why": "t"}]}), encoding="utf-8")
        code, out = self.run_main(base, "--head", head, "--exceptions", str(exceptions))
        self.assertEqual(code, 0, out)
        self.assertIn("skipped by exceptions: 1", out)
        self.assertEqual(self.run_main(base, "--head", head, "--exceptions", str(self.repo / "none.json"))[0], 1)

    def test_real_exceptions_file(self):
        # the exceptions file is read, and every entry has a reason
        data = c.load_exceptions(c.EXCEPTIONS)
        for path, items in data.items():
            for x in items:
                self.assertTrue(x["rule"] and x["anchor"] and x["why"], path)

    def test_behind_master(self):
        # a branch behind master: a new unnamed button in master is not its finding
        a = self.commit(wrap('Button { text: qsTr("A") }'))
        master = self.commit(wrap('Button { text: qsTr("A") }\nButton { icon.source: "m.png" }'))
        c.git(self.repo, "checkout", "-q", "-b", "br", a)
        br = self.commit(wrap('Button { text: qsTr("B") }'))
        self.assertEqual(self.run_main(master, "--head", br)[0], 0)

    def test_parent_lost_action(self):
        # the Rectangle lost its role and action; the MouseArea is far from the removed lines
        base = self.commit(wrap('Rectangle {\n id: day\n Accessible.role: Accessible.Button\n Accessible.name: qsTr("Day")\n'
                                ' Accessible.onPressAction: f()\n color: "red"\n width: 10\n height: 10\n'
                                ' MouseArea {\n  anchors.fill: parent\n  onClicked: f()\n }\n}'))
        head = self.commit(wrap('Rectangle {\n id: day\n Accessible.name: qsTr("Day")\n color: "red"\n width: 10\n height: 10\n'
                                ' MouseArea {\n  anchors.fill: parent\n  onClicked: f()\n }\n}'))
        self.assertIn("tap-only: MouseArea", self.run_main(base, "--head", head)[1])

    def test_ancestor_lost_action(self):
        # the action and the role removed from the grandparent; from the parent - at its end, after the children
        base = self.commit(wrap(' Rectangle {\n  id: day\n  Accessible.role: Accessible.Button\n  Accessible.name: qsTr("Day")\n'
                                '  Accessible.onPressAction: f()\n  color: "red"\n  Item {\n   anchors.fill: parent\n'
                                '   MouseArea { anchors.fill: parent; onClicked: f() }\n  }\n }'))
        head = self.commit(wrap(' Rectangle {\n  id: day\n  Accessible.name: qsTr("Day")\n  color: "red"\n  Item {\n'
                                '   anchors.fill: parent\n   MouseArea { anchors.fill: parent; onClicked: f() }\n  }\n }'))
        self.assertIn("tap-only: MouseArea", self.run_main(base, "--head", head)[1])
        base = self.commit(wrap(' Rectangle {\n  Accessible.name: qsTr("Day")\n  MouseArea { anchors.fill: parent; onClicked: f() }\n'
                                '  Text {\n   text: "1"\n  }\n  Accessible.role: Accessible.Button\n  Accessible.onPressAction: f()\n }'))
        head = self.commit(wrap(' Rectangle {\n  Accessible.name: qsTr("Day")\n  MouseArea { anchors.fill: parent; onClicked: f() }\n'
                                '  Text {\n   text: "1"\n  }\n }'))
        self.assertIn("tap-only: MouseArea", self.run_main(base, "--head", head)[1])

    def test_component_root_lost_name(self):
        self.write("UiBtn.qml", "Button {\n property string label\n text: label\n}\n")
        base = self.commit(wrap('UiBtn { label: qsTr("Go") }'))
        self.write("UiBtn.qml", "Button {\n property string label\n}\n")
        head = self.commit()
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 1)
        self.assertIn("src/Page.qml:3: error: no-name: UiBtn", out)

    def test_component_root_lost_action(self):
        self.write("Sw.qml", "SwitchDelegate {\n Accessible.onPressAction: toggle()\n}\n")
        base = self.commit(wrap('Sw { text: qsTr("On") }'))
        self.write("Sw.qml", "SwitchDelegate {\n}\n")
        head = self.commit()
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 1, out)
        self.assertIn("src/Sw.qml:1: error: no-action: SwitchDelegate", out)
        self.assertIn("src/Page.qml:3: warning: no-action: Sw", out)

    def test_new_instance_of_old_component(self):
        # a new settings switch on a component that never had the action: not a failed check
        self.write("Sw.qml", "SwitchDelegate {\n}\n")
        base = self.commit(wrap('Sw { text: qsTr("On") }'))
        head = self.commit(wrap('Sw { text: qsTr("On") }\nSw { text: qsTr("Off") }'))
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 0, out)
        self.assertIn("errors: 0, warnings: 1", out)

    def test_old_debt_survives_text_and_handler_edit(self):
        base = self.commit(wrap('Rectangle {\n MouseArea { onClicked: f() }\n}\nMenuItem { text: qsTr("Open") }\n'
                                'ToolButton { icon.source: "a.png" }'))
        head = self.commit(wrap('Rectangle {\n MouseArea { onClicked: g() }\n}\nMenuItem { text: qsTr("Open file") }\n'
                                'ToolButton { icon.source: "b.png" }'))
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 0, out)
        self.assertIn("errors: 0, warnings: 0", out)
        self.assertIn("already there before the change: 3", out)

    def test_new_error_not_hidden_by_fixed_one(self):
        base = self.commit(wrap("ComboBox { model: m }"))
        head = self.commit(wrap('ComboBox { model: m; Accessible.name: qsTr("M") + displayText }\nSpinBox { from: 0 }'))
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 1)
        self.assertIn("no-name: SpinBox", out)

    def test_fixed_one_does_not_cover_new_neighbour(self):
        # the fixed button keeps its pair in the base, so a new one next to it does not hide behind its debt
        base = self.commit(wrap(' ToolButton { icon.source: "a.png" }'))
        head = self.commit(wrap(' ToolButton { icon.source: "a.png"; Accessible.name: qsTr("A") }\n'
                                ' ToolButton { icon.source: "b.png" }'))
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 1, out)
        self.assertIn("src/Page.qml:4: error: no-name: ToolButton", out)
        # the handler moved to the new button, and the diff aligns its line with it
        base = self.commit(wrap(' ToolButton {\n  icon.source: "a.png"\n  onClicked: stack.pop()\n }'))
        head = self.commit(wrap(' ToolButton {\n  text: qsTr("Back")\n  icon.source: "a2.png"\n }\n'
                                ' ToolButton {\n  icon.source: "b.png"\n  onClicked: stack.pop()\n }'))
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 1, out)
        self.assertIn("src/Page.qml:7: error: no-name: ToolButton", out)

    def test_key_before_place_in_hunk(self):
        # one-line buttons: the place in the hunk must not pair the old button "b" with "a"
        base = self.commit(wrap(' Row { ToolButton { icon.source: "a"; Accessible.name: qsTr("A") } ToolButton { icon.source: "b" } }'))
        head = self.commit(wrap(' Row { ToolButton { icon.source: "b" } }'))
        self.assertEqual(self.run_main(base, "--head", head)[0], 0)
        base = self.commit(wrap(' ToolButton { icon.source: "a"; Accessible.name: qsTr("A") }\n'
                                ' ToolButton { icon.source: "b"; onClicked: b() }'))
        head = self.commit(wrap(' ToolButton { icon.source: "b"; onClicked: bb() }\n'
                                ' ToolButton { icon.source: "c"; Accessible.name: qsTr("C") }'))
        self.assertEqual(self.run_main(base, "--head", head)[0], 0)

    def test_diff_settings_of_user(self):
        # histogram would align "ToolButton {" of the removed button with the new one: the algorithm is fixed
        c.git(self.repo, "config", "diff.algorithm", "histogram")
        base = self.commit(wrap(' ToolButton {\n  icon.source: "a.png"\n  onClicked: a()\n }\n' + ' Item {}\n' * 6))
        head = self.commit(wrap(' Item {}\n' * 6 + ' ToolButton {\n  icon.source: "z.png"\n  onClicked: z()\n }'))
        self.assertEqual(self.run_main(base, "--head", head)[0], 1)

    def test_untracked_and_renamed(self):
        # a renamed file with old debt: no new findings
        self.write("Old.qml", wrap('Rectangle {\n MouseArea { onClicked: f() }\n}\n' + "Item {}\n" * 5))
        base = self.commit(wrap('Button { text: qsTr("A") }'))
        c.git(self.repo, "mv", "src/Old.qml", "src/New.qml")
        head = self.commit()
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 0, out)
        self.assertNotIn("tap-only", out)
        # renamed without changes, with an instance of a changed component: its base is the old path
        self.write("UiBtn.qml", "Button {\n}\n")
        self.write("Use.qml", wrap('UiBtn { icon.source: "x.png" }\n' + "Item {}\n" * 5))
        base = self.commit()
        c.git(self.repo, "mv", "src/Use.qml", "src/Used.qml")
        self.write("UiBtn.qml", "Button {\n width: 1\n}\n")
        head = self.commit()
        code, out = self.run_main(base, "--head", head)
        self.assertEqual(code, 0, out)
        self.assertIn("already there before the change: 1", out)
        # a new file not yet added to git is checked in the working tree
        self.write("Fresh.qml", wrap('Button { icon.source: "x.png" }'))
        code, out = self.run_main(head)
        self.assertEqual(code, 1, out)
        self.assertIn("src/Fresh.qml", out)

    def test_qml_tests_skipped(self):
        base = self.commit(wrap('Button { text: qsTr("A") }'))
        (self.repo / "tst").mkdir()
        (self.repo / "tst" / "tst_page.qml").write_text(wrap('Button { icon.source: "x.png" }'), encoding="utf-8")
        head = self.commit()
        self.assertEqual(self.run_main(base, "--head", head)[0], 0)

    def test_clean_and_not_parsed(self):
        base = self.commit(wrap('Button { text: qsTr("A") }'))
        head = self.commit(wrap('Button { text: qsTr("B") }'))
        self.assertEqual(self.run_main(base, "--head", head)[0], 0)
        # a file the checker cannot read is a warning, not a failed check
        head2 = self.commit(wrap("Button {"))
        code, out = self.run_main(base, "--head", head2)
        self.assertEqual(code, 0, out)
        self.assertIn("warning: not-parsed", out)


if __name__ == "__main__":
    unittest.main()
