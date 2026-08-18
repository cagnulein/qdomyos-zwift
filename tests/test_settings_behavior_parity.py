import json
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).parents[1]
QML = ROOT / "src/settings.qml"
CATALOG = ROOT / "src/settings-catalog.json"


class SettingsBehaviorParityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.qml = QML.read_text(encoding="utf-8")
        cls.catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
        cls.by_key = {entry["key"]: entry for entry in cls.catalog["settings"]}

    def test_parity_controller_installed(self):
        self.assertEqual(self.qml.count("// SETTINGS_BEHAVIOR_PARITY_V2"), 1)
        self.assertIn("function displayValue(entry)", self.qml)
        self.assertIn("function displaySettingValue(entry)", self.qml)

    def test_restart_policy_is_explicit_for_every_persistent_setting(self):
        self.assertEqual(len(self.by_key), 998)
        missing = [key for key, entry in self.by_key.items() if "restartRequired" not in entry]
        self.assertEqual(missing, [])
        self.assertTrue(all(isinstance(entry["restartRequired"], bool) for entry in self.by_key.values()))

    def test_virtual_restart_policy_is_explicit(self):
        missing = [entry["key"] for entry in self.catalog.get("virtualSettings", []) if "restartRequired" not in entry]
        self.assertEqual(missing, [])

    def test_known_unit_conversions_are_in_shared_controller(self):
        for key in [
            "weight", "bike_weight", "height", "autolap_distance",
            "treadmill_speed_min", "treadmill_speed_max", "treadmill_step_speed",
            "peloton_treadmill_running_min_speed", "peloton_treadmill_walking_min_speed",
            "pacef_1mile", "pacef_5km", "pacef_10km", "pacef_halfmarathon", "pacef_marathon",
        ]:
            self.assertIn('key === "' + key + '"', self.qml)

    def test_known_multi_setting_side_effects_are_in_shared_controller(self):
        for key in [
            "domyos_bike_500_profile_v1", "domyos_bike_500_profile_v2",
            "kingsmith_encrypt_v2", "kingsmith_encrypt_v3", "kingsmith_encrypt_v4",
            "kingsmith_encrypt_v5", "kingsmith_encrypt_g1_walking_pad",
            "peloton_auto_start_with_intro", "peloton_auto_start_without_intro",
            "zwift_ocr", "zwift_ocr_climb_portal", "zwift_workout_ocr",
            "zwift_play_emulator", "watt_bike_emulator",
            "watt_offset", "watt_gain", "power_sensor_name",
        ]:
            self.assertIn(key, self.qml)

    def test_modern_ui_is_default_but_legacy_still_exists(self):
        self.assertIn("Qt.callLater(function() { settingsPane.openModernSettingsPreview() })", self.qml)
        self.assertIn("id: generalOptionsAccordion", self.qml)
        self.assertIn("id: settingsContent", self.qml)

    def test_settings_assignments_are_not_mistaken_for_comparisons(self):
        assignment = re.compile(r"settings\.([A-Za-z_][A-Za-z0-9_]*)\s*=(?!=)")
        self.assertEqual(assignment.findall("if (settings.foo === 1) return 0"), [])
        self.assertEqual(assignment.findall("settings.foo = 1"), ["foo"])


if __name__ == "__main__":
    unittest.main()

# phase4-trigger-v2
