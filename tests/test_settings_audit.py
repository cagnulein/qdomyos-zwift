import sys
import unittest
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).parents[1] / "scripts"))
import settings_audit


class SettingsAuditParserTests(unittest.TestCase):
    def test_parser_preserves_order_type_and_default(self):
        text = '''
        Settings {
            property bool first: false
            property real gain: 1.0 // explanatory comment
            property string url: "https://example.test/a//b"
        }
        '''
        got = settings_audit.parse_declarations(text, "src/settings.qml")
        self.assertEqual(
            [item.abi_tuple() for item in got],
            [
                ("first", "bool", "false"),
                ("gain", "real", "1.0"),
                ("url", "string", '"https://example.test/a//b"'),
            ],
        )

    def test_commented_properties_are_ignored(self):
        text = '''
        // property bool removed: false
        property bool kept: true
        '''
        got = settings_audit.parse_declarations(text, "x.qml")
        self.assertEqual([item.key for item in got], ["kept"])

    def test_append_only_change_is_allowed(self):
        base = {path: [] for path in settings_audit.SETTINGS_FILES}
        head = {path: [] for path in settings_audit.SETTINGS_FILES}
        path = settings_audit.SETTINGS_FILES[0]
        base[path] = [settings_audit.SettingDecl(path, 1, "bool", "a", "false")]
        head[path] = base[path] + [settings_audit.SettingDecl(path, 2, "int", "b", "1")]
        self.assertEqual(settings_audit.compare_abi(base, head), [])

    def test_reorder_or_change_fails(self):
        base = {path: [] for path in settings_audit.SETTINGS_FILES}
        head = {path: [] for path in settings_audit.SETTINGS_FILES}
        path = settings_audit.SETTINGS_FILES[0]
        base[path] = [
            settings_audit.SettingDecl(path, 1, "bool", "a", "false"),
            settings_audit.SettingDecl(path, 2, "int", "b", "1"),
        ]
        head[path] = [
            settings_audit.SettingDecl(path, 1, "int", "b", "1"),
            settings_audit.SettingDecl(path, 2, "bool", "a", "false"),
        ]
        self.assertTrue(settings_audit.compare_abi(base, head))


if __name__ == "__main__":
    unittest.main()
