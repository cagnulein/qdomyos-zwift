import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parents[1] / "scripts"))
import update_garmin_fit_devices as updater


ROOT = Path(__file__).parents[1]
QML = (ROOT / "src/settings.qml").read_text(encoding="utf-8")
PROFILE = (ROOT / "src/fit-sdk/fit_profile.hpp").read_text(encoding="utf-8")


class GarminFitUpdaterTest(unittest.TestCase):
    def setUp(self):
        self.catalog = updater.parse_catalog(PROFILE)

    def test_parses_repository_qml(self):
        start, end = updater.locate_control(QML)
        devices = updater.parse_current(QML[start:end])
        self.assertGreater(len(devices), 100)
        self.assertEqual(devices[20].macro, "EDGE_830")

    def test_adds_one_and_is_idempotent(self):
        catalog = dict(self.catalog, FENIX_TEST=60000)
        updated, added, old = updater.update_qml(QML, catalog)
        self.assertEqual([d.macro for d in added], ["FENIX_TEST"])
        self.assertTrue(all(d.macro in updated for d in old))
        again, second, _ = updater.update_qml(updated, catalog)
        self.assertEqual(again, updated)
        self.assertEqual(second, [])

    def test_multiple_are_deterministic_by_product_number(self):
        catalog = dict(self.catalog, VENU_FUTURE=60002, EDGE_FUTURE=60001)
        updated, added, _ = updater.update_qml(QML, catalog)
        self.assertEqual([d.macro for d in added], ["EDGE_FUTURE", "VENU_FUTURE"])
        self.assertLess(updated.index("Edge Future"), updated.index("Venu Future"))

    def test_upstream_disappearance_preserves_existing(self):
        updated, added, old = updater.update_qml(QML, self.catalog)
        self.assertEqual(added, [])
        self.assertEqual(updater.parse_current(updated[updater.locate_control(updated)[0]:updater.locate_control(updated)[1]]), old)

    def test_rejects_duplicate_and_empty_catalog(self):
        with self.assertRaises(updater.UpdateError):
            updater.parse_catalog("")
        duplicate = "#define FIT_GARMIN_PRODUCT_A ((FIT_GARMIN_PRODUCT)1)\n#define FIT_GARMIN_PRODUCT_A ((FIT_GARMIN_PRODUCT)2)"
        with self.assertRaises(updater.UpdateError):
            updater.parse_catalog(duplicate, minimum=1)

    def test_missing_and_multiple_target_rejected(self):
        with self.assertRaises(updater.UpdateError):
            updater.locate_control("ComboBox {}")
        with self.assertRaises(updater.UpdateError):
            updater.locate_control(QML + QML)

    def test_malformed_model_rejected(self):
        broken = QML.replace('"D2Airvenu",', 'someFunction(),', 1)
        with self.assertRaises(updater.UpdateError):
            updater.update_qml(broken, self.catalog)

    def test_unrelated_qml_is_byte_identical(self):
        updated, _, _ = updater.update_qml(QML, dict(self.catalog, FR_FUTURE=60003))
        old_start, old_end = updater.locate_control(QML)
        new_start, new_end = updater.locate_control(updated)
        self.assertEqual(QML[:old_start], updated[:new_start])
        self.assertEqual(QML[old_end:], updated[new_end:])


if __name__ == "__main__":
    unittest.main()
