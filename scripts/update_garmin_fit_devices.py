#!/usr/bin/env python3
"""Add new FIT Garmin products to the device selector in src/settings.qml.

The catalog is Garmin's generated ``fit_profile.hpp`` from the official FIT SDK.
Only newly assigned products in device families already represented by QZ are
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
MIN_CATALOG_SIZE = 300
FAMILIES = ("D2", "EDGE", "EPIX", "FENIX", "FR", "VENU", "VIVOACTIVE")
REGIONAL_SUFFIXES = ("APAC", "ASIA", "CHINA", "JAPAN", "KOREA", "RUSSIA", "SEA", "TAIWAN", "TWN")
NON_DEVICE_PRODUCTS = {"EDGE_REMOTE"}
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
    starts = list(re.finditer(r"^\s*ComboBox\s*\{", qml[: anchors[0].start()], re.MULTILINE))
    if not starts:
        raise UpdateError("Garmin id is not inside a ComboBox")
    start = starts[-1].start()
    end = _block_end(qml, qml.index("{", start))
    if not start < anchors[0].start() < end:
        raise UpdateError("Garmin id is not inside the located ComboBox")
    return start, end


def parse_current(control: str) -> list[Device]:
    model_matches = list(re.finditer(r"(?ms)^(\s*)model:\s*\[\n(.*?)^\1\]", control))
    if len(model_matches) != 1:
        raise UpdateError(f"expected one model in Garmin ComboBox, found {len(model_matches)}")
    names = re.findall(r'^\s*"([^"\\]+)",?\s*$', model_matches[0].group(2), re.MULTILINE)
    nonblank = [line for line in model_matches[0].group(2).splitlines() if line.strip()]
    if len(names) != len(nonblank) or len(names) < 2:
        raise UpdateError("Garmin model contains unexpected QML syntax")
    index_pairs = re.findall(
        r"settings\.fit_file_garmin_device_training_effect_device === (\d+)\) return (\d+);\s*//\s*([A-Za-z0-9_]+)\s*$",
        control,
        re.MULTILINE,
    )
    case_pairs = re.findall(
        r"case (\d+): settings\.fit_file_garmin_device_training_effect_device = (\d+); break;\s*//\s*([A-Za-z0-9_]+)\s*$",
        control,
        re.MULTILINE,
    )
    if len(index_pairs) != len(names) or len(case_pairs) != len(names):
        raise UpdateError("Garmin model and index mappings have different lengths")
    devices = []
    for index, (name, current, case) in enumerate(zip(names, index_pairs, case_pairs)):
        iproduct, iindex, macro = current
        cindex, cproduct, cmacro = case
        if int(iindex) != index or int(cindex) != index or iproduct != cproduct or macro != cmacro:
            raise UpdateError(f"inconsistent Garmin mapping at index {index}")
        devices.append(Device(macro, int(iproduct), name))
    if len({d.macro for d in devices}) != len(devices) or len({d.product for d in devices}) != len(devices):
        raise UpdateError("current Garmin list contains duplicates")
    return devices


def display_name(macro: str) -> str:
    parts = macro.split("_")
    return " ".join(part.capitalize() if not part.isdigit() else part for part in parts)


def eligible(macro: str, product: int, highest_existing: int) -> bool:
    return product > highest_existing and macro.startswith(FAMILIES) and not macro.endswith(REGIONAL_SUFFIXES)


def update_qml(qml: str, catalog: dict[str, int]) -> tuple[str, list[Device], list[Device]]:
    start, end = locate_control(qml)
    control = qml[start:end]
    current = parse_current(control)
    real = [d for d in current if d.macro not in ("Tacx", "Zwift")]
    special = [d for d in current if d.macro in ("Tacx", "Zwift")]
    # EDGE_REMOTE is a legacy accessory whose FIT product number (10014) is far
    # above current watches and cycling computers. It must not become the
    # chronological cutoff or every normal product below 10014 is suppressed.
    highest = max(d.product for d in real if d.macro not in NON_DEVICE_PRODUCTS)
    known = {d.macro for d in current}
    added = [Device(m, p, display_name(m)) for m, p in catalog.items() if m not in known and eligible(m, p, highest)]
    added.sort(key=lambda d: (d.product, d.macro))
    if not added:
        return qml, [], current
    devices = real + added + special
    indent = re.search(r"(?m)^(\s*)model:", control).group(1)
    item_indent = indent + "    "
    model_body = "\n".join(f'{item_indent}"{d.display}"{"," if i < len(devices)-1 else ""}' for i, d in enumerate(devices))
    control, count = re.subn(r"(?ms)^(\s*model:\s*\[)\n.*?^(\s*\])", lambda m: m.group(1)+"\n"+model_body+"\n"+m.group(2), control, count=1)
    if count != 1:
        raise UpdateError("failed to replace Garmin model")
    current_lines = "\n".join(
        f"{item_indent}if (settings.fit_file_garmin_device_training_effect_device === {d.product}) return {i};  // {d.macro}"
        for i, d in enumerate(devices)
    )
    control, count = re.subn(
        r"(?ms)(\s*currentIndex:\s*\{\n).*?(\s*return \d+;\s*// Default to Edge 830)",
        lambda m: m.group(1)+current_lines+"\n"+m.group(2), control, count=1,
    )
    if count != 1:
        raise UpdateError("failed to replace Garmin currentIndex mapping")
    case_lines = "\n".join(
        f"{item_indent}case {i}: settings.fit_file_garmin_device_training_effect_device = {d.product}; break;  // {d.macro}"
        for i, d in enumerate(devices)
    )
    control, count = re.subn(
        r"(?ms)(\s*onCurrentIndexChanged:\s*\{\n\s*switch\(currentIndex\)\s*\{\n).*?(\s*\}\n\s*\})",
        lambda m: m.group(1)+case_lines+"\n"+m.group(2), control, count=1,
    )
    if count != 1:
        raise UpdateError("failed to replace Garmin switch mapping")
    updated = qml[:start] + control + qml[end:]
    if updated[:start] != qml[:start] or updated[start + len(control):] != qml[end:]:
        raise UpdateError("content outside Garmin ComboBox changed")
    parse_current(control)
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
        highest = max(d.product for d in real if d.macro not in NON_DEVICE_PRODUCTS)
        compatible = sum(
            1
            for macro, product in catalog.items()
            if macro in {d.macro for d in current} or eligible(macro, product, highest)
        )
        lines = [
            "Garmin FIT product catalog update.", "", f"Current QZ devices: {len(current)}",
            f"Products obtained from Garmin: {len(catalog)}", f"Compatible upstream products: {compatible}", f"New compatible products: {len(added)}", "",
            "Devices added:", *([f"- {d.macro} ({d.product}) — {d.display}" for d in added] or ["- None"]), "",
            "Existing QZ entries absent upstream (preserved):", *([f"- {d.macro} ({d.product})" for d in absent] or ["- None"]), "",
            "Existing QZ Garmin products were preserved; no products are automatically removed.",
            "Source: Garmin's official FIT C++ SDK generated fit_profile.hpp.",
            "This PR was generated by the weekly Garmin FIT product synchronization workflow.",
        ]
        summary = "\n".join(lines) + "\n"
        if args.summary:
            Path(args.summary).write_text(summary, encoding="utf-8")
        print(summary, end="")
        return 0
    except (OSError, UnicodeError, UpdateError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
