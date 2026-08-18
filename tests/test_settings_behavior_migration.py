import re
import unittest
from pathlib import Path

ROOT = Path(__file__).parents[1]
QML = ROOT / "src/settings.qml"


def function_bodies(text, name):
    pattern = re.compile(r"function\s+" + re.escape(name) + r"\s*\([^)]*\)\s*\{")
    bodies = []
    for match in pattern.finditer(text):
        brace = text.find("{", match.start())
        depth = 0
        for i in range(brace, len(text)):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    bodies.append(text[brace + 1:i])
                    break
    return bodies


class SettingsBehaviorMigrationTests(unittest.TestCase):
    def setUp(self):
        self.text = QML.read_text(encoding="utf-8")

    def test_controller_is_installed_once(self):
        self.assertEqual(self.text.count("// SETTINGS_BEHAVIOR_CONTROLLER_V1"), 1)
        self.assertEqual(self.text.count("id: settingsBehavior"), 1)

    def test_controller_contains_real_mutation_logic(self):
        bodies = function_bodies(self.text, "setSettingValue")
        self.assertGreaterEqual(len(bodies), 2)
        controller_body = bodies[0]
        self.assertIn("settings[entry.key]", controller_body)
        self.assertIn("afterGenericWrite(entry)", controller_body)
        self.assertNotIn("settingsBehavior.setSettingValue", controller_body)

    def test_outer_generic_setter_delegates_only(self):
        bodies = function_bodies(self.text, "setSettingValue")
        outer_body = bodies[-1]
        self.assertIn("settingsBehavior.setSettingValue(entry, value)", outer_body)
        self.assertNotIn("window.settings_restart_to_apply", outer_body)
        self.assertNotIn("settings[entry.key]", outer_body)

    def test_virtual_controller_and_wrapper_are_not_recursive(self):
        bodies = function_bodies(self.text, "setVirtualSelection")
        self.assertGreaterEqual(len(bodies), 2)
        self.assertIn("settings[entry.options[i].sets] = false", bodies[0])
        self.assertNotIn("settingsBehavior.setVirtualSelection", bodies[0])
        self.assertIn("settingsBehavior.setVirtualSelection(entry, index)", bodies[-1])

    def test_restart_policy_defaults_to_legacy_behavior(self):
        self.assertIn("entry.restartRequired === undefined ? true", self.text)
        self.assertIn("window.settings_restart_to_apply = true", self.text)

    def test_legacy_controls_are_not_mass_rewritten(self):
        self.assertIn("settings.weight =", self.text)
        self.assertIn("settings.ftp =", self.text)


if __name__ == "__main__":
    unittest.main()
