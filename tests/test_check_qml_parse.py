import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).parents[1] / "tools"))
import check_qml_parse as c

VALID = """import QtQuick 2.15
Item {
    Accessible.name: "a"
    width: 10
}
"""

DUPLICATE_ATTACHED = """import QtQuick 2.15
Item {
    Accessible.name: "a"
    width: 10
    Accessible.name: "b"
}
"""

DUPLICATE_PROPERTY = """import QtQuick 2.15
Item {
    width: 10
    width: 20
}
"""

TWO_DUPLICATES = """import QtQuick 2.15
Item {
    width: 10
    width: 20
    height: 10
    height: 20
}
"""

MISSING_BRACE = """import QtQuick 2.15
Item {
    Item {
        width: 10
}
"""

MISSING_MODULE = """import QtQuick 2.15
import ModuleThatDoesNotExist 1.0
Item {
    TypeThatDoesNotExist { width: 10 }
}
"""

DUPLICATE_WITH_MISSING_MODULE = """import ModuleThatDoesNotExist 1.0
Item {
    Accessible.name: "a"
    Accessible.name: "b"
}
"""


class ParseError(unittest.TestCase):
    def test_compiler_line(self):
        text = "Error compiling qml file: /tmp/tree/src/main.qml:1451:13: error: Property value set multiple times"
        self.assertEqual(c.parse_error(text), (1451, 13, "Property value set multiple times"))

    def test_path_with_colon(self):
        text = "Error compiling qml file: C:/src/main.qml:5:1: error: Expected token `}'"
        self.assertEqual(c.parse_error(text), (5, 1, "Expected token `}'"))

    def test_second_error_of_a_file_has_no_prefix(self):
        self.assertEqual(c.parse_error("/tmp/main.qml:9:5: error: Bad"), (9, 5, "Bad"))

    def test_other_output_is_not_parsed(self):
        self.assertIsNone(c.parse_error("qmlcachegen: Unknown option 'x'."))
        self.assertIsNone(c.parse_error("Error compiling qml file: no place here"))
        self.assertIsNone(c.parse_error("Error compiling qml file: main.qml:a:b: error: text"))


class Report(unittest.TestCase):
    def printed(self, text, github):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            c.report("src/main.qml", text, github)
        return out.getvalue().splitlines()

    def test_annotation(self):
        text = "Error compiling qml file: /tmp/main.qml:7:2: error: Bad"
        self.assertEqual(
            self.printed(text, True),
            ["src/main.qml:7:2: Bad", "::error file=src/main.qml,line=7,col=2::Bad"],
        )

    def test_other_output_is_kept(self):
        self.assertEqual(self.printed("something else", False), ["src/main.qml: something else"])


@unittest.skipUnless(c.find_compiler(), "qmlcachegen is not installed")
class Compile(unittest.TestCase):
    def run_on(self, **sources):
        """(exit code, output) of the checker on a tree of the given QML sources"""
        with tempfile.TemporaryDirectory() as root:
            src = Path(root) / "src"
            src.mkdir()
            for name, text in sources.items():
                (src / f"{name}.qml").write_text(text)
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                code = c.main([root])
        return code, out.getvalue()

    def assert_fails_at(self, source, place):
        code, out = self.run_on(Page=source)
        self.assertEqual(code, 1, out)
        self.assertIn(f"src/Page.qml:{place}: ", out)

    def test_valid(self):
        self.assertEqual(self.run_on(Page=VALID)[0], 0)

    def test_duplicate_attached_property(self):
        self.assert_fails_at(DUPLICATE_ATTACHED, "5:5")

    def test_duplicate_property(self):
        self.assert_fails_at(DUPLICATE_PROPERTY, "4:5")

    def test_two_errors_in_one_file(self):
        code, out = self.run_on(Page=TWO_DUPLICATES)
        self.assertEqual(code, 1, out)
        self.assertIn("src/Page.qml:4:5: ", out)
        self.assertIn("src/Page.qml:6:5: ", out)
        self.assertIn("QML files: 1, with errors: 1", out)

    def test_syntax_error(self):
        self.assert_fails_at(MISSING_BRACE, "6:1")

    def test_missing_module_does_not_fail(self):
        self.assertEqual(self.run_on(Page=MISSING_MODULE)[0], 0)

    def test_duplicate_found_with_missing_module(self):
        self.assert_fails_at(DUPLICATE_WITH_MISSING_MODULE, "4:5")

    def test_every_file_is_reported(self):
        code, out = self.run_on(Bad1=DUPLICATE_PROPERTY, Good=VALID, Bad2=MISSING_BRACE)
        self.assertEqual(code, 1)
        self.assertIn("src/Bad1.qml:", out)
        self.assertIn("src/Bad2.qml:", out)
        self.assertIn("QML files: 3, with errors: 2", out)


class NothingToCheck(unittest.TestCase):
    @unittest.skipUnless(c.find_compiler(), "qmlcachegen is not installed")
    def test_no_files_is_an_error(self):
        with tempfile.TemporaryDirectory() as root:
            with contextlib.redirect_stdout(io.StringIO()):
                self.assertEqual(c.main([root]), 2)


if __name__ == "__main__":
    unittest.main()
