#!/usr/bin/env python3
"""Add new FIT Garmin products to the value-role device selector in src/settings.qml.

The catalog is Garmin's generated ``fit_profile.hpp`` from the official FIT SDK.
Products missing from QZ in device families already represented by QZ are
eligible. Existing entries are never removed, including products that disappear
upstream. Regional variants are intentionally ignored unless already maintained
by QZ; they identify the same hardware and would make the selector unwieldy.
"""

from __future__ import annotations

import argparse
import re
import sys
import urllib.request
from dataclasses import dataclass
from pathlib import Path

DEFAULT_URL = "https://raw.githubusercontent.com/garmin/fit-cpp-sdk/main/src/fit_profile.hpp"
UPDATER_VERSION = "4"
MIN_CATALOG_SIZE = 300
# Garmin's FIT profile has no device-category field, so use explicit prefixes for
# consumer watches and cycling computers that can produce activity FIT files.
# Sensors, scales, cameras, handhelds, apps, and trainers are intentionally absent.
FAMILIES = (
    "APPROACH", "D2", "DESCENT", "EDGE", "ENDURO", "EPIX", "FENIX", "FR",
    "INSTINCT", "LEGACY", "LILY", "MARQ", "SWIM", "TACTIX", "VENU",
    "VIVOACTIVE", "VIVO_ACTIVE", "VIVO_MOVE", "VIVOMOVE",
)
EXCLUDED_VARIANT_TOKENS = {
    "APAC", "ASIA", "CHINA", "CHN", "HEBREW", "JAPAN", "JPN", "KOREA", "KOR",
    "RUSSIA", "SEA", "TAIWAN", "THAI", "TWN", "BONTRAGER", "DAIMLER",
}
EXCLUDED_PRODUCTS = {
    "DESCENT_T1", "DESCENT_T2", "EDGE_REMOTE", "FR225_SINGLE_BYTE_PRODUCT_ID",
}
DISPLAY_OVERRIDES = {
    "D2CHARLIE": "D2Charlie",
    "VENUSQ_MUSIC_V2": "Venu Sq Music V2",
}
DEFINE_RE = re.compile(
    r"^#define FIT_GARMIN_PRODUCT_([A-Z0-9_]+)\s+\(\(FIT_GARMIN_PRODUCT\)(\d+)\)", re.MULTILINE
)


class UpdateError(RuntimeError):
    pass


@dataclass(frozen=True)
class Device:
    macro: str
    product: int
    display: str


def parse_catalog(text: str, minimum: int = MIN_CATALOG_SIZE) -> dict[str, int]:
    matches = DEFINE_RE.findall(text)
    if len(matches) < minimum:
        raise UpdateError(f"FIT catalog contains only {len(matches)} products; expected at least {minimum}")
    result: dict[str, int] = {}
    products: dict[int, str] = {}
    for macro, raw_product in matches:
        product = int(raw_product)
        if macro in result:
            raise UpdateError(f"duplicate FIT product name: {macro}")
        if product in products:
            raise UpdateError(f"duplicate FIT product number {product}: {products[product]} and {macro}")
        result[macro] = product
        products[product] = macro
    return result


def _block_end(text: str, open_brace: int) -> int:
    depth = 0
    quote = None
    escaped = False
    for index in range(open_brace, len(text)):
        char = text[index]
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char in "\"'":
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index + 1
    raise UpdateError("unterminated Garmin ComboBox")


def locate_control(qml: str) -> tuple[int, int]:
    anchors = list(re.finditer(r"^\s*id:\s*garminDeviceComboBoxDelegate\s*$", qml, re.MULTILINE))
    if len(anchors) != 1:
        raise UpdateError(f"expected one Garmin ComboBox id, found {len(anchors)}")
    starts = list(re.finditer(r"^\s*(?:Ui)?ComboBox\s*\{", qml[: anchors[0].start()], re.MULTILINE))
    if not starts:
        raise UpdateError("Garmin id is not inside a ComboBox")
    start = starts[-1].start()
    end = _block_end(qml, qml.index("{", start))
    if not start < anchors[0].start() < end:
        raise UpdateError("Garmin id is not inside the located ComboBox")
    return start, end


def parse_current(control: str) -> list[Device]:
    # Each display name and FIT Product ID live together on one QML model line.
    # Reject malformed or duplicate entries instead of silently dropping devices.
    if not re.search(r'^\s*textRole:\s*"name"\s*$', control, re.MULTILINE):
        raise UpdateError("Garmin ComboBox is missing textRole")
    if not re.search(r'^\s*valueRole:\s*"productId"\s*$', control, re.MULTILINE):
        raise UpdateError("Garmin ComboBox is missing valueRole")
    model_matches = list(re.finditer(r"(?ms)^(\s*)model:\s*\[\n(.*?)^\1\]", control))
    if len(model_matches) != 1:
        raise UpdateError(f"expected one model in Garmin ComboBox, found {len(model_matches)}")
    model_lines = [line for line in model_matches[0].group(2).splitlines() if line.strip()]
    line_re = re.compile(
        r'^\s*\{\s*name:\s*"([^"\\]+)",\s*productId:\s*(\d+)\s*\}\s*,?\s*//\s*([A-Za-z0-9_]+)\s*$'
    )
    devices = []
    for line in model_lines:
        match = line_re.fullmatch(line)
        if not match:
            raise UpdateError(f"malformed Garmin model line: {line.strip()}")
        name, raw_id, macro = match.groups()
        devices.append(Device(macro, int(raw_id), name))
    if len(devices) < 2:
        raise UpdateError("Garmin model is unexpectedly small")
    if len({d.macro for d in devices}) != len(devices):
        raise UpdateError("current Garmin list contains duplicate macro names")
    if len({d.product for d in devices}) != len(devices):
        raise UpdateError("current Garmin list contains duplicate FIT Product IDs")
    if len({d.display for d in devices}) != len(devices):
        raise UpdateError("current Garmin list contains duplicate display names")
    return devices


def display_name(macro: str) -> str:
    if macro in DISPLAY_OVERRIDES:
        return DISPLAY_OVERRIDES[macro]
    # FIT uses FR as the product macro prefix, while the user-facing family name
    # is Forerunner. Expand it before the completed list is alphabetically sorted.
    forerunner = re.fullmatch(r"FR(\d+)(?:_(.+))?", macro)
    if forerunner:
        suffix = forerunner.group(2)
        return "Forerunner " + forerunner.group(1) + (
            " " + " ".join(part.capitalize() for part in suffix.split("_")) if suffix else ""
        )
    parts = macro.split("_")
    return " ".join(part.capitalize() if not part.isdigit() else part for part in parts)


def eligible(macro: str) -> bool:
    """Return products matching QZ's existing watch/computer scope.

    FIT product numbers are identifiers, not chronology: older missing products
    and new products may have values below existing accessories. Compatibility
    therefore comes from the established families and structured variant tokens.
    """
    tokens = set(macro.split("_"))
    return (
        macro.startswith(FAMILIES)
        and macro not in EXCLUDED_PRODUCTS
        and tokens.isdisjoint(EXCLUDED_VARIANT_TOKENS)
    )


def update_qml(qml: str, catalog: dict[str, int]) -> tuple[str, list[Device], list[Device]]:
    start, end = locate_control(qml)
    control = qml[start:end]
    current = parse_current(control)
    known = {d.macro for d in current}
    known_ids = {d.product for d in current}
    added = [Device(m, p, display_name(m)) for m, p in catalog.items() if m not in known and eligible(m)]
    if not added:
        return qml, [], current
    for device in added:
        if device.product in known_ids:
            raise UpdateError(f"new Garmin device reuses FIT Product ID {device.product}: {device.macro}")
    devices = sorted(current + added, key=lambda d: (d.display.casefold(), d.macro))
    indent = re.search(r"(?m)^([ \t]*)model:", control).group(1)
    item_indent = indent + "    "
    model_body = "\n".join(
        f'{item_indent}{{ name: "{d.display}", productId: {d.product} }}'
        f'{"," if i < len(devices) - 1 else ""} // {d.macro}'
        for i, d in enumerate(devices)
    )
    control, count = re.subn(
        r"(?ms)^([ \t]*model:\s*\[)\n.*?^([ \t]*\])",
        lambda m: m.group(1) + "\n" + model_body + "\n" + m.group(2),
        control,
        count=1,
    )
    if count != 1:
        raise UpdateError("failed to replace Garmin model")
    updated = qml[:start] + control + qml[end:]
    if updated[:start] != qml[:start] or updated[start + len(control):] != qml[end:]:
        raise UpdateError("content outside Garmin ComboBox changed")
    if parse_current(control) != devices:
        raise UpdateError("generated Garmin model did not round-trip")
    return updated, added, current


def load_catalog(source: str) -> str:
    path = Path(source)
    if path.exists():
        return path.read_text(encoding="utf-8")
    if not source.startswith("https://"):
        raise UpdateError(f"catalog does not exist: {source}")
    try:
        with urllib.request.urlopen(source, timeout=60) as response:
            if response.status != 200:
                raise UpdateError(f"Garmin catalog returned HTTP {response.status}")
            return response.read().decode("utf-8")
    except Exception as error:
        raise UpdateError(f"could not retrieve Garmin FIT catalog: {error}") from error


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalog", default=DEFAULT_URL, help="fit_profile.hpp path or HTTPS URL")
    parser.add_argument("--qml", default="src/settings.qml")
    parser.add_argument("--summary", help="write a Markdown run/PR summary")
    parser.add_argument("--github-output", help="write changed=true/false for GitHub Actions")
    args = parser.parse_args()
    try:
        catalog = parse_catalog(load_catalog(args.catalog))
        path = Path(args.qml)
        original = path.read_text(encoding="utf-8")
        updated, added, current = update_qml(original, catalog)
        if added:
            path.write_text(updated, encoding="utf-8")
        absent = [d for d in current if d.macro not in catalog and d.macro not in ("Tacx", "Zwift")]
        real = [d for d in current if d.macro not in ("Tacx", "Zwift")]
        current_macros = {d.macro for d in current}
        eligible_upstream = {macro for macro in catalog if eligible(macro)}
        present_upstream = eligible_upstream & current_macros
        lines = [
            f"Garmin FIT product catalog update (updater v{UPDATER_VERSION}).", "",
            f"Current QZ selector entries: {len(current)}",
            f"Products obtained from Garmin: {len(catalog)}",
            f"Eligible Garmin products: {len(eligible_upstream)}",
            f"Eligible products already in QZ: {len(present_upstream)}",
            f"Eligible products missing from QZ: {len(eligible_upstream - current_macros)}",
            f"Products added in this run: {len(added)}", "",
            "Devices added:", *([f"- {d.macro} ({d.product}) — {d.display}" for d in added] or ["- None"]), "",
            "Existing QZ entries absent upstream (preserved):", *([f"- {d.macro} ({d.product})" for d in absent] or ["- None"]), "",
            "Existing QZ Garmin products were preserved; no products are automatically removed.",
            "Source: Garmin's official FIT C++ SDK generated fit_profile.hpp.",
            "This PR was generated by the weekly Garmin FIT product synchronization workflow.",
        ]
        summary = "\n".join(lines) + "\n"
        if args.summary:
            Path(args.summary).write_text(summary, encoding="utf-8")
        if args.github_output:
            with Path(args.github_output).open("a", encoding="utf-8") as output:
                output.write(f"changed={'true' if added else 'false'}\n")
        print(summary, end="")
        return 0
    except (OSError, UnicodeError, UpdateError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
