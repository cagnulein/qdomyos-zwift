import json
import unittest
from pathlib import Path

ROOT = Path(__file__).parents[1]
QML = (ROOT / "src/settings.qml").read_text(encoding="utf-8")
CATALOG = json.loads((ROOT / "src/settings-catalog.json").read_text(encoding="utf-8"))


class SettingsHierarchyTests(unittest.TestCase):
    def test_hierarchy_is_generated_from_legacy_accordion_structure(self):
        hierarchy = CATALOG.get("legacyHierarchy")
        self.assertIsInstance(hierarchy, dict)
        self.assertEqual(hierarchy.get("source"), "src/settings.qml AccordionElement nesting")
        nodes = hierarchy.get("nodes", [])
        self.assertGreater(len(nodes), 20)
        roots = [n for n in nodes if not n.get("parent")]
        self.assertGreater(len(roots), 1)
        self.assertLess(len(roots), 30)
        names = {n["name"] for n in roots}
        self.assertIn("General Options", names)
        self.assertIn("Bike Options", names)

    def test_nested_legacy_categories_remain_nested(self):
        nodes = {n["key"]: n for n in CATALOG["legacyHierarchy"]["nodes"]}
        bike = next(n for n in nodes.values() if n["name"] == "Bike Options")
        descendants = [n for n in nodes.values() if n.get("parent") == bike["key"]]
        self.assertTrue(descendants, "Bike Options should expose legacy subcategories, not flatten them into root")

    def test_internal_calibration_storage_is_not_rendered(self):
        by_key = {e["key"]: e for e in CATALOG["settings"]}
        for key in ("ergDataPoints", "treadmillDataPoints"):
            self.assertFalse(by_key[key]["visible"])
            self.assertEqual(by_key[key]["control"], "internal")

    def test_modern_qml_uses_generated_hierarchy(self):
        self.assertIn("// MODERN_SETTINGS_LEGACY_HIERARCHY_V2", QML)
        self.assertIn("settingsCatalog.legacyHierarchy", QML)
        self.assertIn("function modernEntryNodeKey(entry)", QML)
        self.assertNotIn("function legacyRootIcon(name)", QML)
        self.assertIn('"__category__:"', QML)


if __name__ == "__main__":
    unittest.main()
