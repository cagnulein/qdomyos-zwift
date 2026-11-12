#!/usr/bin/env python3
"""Compile the real native treadmill bridge and test its opt-in FTMS queue on macOS."""

from pathlib import Path
import subprocess
import tempfile


def main():
    repository = Path(__file__).resolve().parents[1]
    ios = repository / "src" / "ios"
    # These globals normally come from the other Swift files in the iOS target.
    uuid_declarations = []
    for filename in ("BLEPeripheralManager.swift", "virtualbike_zwift.swift"):
        for line in (ios / filename).read_text().splitlines():
            if line.startswith("let ") and "CBUUID(string:" in line:
                uuid_declarations.append(line)

    with tempfile.TemporaryDirectory(prefix="qz-ypoo-ios-") as directory:
        temporary = Path(directory)
        support = temporary / "Support.swift"
        support.write_text(
            "import CoreBluetooth\n"
            + "\n".join(uuid_declarations)
            + "\nclass swiftDebug { func qtDebug(_ message: String) {} }\n"
        )
        executable = temporary / "ypoo-ios-tests"
        subprocess.run(
            [
                "xcrun", "swiftc", "-swift-version", "5",
                "-module-cache-path", str(temporary / "modules"),
                str(support), str(ios / "virtualtreadmill_zwift.swift"),
                str(repository / "tst" / "ios" / "TestYpooM5ControlPointQueue.swift"),
                "-o", str(executable),
            ],
            check=True,
        )
        subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    main()
