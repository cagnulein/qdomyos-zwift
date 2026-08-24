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

    def test_navigation_history_preserves_scroll_position(self):
        self.assertEqual(self.text.count("// MODERN_SETTINGS_NAVIGATION_STATE_V5"), 1)
        self.assertIn("property var modernSettingsHistory: []", self.text)
        self.assertIn("scrollY: modernCurrentScrollY()", self.text)
        self.assertIn("modernCategoryList.contentY = state.scrollY", self.text)
        self.assertIn("modernItemList.contentY = state.scrollY", self.text)
        self.assertIn("modernPushNavigationState()", self.text)
        self.assertIn("modernApplyNavigationState(state)", self.text)

    def test_external_pages_resume_modern_settings(self):
        self.assertIn("property bool modernSettingsAwaitingExternalReturn: false", self.text)
        self.assertIn("StackView.onActivated", self.text)
        self.assertIn("settingsPane.resumeModernSettingsAfterExternalPage()", self.text)
        self.assertIn("modernSettingsExternalReturnState = modernCaptureNavigationState()", self.text)
        self.assertIn("modernSettingsAwaitingExternalReturn = true", self.text)
        self.assertIn("modernSettingsDrawer.open()", self.text)

    def test_search_back_is_handled_before_navigation_history(self):
        start = self.text.index("        function modernSettingsBack() {")
        end = self.text.index("        function showSettingsSearch()", start)
        body = self.text[start:end]
        self.assertLess(body.index("modernSettingsSearch.text.length > 0"), body.index("modernSettingsHistory.length > 0"))

    def test_preview_avoids_private_material_dialog_color(self):
        self.assertNotIn("Material.dialogColor", self.text)

    def test_legacy_renderer_remains_present(self):
        self.assertIn("id: settingsContent", self.text)
        self.assertIn("id: generalOptionsAccordion", self.text)


if __name__ == "__main__":
    unittest.main()
