#!/usr/bin/env python3
"""Regression checks for the iOS 27 TestFlight toolchain and iOS 15 minimum.

This is deliberately stdlib-only so it can run on Linux PR CI before
spending macOS runner time or accessing App Store Connect credentials.
The release workflow also checks the *actual* selected Xcode/iOS SDK.
"""

from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parent.parent
MINIMUM_IOS = (15, 0)


def assert_ios_targets_at_least(project_text: str, expected_count: int) -> None:
    targets = re.findall(
        r"\bIPHONEOS_DEPLOYMENT_TARGET\s*=\s*(\d+(?:\.\d+)?)\s*;",
        project_text,
    )
    if len(targets) != expected_count:
        raise AssertionError(
            f"Expected {expected_count} Xcode iOS deployment targets, got {len(targets)}"
        )
    for target in targets:
        version = tuple(int(part) for part in target.split("."))
        if version < MINIMUM_IOS:
            raise AssertionError(f"Found obsolete iOS deployment target {target}")


class IOSReleaseRequirementsTests(unittest.TestCase):
    def read(self, path: str) -> str:
        return (ROOT / path).read_text(encoding="utf-8")

    def test_qmake_and_qios_backport_target_ios_15(self) -> None:
        for path, expected in (
            ("qdomyos-zwift.pro", "QMAKE_IOS_DEPLOYMENT_TARGET = 15.0"),
            ("src/qdomyos-zwift.pro", "QMAKE_IOS_DEPLOYMENT_TARGET = 15.0"),
            (
                "build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/ci_scripts/ci_pre_xcodebuild.sh",
                "export QMAKE_IOS_DEPLOYMENT_TARGET=15.0",
            ),
            ("tools/build_qios_uiscene.sh", "QMAKE_IOS_DEPLOYMENT_TARGET=15.0"),
        ):
            with self.subTest(path=path):
                self.assertIn(expected, self.read(path))

    def test_committed_xcode_project_has_no_older_ios_targets(self) -> None:
        project = self.read(
            "build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/"
            "qdomyoszwift.xcodeproj/project.pbxproj"
        )
        # Main app (Debug/Release), generated target, and widget (Debug/Release).
        assert_ios_targets_at_least(project, expected_count=6)
        self.assertEqual(
            len(re.findall(r"IPHONEOS_DEPLOYMENT_TARGET = 15\.0;", project)), 4
        )
        # The widget intentionally keeps its existing iOS 26 minimum.
        self.assertEqual(
            len(re.findall(r"IPHONEOS_DEPLOYMENT_TARGET = 26\.0;", project)), 2
        )

    def test_older_ios_target_is_rejected(self) -> None:
        with self.assertRaisesRegex(AssertionError, "12.0"):
            assert_ios_targets_at_least(
                "IPHONEOS_DEPLOYMENT_TARGET = 12.0;", expected_count=1
            )

    def test_release_uses_xcode_27_and_checks_actual_sdk(self) -> None:
        workflow = self.read(".github/workflows/release-testflight.yml")
        self.assertIn("runs-on: xcode-27", workflow)
        self.assertIn("xcodebuild -version", workflow)
        self.assertIn("xcrun --sdk iphoneos --show-sdk-version", workflow)
        self.assertIn('"$IOS_SDK_VERSION" != 27.*', workflow)
        self.assertIn('"$XCODE_VERSION" != 27.*', workflow)

    def test_fastlane_has_altool_false_failure_fix(self) -> None:
        # Fastlane 2.240.0 includes fastlane/fastlane#30196. Older
        # versions reject a completed upload if altool prints a diagnostic.
        workflow = self.read(".github/workflows/release-testflight.yml")
        versions = [
            line.strip().split(" --version ", 1)[1].split()[0]
            for line in workflow.splitlines()
            if line.strip().startswith("gem install fastlane --version ")
        ]
        self.assertEqual(len(versions), 1)
        version = tuple(int(part) for part in versions[0].split("."))
        self.assertGreaterEqual(version, (2, 240, 0))

    def test_qt_bundle_is_the_xcode27_uiscene_build(self) -> None:
        setup = self.read(
            "build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug/"
            "ci_scripts/ci_post_clone.sh"
        )
        self.assertIn('qt-5.15.2-xcode27-uiscene.tar.xz', setup)


if __name__ == "__main__":
    unittest.main()
