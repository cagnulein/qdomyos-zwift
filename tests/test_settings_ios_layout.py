import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class SettingsIosLayoutTests(unittest.TestCase):
    def test_modern_renderer_uses_grouped_ios_layout(self):
        text = (ROOT / "src/settings.qml").read_text(encoding="utf-8")
        self.assertIn("MODERN_SETTINGS_IOS_GROUPED_V3", text)
        self.assertIn("function modernCardColor()", text)
        self.assertIn("function modernPageColor()", text)
        self.assertIn("function openModernCatalogPage(entry)", text)
        self.assertIn("items.sort(function(a, b) { return modernItemOrder(a) - modernItemOrder(b) })", text)
        self.assertIn('modelData.catalogKind === "page"', text)

    def test_root_new_page_elements_are_preserved(self):
        catalog = json.loads((ROOT / "src/settings-catalog.json").read_text(encoding="utf-8"))
        layout = catalog["legacyLayout"]
        roots = {entry["key"]: entry for entry in layout["rootPages"]}
        self.assertIn("page_tiles_options", roots)
        self.assertEqual("settings-tiles.qml", roots["page_tiles_options"]["target"])

    def test_secondary_settings_pages_stay_in_modern_renderer(self):
        catalog = json.loads((ROOT / "src/settings-catalog.json").read_text(encoding="utf-8"))
        layout = catalog["legacyLayout"]
        self.assertEqual("settings-tiles.qml", layout["pageParentTargetByKey"]["page_keyboard_shortcuts"])
        self.assertTrue(any(source == "settings-tiles.qml" for source in layout["sourceFileByKey"].values()))

    def test_hierarchy_covers_all_legacy_accordion_types(self):
        text = (ROOT / "scripts/settings_hierarchy.py").read_text(encoding="utf-8")
        self.assertIn("StaticAccordionElement", text)
        self.assertIn("AccordionCheckElement", text)

    def test_experimental_is_root_and_its_children_are_not(self):
        catalog = json.loads((ROOT / "src/settings-catalog.json").read_text(encoding="utf-8"))
        nodes = {node["key"]: node for node in catalog["legacyHierarchy"]["nodes"]}
        self.assertIn("experimentalFeatureAccordion", nodes)
        self.assertIsNone(nodes["experimentalFeatureAccordion"]["parent"])
        for child in ("mqttAccordion", "oscAccordion", "templateSettingsAccordion"):
            self.assertIn(child, nodes)
            self.assertEqual("experimentalFeatureAccordion", nodes[child]["parent"])

    def test_root_is_text_only_and_legacy_ui_is_hidden(self):
        text = (ROOT / "src/settings.qml").read_text(encoding="utf-8")
        self.assertIn("MODERN_SETTINGS_ROOT_TEXT_ONLY_V4", text)
        self.assertNotIn("function legacyRootIcon(", text)
        self.assertNotIn("function legacyRootIconColor(", text)
        self.assertIn("property bool legacySettingsUiEnabled: false", text)
        self.assertIn("visible: legacySettingsUiEnabled && !settingsSearchActive", text)


if __name__ == "__main__":
    unittest.main()
