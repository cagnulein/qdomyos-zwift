#!/usr/bin/env python3
"""Fail closed if a TestFlight build drops the QZ Watch or widget targets.

The simulator-only CI workaround intentionally removes the WatchKit embed step.
Never apply it to the signed distribution archive.
"""
import argparse
import plistlib
import re
import sys
import zipfile
from pathlib import Path


EXPECTED = {
    "Info.plist": "org.cagnulein.qdomyoszwift",
    "Watch/watchkit.app/Info.plist": "org.cagnulein.qdomyoszwift.watchkitapp",
    "Watch/watchkit.app/PlugIns/watchkit Extension.appex/Info.plist":
        "org.cagnulein.qdomyoszwift.watchkitapp.watchkitextension",
    "PlugIns/QZWidgetExtension.appex/Info.plist":
        "org.cagnulein.qdomyoszwift.QZWidget",
}


def require(ok, message):
    if not ok:
        raise ValueError(message)


def verify_project(path):
    project = Path(path).read_text(encoding="utf-8")
    native = project.split("/* Begin PBXNativeTarget section */", 1)[1].split(
        "/* End PBXNativeTarget section */", 1
    )[0]

    def target(name):
        match = re.search(
            r"/\* " + re.escape(name) + r" \*/ = \{(.*?)\n\s*\};",
            native, flags=re.S,
        )
        require(match is not None, f"Missing Xcode target: {name}")
        return match.group(1)

    app = target("qdomyoszwift")
    watch = target("watchkit")
    target("watchkit Extension")
    target("QZWidgetExtension")
    for phase in ("Embed Watch Content", "Embed Foundation Extensions"):
        require(f"/* {phase} */" in app,
                f"App target does not include build phase: {phase}")
    require("/* Embed App Extensions */" in watch,
            "Watch app does not embed its WatchKit extension")

    # The copy phase alone is not sufficient: the host must also depend on
    # the WatchKit app. The old simulator workaround removed both references.
    dep_section = project.split("/* Begin PBXTargetDependency section */", 1)[1].split(
        "/* End PBXTargetDependency section */", 1
    )[0]
    def require_dependency(parent, child):
        dependency = re.search(
            r"^\s*([A-F0-9]{24}) /\* PBXTargetDependency \*/ = "
            r"\{[^}]*target = [A-F0-9]{24} /\* "
            + re.escape(child) + r" \*/;",
            dep_section, flags=re.M,
        )
        require(dependency is not None, f"Missing target dependency: {child}")
        require(dependency.group(1) + " /* PBXTargetDependency */" in parent,
                f"Parent target no longer depends on {child}")

    require_dependency(app, "watchkit")
    require_dependency(watch, "watchkit Extension")
    require_dependency(app, "QZWidgetExtension")
    print("Xcode project: iOS, WatchKit app, WatchKit extension and widget preserved")


def verify_bundles(read_plist, location):
    info = {}
    for relative, bundle_id in EXPECTED.items():
        try:
            info[relative] = plistlib.loads(read_plist(relative))
        except (FileNotFoundError, KeyError) as exc:
            raise ValueError(f"{location}: missing {relative}") from exc
        actual = info[relative].get("CFBundleIdentifier")
        require(actual == bundle_id,
                f"{location}: {relative}: expected {bundle_id}, got {actual}")

    host = info["Info.plist"]
    watch = info["Watch/watchkit.app/Info.plist"]
    extension = info["Watch/watchkit.app/PlugIns/watchkit Extension.appex/Info.plist"]
    require(watch.get("WKCompanionAppBundleIdentifier") == host["CFBundleIdentifier"],
            f"{location}: Watch companion bundle ID mismatch")
    require(extension.get("NSExtension", {}).get("NSExtensionAttributes", {}).get(
        "WKAppBundleIdentifier") == watch["CFBundleIdentifier"],
        f"{location}: WatchKit extension/app bundle ID mismatch")
    versions = {entry.get("CFBundleVersion") for entry in info.values()}
    require(None not in versions and len(versions) == 1,
            f"{location}: targets have inconsistent build numbers: {versions}")
    print(f"{location}: all 4 bundles present, correctly associated, build {versions.pop()}")


def verify_archive(path):
    app = Path(path) / "Products/Applications/qdomyoszwift.app"
    verify_bundles(lambda rel: (app / rel).read_bytes(), str(path))


def verify_ipa(path):
    with zipfile.ZipFile(path) as ipa:
        verify_bundles(
            lambda rel: ipa.read("Payload/qdomyoszwift.app/" + rel),
            str(path),
        )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", help="Generated Xcode project.pbxproj")
    parser.add_argument("--archive", help="Signed .xcarchive")
    parser.add_argument("--ipa", help="Exported .ipa")
    args = parser.parse_args()
    if not (args.project or args.archive or args.ipa):
        parser.error("Provide --project, --archive and/or --ipa")
    try:
        if args.project:
            verify_project(args.project)
        if args.archive:
            verify_archive(args.archive)
        if args.ipa:
            verify_ipa(args.ipa)
    except (ValueError, OSError, zipfile.BadZipFile) as exc:
        print(f"iOS distribution verification failed: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
