"""Static wiring checks for Android/iOS QZ workout-control feedback."""
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


def source(relative):
    return (ROOT / relative).read_text(encoding="utf-8")


class MobileControlHapticsTests(unittest.TestCase):
    def test_off_by_default_and_live_settings_sync(self):
        settings = source("src/settings.qml")
        home = source("src/Home.qml")
        self.assertIn("property bool haptic_feedback_enabled: false", settings)
        self.assertIn("property bool haptic_feedback_enabled: false", home)
        self.assertIn('window.tileSettingChanged("haptic_feedback_enabled"', settings)

    def test_main_controls_and_both_tile_uis(self):
        home = source("src/Home.qml")
        for handler in ("confirmPlus(objectName)", "confirmMinus(objectName)",
                        "confirmLargeButton(objectName)"):
            # OnClicked and accessible press in modern and classic tile components
            self.assertEqual(home.count(handler), 4)
        self.assertIn("start_clicked(); confirmHaptic();", home)
        self.assertIn("stop.onClicked: {\n        confirmHaptic()", home)

    def test_native_android_ios_backends(self):
        cpp = source("src/homeform.cpp")
        java = source("src/android/src/CustomQtActivity.java")
        ios = source("src/ios/haptic_feedback.mm")
        pri = source("src/qdomyos-zwift.pri")
        self.assertIn('activity.callMethod<void>("playControlHaptic", "()V")', cpp)
        self.assertIn("root.performHapticFeedback(kind)", java)
        self.assertIn("UISelectionFeedbackGenerator", ios)
        self.assertIn("ios/haptic_feedback.mm", pri)


if __name__ == "__main__":
    unittest.main()
