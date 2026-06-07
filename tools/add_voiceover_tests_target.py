#!/usr/bin/env python3
"""
Adds the QZVoiceOverTests UI test target to the qdomyoszwift Xcode project.

Run from the repo root:
    python3 tools/add_voiceover_tests_target.py

Idempotent: re-running after a successful patch is a no-op.
"""

import re
import sys
import os

PBXPROJ = os.path.join(
    os.path.dirname(__file__),
    "..",
    "build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug",
    "qdomyoszwift.xcodeproj",
    "project.pbxproj",
)
PBXPROJ = os.path.normpath(PBXPROJ)

# Fixed UUIDs for every new object we add (deterministic so the script is idempotent).
U = {
    "buildfile_swift":   "AA000001000000000000000A",  # PBXBuildFile – swift source
    "fileref_swift":     "AA000002000000000000000A",  # PBXFileReference – .swift
    "fileref_product":   "AA000003000000000000000A",  # PBXFileReference – .xctest product
    "frameworks_phase":  "AA000004000000000000000A",  # PBXFrameworksBuildPhase
    "group_tests":       "AA000005000000000000000A",  # PBXGroup – QZVoiceOverTests/
    "native_target":     "AA000006000000000000000A",  # PBXNativeTarget
    "sources_phase":     "AA000007000000000000000A",  # PBXSourcesBuildPhase
    "cfg_debug":         "AA000008000000000000000A",  # XCBuildConfiguration Debug (target)
    "cfg_release":       "AA000009000000000000000A",  # XCBuildConfiguration Release (target)
    "cfglist":           "AA00000A000000000000000A",  # XCConfigurationList (target)
}

# Main app target UUID (the "host" for UITESTINGHOSTBUNDLEID).
APP_TARGET_UUID = "799833E5566DEFFC37E4BF1E"
# Main group UUID.
MAIN_GROUP_UUID = "E8C543AB96796ECAA2E65C57"
# Products group UUID.
PRODUCTS_GROUP_UUID = "FE0A091FDBFB3E9C31B7A1BD"

GUARD_STRING = U["native_target"]  # presence means already patched


def patch(text: str) -> str:
    if GUARD_STRING in text:
        print("project.pbxproj already contains the QZVoiceOverTests target – nothing to do.")
        return text

    # ------------------------------------------------------------------
    # 1. PBXBuildFile
    # ------------------------------------------------------------------
    build_file_entry = (
        f"\t\t{U['buildfile_swift']} /* QZVoiceOverTests.swift in Sources */ = "
        f"{{isa = PBXBuildFile; fileRef = {U['fileref_swift']} /* QZVoiceOverTests.swift */; }};\n"
    )
    text = text.replace(
        "/* End PBXBuildFile section */",
        build_file_entry + "/* End PBXBuildFile section */"
    )

    # ------------------------------------------------------------------
    # 2. PBXFileReference – source file + product
    # ------------------------------------------------------------------
    fileref_entries = (
        f"\t\t{U['fileref_swift']} /* QZVoiceOverTests.swift */ = "
        f"{{isa = PBXFileReference; lastKnownFileType = sourcecode.swift; "
        f"path = QZVoiceOverTests.swift; sourceTree = \"<group>\"; }};\n"
        f"\t\t{U['fileref_product']} /* QZVoiceOverTests.xctest */ = "
        f"{{isa = PBXFileReference; explicitFileType = wrapper.cfbundle; "
        f"includeInIndex = 0; path = QZVoiceOverTests.xctest; sourceTree = BUILT_PRODUCTS_DIR; }};\n"
    )
    text = text.replace(
        "/* End PBXFileReference section */",
        fileref_entries + "/* End PBXFileReference section */"
    )

    # ------------------------------------------------------------------
    # 3. PBXFrameworksBuildPhase (empty – UI tests don't need explicit frameworks)
    # ------------------------------------------------------------------
    frameworks_entry = (
        f"\t\t{U['frameworks_phase']} /* Frameworks */ = {{\n"
        f"\t\t\tisa = PBXFrameworksBuildPhase;\n"
        f"\t\t\tbuildActionMask = 2147483647;\n"
        f"\t\t\tfiles = (\n"
        f"\t\t\t);\n"
        f"\t\t\trunOnlyForDeploymentPostprocessing = 0;\n"
        f"\t\t}};\n"
    )
    text = text.replace(
        "/* End PBXFrameworksBuildPhase section */",
        frameworks_entry + "/* End PBXFrameworksBuildPhase section */"
    )

    # ------------------------------------------------------------------
    # 4. PBXGroup for QZVoiceOverTests/
    # ------------------------------------------------------------------
    group_entry = (
        f"\t\t{U['group_tests']} /* QZVoiceOverTests */ = {{\n"
        f"\t\t\tisa = PBXGroup;\n"
        f"\t\t\tchildren = (\n"
        f"\t\t\t\t{U['fileref_swift']} /* QZVoiceOverTests.swift */,\n"
        f"\t\t\t);\n"
        f"\t\t\tpath = QZVoiceOverTests;\n"
        f"\t\t\tsourceTree = \"<group>\";\n"
        f"\t\t}};\n"
    )
    text = text.replace(
        "/* End PBXGroup section */",
        group_entry + "/* End PBXGroup section */"
    )

    # Add the test group to the main group's children list.
    text = text.replace(
        f"\t\t\t\t{PRODUCTS_GROUP_UUID} /* Products */,\n"
        f"\t\t\t\t8798FDCA2D6F338200CF8EE8 /* Recovered References */,",
        f"\t\t\t\t{U['group_tests']} /* QZVoiceOverTests */,\n"
        f"\t\t\t\t{PRODUCTS_GROUP_UUID} /* Products */,\n"
        f"\t\t\t\t8798FDCA2D6F338200CF8EE8 /* Recovered References */,"
    )

    # Add product to Products group children.
    text = text.replace(
        f"\t\t\t\t87EFC5642E918D35005BB573 /* QZWidgetExtension.appex */,\n"
        f"\t\t\t);\n"
        f"\t\t\tname = Products;",
        f"\t\t\t\t87EFC5642E918D35005BB573 /* QZWidgetExtension.appex */,\n"
        f"\t\t\t\t{U['fileref_product']} /* QZVoiceOverTests.xctest */,\n"
        f"\t\t\t);\n"
        f"\t\t\tname = Products;"
    )

    # ------------------------------------------------------------------
    # 5. PBXNativeTarget
    # ------------------------------------------------------------------
    native_target_entry = (
        f"\t\t{U['native_target']} /* QZVoiceOverTests */ = {{\n"
        f"\t\t\tisa = PBXNativeTarget;\n"
        f"\t\t\tbuildConfigurationList = {U['cfglist']} /* Build configuration list for PBXNativeTarget \"QZVoiceOverTests\" */;\n"
        f"\t\t\tbuildPhases = (\n"
        f"\t\t\t\t{U['sources_phase']} /* Sources */,\n"
        f"\t\t\t\t{U['frameworks_phase']} /* Frameworks */,\n"
        f"\t\t\t);\n"
        f"\t\t\tbuildRules = (\n"
        f"\t\t\t);\n"
        f"\t\t\tdependencies = (\n"
        f"\t\t\t);\n"
        f"\t\t\tname = QZVoiceOverTests;\n"
        f"\t\t\tproductName = QZVoiceOverTests;\n"
        f"\t\t\tproductReference = {U['fileref_product']} /* QZVoiceOverTests.xctest */;\n"
        f"\t\t\tproductType = \"com.apple.product-type.bundle.ui-testing\";\n"
        f"\t\t}};\n"
    )
    text = text.replace(
        "/* End PBXNativeTarget section */",
        native_target_entry + "/* End PBXNativeTarget section */"
    )

    # ------------------------------------------------------------------
    # 6. PBXSourcesBuildPhase
    # ------------------------------------------------------------------
    sources_phase_entry = (
        f"\t\t{U['sources_phase']} /* Sources */ = {{\n"
        f"\t\t\tisa = PBXSourcesBuildPhase;\n"
        f"\t\t\tbuildActionMask = 2147483647;\n"
        f"\t\t\tfiles = (\n"
        f"\t\t\t\t{U['buildfile_swift']} /* QZVoiceOverTests.swift in Sources */,\n"
        f"\t\t\t);\n"
        f"\t\t\trunOnlyForDeploymentPostprocessing = 0;\n"
        f"\t\t}};\n"
    )
    text = text.replace(
        "/* End PBXSourcesBuildPhase section */",
        sources_phase_entry + "/* End PBXSourcesBuildPhase section */"
    )

    # ------------------------------------------------------------------
    # 7. XCBuildConfiguration (Debug + Release for the UI test target)
    # ------------------------------------------------------------------
    cfg_debug = (
        f"\t\t{U['cfg_debug']} /* Debug */ = {{\n"
        f"\t\t\tisa = XCBuildConfiguration;\n"
        f"\t\t\tbuildSettings = {{\n"
        f"\t\t\t\tALWAYS_EMBED_SWIFT_STANDARD_LIBRARIES = YES;\n"
        f"\t\t\t\tCODE_SIGN_STYLE = Automatic;\n"
        f"\t\t\t\tDEVELOPMENT_TEAM = 6335M7T29D;\n"
        f"\t\t\t\tGENERATE_INFOPLIST_FILE = YES;\n"
        f"\t\t\t\tIPHONEOS_DEPLOYMENT_TARGET = 12.0;\n"
        f"\t\t\t\tPRODUCT_BUNDLE_IDENTIFIER = \"org.cagnulein.qdomyoszwift.QZVoiceOverTests\";\n"
        f"\t\t\t\tPRODUCT_NAME = \"$(TARGET_NAME)\";\n"
        f"\t\t\t\tSDKROOT = iphoneos;\n"
        f"\t\t\t\tSWIFT_VERSION = 5.0;\n"
        f"\t\t\t\tTARGETED_DEVICE_FAMILY = \"1,2\";\n"
        f"\t\t\t\tTEST_TARGET_NAME = qdomyoszwift;\n"
        f"\t\t\t\tUITESTINGHOSTBUNDLEID = \"org.cagnulein.qdomyoszwift\";\n"
        f"\t\t\t}};\n"
        f"\t\t\tname = Debug;\n"
        f"\t\t}};\n"
    )
    cfg_release = (
        f"\t\t{U['cfg_release']} /* Release */ = {{\n"
        f"\t\t\tisa = XCBuildConfiguration;\n"
        f"\t\t\tbuildSettings = {{\n"
        f"\t\t\t\tALWAYS_EMBED_SWIFT_STANDARD_LIBRARIES = YES;\n"
        f"\t\t\t\tCODE_SIGN_STYLE = Automatic;\n"
        f"\t\t\t\tDEVELOPMENT_TEAM = 6335M7T29D;\n"
        f"\t\t\t\tGENERATE_INFOPLIST_FILE = YES;\n"
        f"\t\t\t\tIPHONEOS_DEPLOYMENT_TARGET = 12.0;\n"
        f"\t\t\t\tPRODUCT_BUNDLE_IDENTIFIER = \"org.cagnulein.qdomyoszwift.QZVoiceOverTests\";\n"
        f"\t\t\t\tPRODUCT_NAME = \"$(TARGET_NAME)\";\n"
        f"\t\t\t\tSDKROOT = iphoneos;\n"
        f"\t\t\t\tSWIFT_VERSION = 5.0;\n"
        f"\t\t\t\tTARGETED_DEVICE_FAMILY = \"1,2\";\n"
        f"\t\t\t\tTEST_TARGET_NAME = qdomyoszwift;\n"
        f"\t\t\t\tUITESTINGHOSTBUNDLEID = \"org.cagnulein.qdomyoszwift\";\n"
        f"\t\t\t}};\n"
        f"\t\t\tname = Release;\n"
        f"\t\t}};\n"
    )
    text = text.replace(
        "/* End XCBuildConfiguration section */",
        cfg_debug + cfg_release + "/* End XCBuildConfiguration section */"
    )

    # ------------------------------------------------------------------
    # 8. XCConfigurationList for the target
    # ------------------------------------------------------------------
    cfglist_entry = (
        f"\t\t{U['cfglist']} /* Build configuration list for PBXNativeTarget \"QZVoiceOverTests\" */ = {{\n"
        f"\t\t\tisa = XCConfigurationList;\n"
        f"\t\t\tbuildConfigurations = (\n"
        f"\t\t\t\t{U['cfg_debug']} /* Debug */,\n"
        f"\t\t\t\t{U['cfg_release']} /* Release */,\n"
        f"\t\t\t);\n"
        f"\t\t\tdefaultConfigurationIsVisible = 0;\n"
        f"\t\t\tdefaultConfigurationName = Debug;\n"
        f"\t\t}};\n"
    )
    text = text.replace(
        "/* End XCConfigurationList section */",
        cfglist_entry + "/* End XCConfigurationList section */"
    )

    # ------------------------------------------------------------------
    # 9. PBXProject – add to targets list + TargetAttributes
    # ------------------------------------------------------------------
    text = text.replace(
        f"\t\t\t\t87EFC5632E918D35005BB573 /* QZWidgetExtension */,\n"
        f"\t\t\t);\n"
        f"\t\t}};\n"
        f"/* End PBXProject section */",
        f"\t\t\t\t87EFC5632E918D35005BB573 /* QZWidgetExtension */,\n"
        f"\t\t\t\t{U['native_target']} /* QZVoiceOverTests */,\n"
        f"\t\t\t);\n"
        f"\t\t}};\n"
        f"/* End PBXProject section */"
    )

    # Add TargetAttributes for the new target.
    text = text.replace(
        f"\t\t\t\t87EFC5632E918D35005BB573 = {{\n"
        f"\t\t\t\t\tCreatedOnToolsVersion = 26.0.1;\n"
        f"\t\t\t\t}};",
        f"\t\t\t\t87EFC5632E918D35005BB573 = {{\n"
        f"\t\t\t\t\tCreatedOnToolsVersion = 26.0.1;\n"
        f"\t\t\t\t}};\n"
        f"\t\t\t\t{U['native_target']} = {{\n"
        f"\t\t\t\t\tCreatedOnToolsVersion = 15.4;\n"
        f"\t\t\t\t\tDevelopmentTeam = 6335M7T29D;\n"
        f"\t\t\t\t\tTestTargetID = {APP_TARGET_UUID};\n"
        f"\t\t\t\t}};"
    )

    return text


def main():
    if not os.path.exists(PBXPROJ):
        print(f"ERROR: project.pbxproj not found at {PBXPROJ}", file=sys.stderr)
        sys.exit(1)

    with open(PBXPROJ, "r", encoding="utf-8") as f:
        original = f.read()

    patched = patch(original)

    if patched == original:
        # Already patched or replacement targets not found.
        return

    # Back up before writing.
    backup = PBXPROJ + ".before_voiceover_tests"
    if not os.path.exists(backup):
        with open(backup, "w", encoding="utf-8") as f:
            f.write(original)
        print(f"Backup written to {backup}")

    with open(PBXPROJ, "w", encoding="utf-8") as f:
        f.write(patched)

    print("project.pbxproj patched successfully.")
    print(f"QZVoiceOverTests target UUID: {U['native_target']}")
    print()
    print("Next steps:")
    print("  1. Open qdomyoszwift.xcodeproj in Xcode (it will reload automatically).")
    print("  2. Select the QZVoiceOverTests scheme from the scheme picker.")
    print("  3. Choose your device / simulator, then Product > Test (⌘U).")


if __name__ == "__main__":
    main()
