"""Guardrails for opt-in haptic confirmation with screen readers."""
import pathlib
import re
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


def source(relative):
    return (ROOT / relative).read_text(encoding="utf-8")


class MobileControlHapticsTests(unittest.TestCase):
    def test_default_off_and_live_settings(self):
        settings = source("src/settings.qml")
        home = source("src/Home.qml")
        self.assertIn("property bool haptic_feedback_enabled: false", settings)
        self.assertIn("property bool haptic_feedback_enabled: false", home)
        self.assertIn('window.tileSettingChanged("haptic_feedback_enabled"', settings)
        self.assertIn("onHaptic_feedback_enabledChanged()", settings)
        self.assertIn("checked: settings.haptic_feedback_enabled", settings)
        self.assertIn('if (settings.haptic_feedback_enabled &&', home)
        self.assertIn('(Qt.platform.os === "android" || Qt.platform.os === "ios")', home)

    def test_pointer_and_accessibility_share_one_action(self):
        home = source("src/Home.qml")
        for handler in ("confirmPlus(objectName)", "confirmMinus(objectName)",
                        "confirmLargeButton(objectName)"):
            self.assertEqual(home.count(handler), 4)
            self.assertIn("onClicked: " + handler, home)
            self.assertIn("Accessible.onPressAction: { " + handler + " }", home)
        self.assertIn("function confirmPlus(name) { plus_clicked(name); confirmHaptic() }", home)
        self.assertIn("function confirmMinus(name) { minus_clicked(name); confirmHaptic() }", home)
        self.assertIn("start.clicked()", source("src/HomeForm.ui.qml"))
        self.assertIn("stop.clicked()", source("src/HomeForm.ui.qml"))
        self.assertIn("start.onClicked: { start_clicked(); confirmHaptic(); }", home)
        self.assertIn("stop.onClicked:", home)
        self.assertIn("lap.onClicked:", home)

    def test_native_mobile_backends_and_throttle(self):
        cpp = source("src/homeform.cpp")
        java = source("src/android/src/CustomQtActivity.java")
        ios = source("src/ios/haptic_feedback.mm")
        pri = source("src/qdomyos-zwift.pri")
        self.assertIn('activity.callMethod<void>("playControlHaptic", "()V")', cpp)
        self.assertIn("runOnUiThread", java)
        self.assertIn("root.performHapticFeedback(kind)", java)
        self.assertIn("QZ_HAPTIC_EVENT performed=", java)
        self.assertIn("HapticFeedbackConstants.CONFIRM", java)
        self.assertIn("HapticFeedbackConstants.VIRTUAL_KEY", java)
        self.assertIn("UISelectionFeedbackGenerator", ios)
        self.assertIn("dispatch_get_main_queue()", ios)
        self.assertIn("ios/haptic_feedback.mm", pri)
        self.assertIn("sinceLastFeedback.elapsed() < 120", cpp)

    def test_emulator_exercises_haptic_behavior_not_just_source(self):
        workflow = source(".github/workflows/main.yml")
        script = source(".github/scripts/android_haptic_accessibility_test.sh")
        java = source(".github/scripts/AccessibilityTest.java")
        self.assertIn("sh .github/scripts/android_haptic_accessibility_test.sh", workflow)
        for step in (
            "focus:Increase_Resistance",
            "Increase_Resistance=Resistance:_1",
            "Vibration_feedback_on_controls",
            "Increase_Speed", "Decrease_Speed",
            "Increase_Inclination", "Decrease_Inclination",
            "QZ_HAPTIC_EVENT",
        ):
            self.assertIn(step, script)
        self.assertIn("ACTION_ACCESSIBILITY_FOCUS", java)
        self.assertIn('press.startsWith("focus:")', java)
        self.assertIn("assert_qz_foreground", script)
        self.assertIn("assert_haptics 0", script)
        self.assertIn("run_a11y Start=Pause", script)
        self.assertIn("run_a11y Lap", script)
        self.assertIn("configure treadmill true modern", script)
        self.assertIn("assert_haptics 7", script)


if __name__ == "__main__":
    unittest.main()
