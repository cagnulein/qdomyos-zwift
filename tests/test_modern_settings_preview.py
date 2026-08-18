import unittest
from pathlib import Path

ROOT = Path(__file__).parents[1]
QML = ROOT / "src/settings.qml"


class ModernSettingsPreviewTests(unittest.TestCase):
    def setUp(self):
        self.text = QML.read_text(encoding="utf-8")

    def test_preview_is_opt_in_drawer(self):
        self.assertEqual(self.text.count("// MODERN_SETTINGS_PREVIEW_V1"), 1)
        self.assertIn("id: modernSettingsDrawer", self.text)
        self.assertIn("text: qsTr(\"New UI\")", self.text)
        self.assertIn("onClicked: settingsPane.openModernSettingsPreview()", self.text)

    def test_preview_reuses_shared_behavior(self):
        self.assertIn("settingsPane.setSettingValue(entry, checked)", self.text)
        self.assertIn("settingsPane.setSettingValue(entry, selectedValue)", self.text)
        self.assertIn("settingsPane.setVirtualSelection(entry, currentIndex)", self.text)
        self.assertIn("settingsBehavior.setSettingValue(entry, value)", self.text)

    def test_preview_supports_categories_search_and_pages(self):
        self.assertIn("modernSettingsCategories", self.text)
        self.assertIn("id: modernSettingsSearch", self.text)
        self.assertIn("settingsPane.parentDisplayName(entry)", self.text)
        self.assertIn("function modernSettingsParentName()", self.text)
        self.assertIn("stackView.push(entry.target)", self.text)

    def test_preview_avoids_private_material_dialog_color(self):
        self.assertNotIn("Material.dialogColor", self.text)

    def test_legacy_renderer_remains_present(self):
        self.assertIn("id: settingsContent", self.text)
        self.assertIn("id: generalOptionsAccordion", self.text)


if __name__ == "__main__":
    unittest.main()
