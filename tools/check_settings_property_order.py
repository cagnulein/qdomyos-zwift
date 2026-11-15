#!/usr/bin/env python3

import re
import subprocess
import sys
from pathlib import Path

SETTINGS_PATH = Path("src/settings.qml")
PROPERTY_RE = re.compile(r"^\s*property\s+\S+\s+([A-Za-z_][A-Za-z0-9_]*)\s*:")


def read_base_file(base_ref: str) -> str:
    try:
        return subprocess.check_output(
            ["git", "show", f"{base_ref}:{SETTINGS_PATH.as_posix()}"],
            text=True,
        )
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(
            f"Unable to read {SETTINGS_PATH} from base ref {base_ref}"
        ) from exc


def strip_strings_and_line_comments(line: str) -> str:
    out = []
    quote = None
    escaped = False
    i = 0

    while i < len(line):
        ch = line[i]

        if quote is not None:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = None
            out.append(" ")
            i += 1
            continue

        if ch in ('"', "'"):
            quote = ch
            out.append(" ")
            i += 1
            continue

        if ch == "/" and i + 1 < len(line) and line[i + 1] == "/":
            break

        out.append(ch)
        i += 1

    return "".join(out)


def settings_properties(text: str) -> list[str]:
    lines = text.splitlines()
    settings_start = None

    for i, line in enumerate(lines):
        if re.match(r"^\s*Settings\s*\{\s*$", line):
            lookahead = "\n".join(lines[i + 1 : i + 8])
            if re.search(r"^\s*id\s*:\s*settings\s*$", lookahead, re.MULTILINE):
                settings_start = i
                break

    if settings_start is None:
        raise RuntimeError("Could not find the persistent Settings { id: settings } block")

    depth = 0
    properties = []

    for line in lines[settings_start:]:
        clean = strip_strings_and_line_comments(line)

        # Direct children of the Settings block are at depth 1 before this line.
        if depth == 1:
            match = PROPERTY_RE.match(line)
            if match:
                properties.append(match.group(1))

        depth += clean.count("{")
        depth -= clean.count("}")

        if depth == 0 and properties:
            break

    if not properties:
        raise RuntimeError("No persistent settings properties found")

    return properties


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <base-git-ref>", file=sys.stderr)
        return 2

    base_ref = sys.argv[1]
    current_text = SETTINGS_PATH.read_text(encoding="utf-8")
    base_text = read_base_file(base_ref)

    base_properties = settings_properties(base_text)
    current_properties = settings_properties(current_text)

    prefix = current_properties[: len(base_properties)]
    if prefix == base_properties:
        added = current_properties[len(base_properties) :]
        if added:
            print("OK: new settings properties were appended at the end:")
            for name in added:
                print(f"  + {name}")
        else:
            print("OK: persistent settings property order is unchanged")
        return 0

    mismatch = next(
        (
            i
            for i, (old, new) in enumerate(zip(base_properties, prefix))
            if old != new
        ),
        min(len(base_properties), len(prefix)),
    )

    expected = base_properties[mismatch] if mismatch < len(base_properties) else "<end>"
    actual = current_properties[mismatch] if mismatch < len(current_properties) else "<missing>"

    print(
        "ERROR: persistent settings properties in src/settings.qml must never be "
        "inserted, removed, renamed, or reordered. New properties must be appended "
        "after all existing properties.",
        file=sys.stderr,
    )
    print(
        f"First mismatch at property #{mismatch + 1}: expected '{expected}', got '{actual}'.",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(2)
