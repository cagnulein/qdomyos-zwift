import unittest
from pathlib import Path

ROOT = Path(__file__).parents[1]
QML = ROOT / "src/settings.qml"


class SettingsBehaviorMigrationTests(unittest.TestCase):
    def test_controller_is_installed(self):
        text = QML.read_text(encoding="utf-8")
        self.assertIn("// SETTINGS_BEHAVIOR_CONTROLLER_V1", text)
        self.assertIn("id: settingsBehavior", text)

    def test_generic_setters_delegate_to_controller(self):
        text = QML.read_text(encoding="utf-8")
        self.assertIn("settingsBehavior.setSettingValue(entry, value)", text)
        self.assertIn("settingsBehavior.setVirtualSelection(entry, index)", text)

    def test_restart_policy_defaults_to_legacy_behavior(self):
        text = QML.read_text(encoding="utf-8")
        self.assertIn("entry.restartRequired === undefined ? true", text)
        self.assertIn("window.settings_restart_to_apply = true", text)

    def test_legacy_controls_are_not_mass_rewritten(self):
        text = QML.read_text(encoding="utf-8")
        # Representative legacy direct writes intentionally remain in place.
        self.assertIn("settings.weight =", text)
        self.assertIn("settings.ftp =", text)


if __name__ == "__main__":
    unittest.main()
