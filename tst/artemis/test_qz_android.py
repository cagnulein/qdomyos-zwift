"""Pilot end-to-end Android UI tests for QZ using Google Artemis.

These tests are intentionally opt-in. They require an Artemis daemon connected to
an Android emulator or dedicated test device with QZ already installed.
"""

from __future__ import annotations

import os
import unittest

from artemis_client import ArtemisClient


QZ_PACKAGE = os.environ.get("QZ_ANDROID_PACKAGE", "org.cagnulen.qdomyoszwift")
ARTEMIS_BASE_URL = os.environ.get("ARTEMIS_BASE_URL", "http://127.0.0.1:8000")
ARTEMIS_DEVICE_SERIAL = os.environ.get("ARTEMIS_DEVICE_SERIAL") or None
ARTEMIS_PROFILE = os.environ.get("ARTEMIS_PROFILE", "flash")
ARTEMIS_TIMEOUT = float(os.environ.get("ARTEMIS_TIMEOUT", "900"))
FTMS_BIKE_EXPECTED = os.environ.get("QZ_FTMS_BIKE_EXPECTED", "ARTEMIS_TEST_BIKE")


class QZArtemisTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self) -> None:
        self.client = ArtemisClient(
            ARTEMIS_BASE_URL,
            token=os.environ.get("ARTEMIS_TOKEN"),
            device_serial=ARTEMIS_DEVICE_SERIAL,
            default_profile=ARTEMIS_PROFILE,
        )

    async def run_qz_task(self, goal: str, expected_output: str) -> None:
        result = await self.client.run(
            goal,
            expected_output=expected_output,
            locked_app_package=QZ_PACKAGE,
            timeout=ARTEMIS_TIMEOUT,
        )
        self.assertTrue(
            result.succeeded,
            msg=(
                f"Artemis task failed: status={result.status!r}, "
                f"error={result.error!r}, output={result.output!r}, "
                f"trace_id={result.trace_id!r}"
            ),
        )

    async def test_ftms_bike_value_is_consistent_in_search(self) -> None:
        expected = FTMS_BIKE_EXPECTED
        await self.run_qz_task(
            f"""
Open QZ Fitness and navigate to Settings. Do not change any setting.
Find the setting named "FTMS Bike" in the normal settings hierarchy and verify
that its displayed value is exactly "{expected}". Then use the Settings search
field and search for "ftms". In the filtered results find "FTMS Bike" again and
verify that its displayed value is still exactly "{expected}" and is not
"Disabled". Fail the task if the value differs between the normal view and the
search results, if either value is missing, or if QZ crashes.
""".strip(),
            expected_output=(
                f"PASS only if FTMS Bike shows {expected!r} both normally and "
                "after searching for 'ftms', with no crash."
            ),
        )

    async def test_experimental_settings_persist_after_reopen(self) -> None:
        await self.run_qz_task(
            """
Open QZ Fitness and navigate to Settings > Experimental Features. Locate
"Android Notification" and "Fake Device". Enable each one if it is currently
disabled; leave it enabled if it is already enabled. Return to the QZ main
screen, then reopen Settings > Experimental Features and verify that both
settings are still enabled. Fail the task if either value does not persist or if
QZ crashes.
""".strip(),
            expected_output=(
                "PASS only if Android Notification and Fake Device are both "
                "enabled after leaving Settings and reopening it, with no crash."
            ),
        )

    async def test_settings_navigation_smoke(self) -> None:
        await self.run_qz_task(
            """
Run a QZ Fitness settings smoke test without changing settings: start from the
main screen, open Settings, use the search field to search for "ftms", verify
that at least one relevant result is shown, clear the search, open Experimental
Features, scroll through that section, then return to the QZ main screen.
During the whole flow verify that QZ remains responsive and that no Android
crash dialog, ANR dialog, blank page, or unrecoverable navigation error appears.
""".strip(),
            expected_output=(
                "PASS only if the complete Settings/search/Experimental Features "
                "navigation finishes and QZ remains responsive with no crash or ANR."
            ),
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
