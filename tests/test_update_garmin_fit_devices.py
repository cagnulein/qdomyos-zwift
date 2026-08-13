import sys
import tempfile
import unittest
from unittest import mock
from pathlib import Path

sys.dont_write_bytecode = True
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
        catalog = dict(self.catalog, FENIX_TEST=5000)
        updated, added, old = updater.update_qml(QML, catalog)
        self.assertIn("FENIX_TEST", [d.macro for d in added])
        self.assertTrue(all(d.macro in updated for d in old))
        again, second, _ = updater.update_qml(updated, catalog)
        self.assertEqual(again, updated)
        self.assertEqual(second, [])

    def test_multiple_are_deterministic_by_product_number(self):
        catalog = dict(self.catalog, VENU_FUTURE=5002, EDGE_FUTURE=5001)
        updated, added, _ = updater.update_qml(QML, catalog)
        self.assertIn("EDGE_FUTURE", [d.macro for d in added])
        self.assertIn("VENU_FUTURE", [d.macro for d in added])
        self.assertLess(updated.index("Edge Future"), updated.index("Venu Future"))

    def test_upstream_disappearance_preserves_existing(self):
        start, end = updater.locate_control(QML)
        old = updater.parse_current(QML[start:end])
        catalog = {d.macro: d.product for d in old if d.macro not in ("Tacx", "Zwift")}
        catalog.pop("FENIX8")
        updated, added, _ = updater.update_qml(QML, catalog)
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
        updated, _, _ = updater.update_qml(QML, dict(self.catalog, FR_FUTURE=5003))
        old_start, old_end = updater.locate_control(QML)
        new_start, new_end = updater.locate_control(updated)
        self.assertEqual(QML[:old_start], updated[:new_start])
        self.assertEqual(QML[old_end:], updated[new_end:])

    def test_product_number_is_not_treated_as_chronology(self):
        _, added, _ = updater.update_qml(QML, dict(self.catalog, FENIX_FUTURE=13))
        self.assertIn("FENIX_FUTURE", [d.macro for d in added])

    def test_structured_variants_and_non_devices_are_excluded(self):
        catalog = dict(
            self.catalog,
            FENIX_FUTURE_JPN=5004,
            EDGE_FUTURE_BONTRAGER=5005,
            FR225_SINGLE_BYTE_PRODUCT_ID=14,
        )
        _, added, _ = updater.update_qml(QML, catalog)
        macros = {d.macro for d in added}
        self.assertTrue(macros.isdisjoint(catalog.keys() - self.catalog.keys()))

    def test_cli_reports_if_qml_changed(self):
        with tempfile.TemporaryDirectory() as directory:
            qml = Path(directory) / "settings.qml"
            catalog = Path(directory) / "fit_profile.hpp"
            output = Path(directory) / "github-output"
            qml.write_text(QML, encoding="utf-8")
            catalog.write_text(PROFILE, encoding="utf-8")
            argv = [
                "update_garmin_fit_devices.py", "--qml", str(qml),
                "--catalog", str(catalog), "--github-output", str(output),
            ]
            with mock.patch.object(sys, "argv", argv):
                self.assertEqual(updater.main(), 0)
            self.assertEqual(output.read_text(encoding="utf-8"), "changed=true\n")
            with mock.patch.object(sys, "argv", argv):
                self.assertEqual(updater.main(), 0)
            self.assertTrue(output.read_text(encoding="utf-8").endswith("changed=false\n"))


if __name__ == "__main__":
    unittest.main()
