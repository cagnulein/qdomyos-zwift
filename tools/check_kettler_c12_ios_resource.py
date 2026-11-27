#!/usr/bin/env python3
"""Check that the committed iOS app target links the C12 Qt resource."""
import re
import sys
from pathlib import Path


def verify(project):
    def section(name):
        return project.split(f"/* Begin {name} section */", 1)[1].split(
            f"/* End {name} section */", 1)[0]

    def require(condition, message):
        if not condition:
            raise ValueError(message)

    name = "qrc_kettlerc12_resources.cpp"
    ref = re.search(r"([A-F0-9]{24}) /\* " + re.escape(name) +
                    r" \*/ = \{isa = PBXFileReference;[^}]*path = " +
                    re.escape(name) + r";", section("PBXFileReference"))
    require(ref, "Missing C12 resource file reference")
    build = re.search(r"([A-F0-9]{24}) /\* " + re.escape(name) +
                      r" in Compile Sources \*/ = \{isa = PBXBuildFile; fileRef = " +
                      ref.group(1), section("PBXBuildFile"))
    require(build, "Missing C12 resource build file")
    app = re.search(r"/\* qdomyoszwift \*/ = \{(.*?)\n\s*\};",
                    section("PBXNativeTarget"), re.S)
    require(app, "Missing QZ app target")
    phases = re.findall(r"([A-F0-9]{24}) /\* (?:Compile )?Sources \*/", app.group(1))
    require(phases, "Missing QZ app Sources phase")
    sources = section("PBXSourcesBuildPhase")
    require(any(re.search(
        re.escape(phase) + r" /\* (?:Compile )?Sources \*/ = \{[^}]*" +
        re.escape(build.group(1)) + r" /\* " + re.escape(name) +
        r" in Compile Sources \*/", sources) for phase in phases),
        "C12 resource is not in the QZ app Sources phase")


if __name__ == "__main__":
    verify(Path(sys.argv[1]).read_text(encoding="utf-8"))
    print("OK: C12 resource linked by the iOS app target")
