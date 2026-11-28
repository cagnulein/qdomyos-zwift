import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from check_settings_key_properties import classify_keys


class SettingsKeyPropertiesTest(unittest.TestCase):
    def test_ordinary_new_setting_requires_a_property(self):
        self.assertEqual(classify_keys(["new_setting"], set()), (["new_setting"], []))
        self.assertEqual(classify_keys(["new_setting"], {"new_setting"}), ([], []))

    def test_cpp_only_flags_must_remain_out_of_qml(self):
        flags = ["log_debug_first_setup", "log_debug_first_setup_since", "wahoo_wheel_question_answered"]
        self.assertEqual(classify_keys(flags, set()), ([], []))
        self.assertEqual(classify_keys(flags, set(flags)), ([], sorted(flags)))
        self.assertEqual(classify_keys([], set(flags)), ([], sorted(flags)))


if __name__ == "__main__":
    unittest.main()
