#!/usr/bin/env python3
"""Complete settings-catalog.json coverage for the Phase 2 settings refactor.

This is intentionally a one-shot, deterministic migration. It refuses to run if the
set of currently uncataloged persistent keys differs from the baseline discovered
in Phase 1, so it cannot silently paper over future catalog regressions.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Dict

import settings_audit

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / settings_audit.CATALOG_PATH

INTERNAL_KEYS = {
    "garmin_expires_at",
    "garmin_refresh_token_expires_at",
    "service_changed",
}

METADATA: Dict[str, Dict[str, Any]] = {
    "ant_garmin": {"name": "Garmin ANT+", "parent": "Garmin Options"},
    "garmin_expires_at": {"name": "Garmin Access Token Expiry", "parent": "Garmin Options"},
    "garmin_refresh_token_expires_at": {"name": "Garmin Refresh Token Expiry", "parent": "Garmin Options"},
    "peloton_bike_ocr": {"name": "Peloton Bike OCR", "parent": "Peloton Options"},
    "service_changed": {"name": "Bluetooth Service Changed State", "parent": "Advanced Settings"},
    "watt_bike_emulator": {"name": "Wattbike Emulator", "parent": "Bike Options"},
    "mywhoosh_link_enabled": {"name": "Enable OpenBikeControl", "parent": "OpenBikeControl"},
    "mywhoosh_link_override_gears": {"name": "OpenBikeControl Override Gears", "parent": "OpenBikeControl"},
    "mywhoosh_link_left_up": {"name": "Left Controller Up", "parent": "OpenBikeControl"},
    "mywhoosh_link_left_down": {"name": "Left Controller Down", "parent": "OpenBikeControl"},
    "mywhoosh_link_left_left": {"name": "Left Controller Left", "parent": "OpenBikeControl"},
    "mywhoosh_link_left_right": {"name": "Left Controller Right", "parent": "OpenBikeControl"},
    "mywhoosh_link_left_power": {"name": "Left Controller Power", "parent": "OpenBikeControl"},
    "mywhoosh_link_left_shoulder": {"name": "Left Controller Shoulder", "parent": "OpenBikeControl"},
    "mywhoosh_link_right_a": {"name": "Right Controller A", "parent": "OpenBikeControl"},
    "mywhoosh_link_right_b": {"name": "Right Controller B", "parent": "OpenBikeControl"},
    "mywhoosh_link_right_y": {"name": "Right Controller Y", "parent": "OpenBikeControl"},
    "mywhoosh_link_right_z": {"name": "Right Controller Z", "parent": "OpenBikeControl"},
    "mywhoosh_link_right_power": {"name": "Right Controller Power", "parent": "OpenBikeControl"},
    "mywhoosh_link_right_shoulder": {"name": "Right Controller Shoulder", "parent": "OpenBikeControl"},
    "mywhoosh_link_camera_value": {"name": "Camera Action", "parent": "OpenBikeControl"},
    "mywhoosh_link_emote_value": {"name": "Emote Action", "parent": "OpenBikeControl"},
    "zwiftplay_gear_paddle_left": {"name": "Left Paddle Gear Action", "parent": "Zwift Play Options"},
    "zwiftplay_gear_paddle_right": {"name": "Right Paddle Gear Action", "parent": "Zwift Play Options"},
    "zwiftplay_gear_lb": {"name": "Left Button Gear Action", "parent": "Zwift Play Options"},
    "zwiftplay_gear_rb": {"name": "Right Button Gear Action", "parent": "Zwift Play Options"},
    "zwiftplay_gear_ls1": {"name": "Left Shift 1 Gear Action", "parent": "Zwift Play Options"},
    "zwiftplay_gear_ls2": {"name": "Left Shift 2 Gear Action", "parent": "Zwift Play Options"},
    "zwiftplay_gear_rs1": {"name": "Right Shift 1 Gear Action", "parent": "Zwift Play Options"},
    "zwiftplay_gear_rs2": {"name": "Right Shift 2 Gear Action", "parent": "Zwift Play Options"},
}

EXPECTED_MISSING = set(METADATA)


def parse_default(qml_type: str, expression: str) -> Any:
    expr = expression.strip()
    if qml_type == "bool":
        if expr == "true":
            return True
        if expr == "false":
            return False
    if qml_type == "int":
        return int(float(expr))
    if qml_type == "real":
        return float(expr)
    if qml_type == "string" and len(expr) >= 2 and expr[0] == expr[-1] == '"':
        return bytes(expr[1:-1], "utf-8").decode("unicode_escape")
    return None


def catalog_type(qml_type: str) -> str:
    return {
        "bool": "boolean",
        "int": "integer",
        "real": "number",
        "string": "string",
    }.get(qml_type, "string")


def main() -> int:
    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
    declarations = settings_audit.flatten(settings_audit.declarations_from(ROOT))
    unique = {}
    for decl in declarations:
        unique.setdefault(decl.key, decl)

    existing = {entry["key"] for entry in catalog.get("settings", [])}
    missing = set(unique) - existing
    if missing != EXPECTED_MISSING:
        raise SystemExit(
            "Refusing Phase 2 migration because the uncataloged baseline changed.\n"
            f"Expected: {sorted(EXPECTED_MISSING)}\n"
            f"Actual:   {sorted(missing)}"
        )

    for key in sorted(missing):
        decl = unique[key]
        meta = METADATA[key]
        qml_type = decl.qml_type
        entry = {
            "key": key,
            "name": meta["name"],
            "description": None,
            "parent": meta["parent"],
            "type": catalog_type(qml_type),
            "qmlType": qml_type,
            "control": "switch" if qml_type == "bool" else "text",
            "visible": key not in INTERNAL_KEYS,
            "defaultValue": parse_default(qml_type, decl.default_expression),
            "defaultExpression": settings_audit.normalize_expression(decl.default_expression),
            "options": None,
        }
        catalog["settings"].append(entry)

    catalog["settingCount"] = len({entry["key"] for entry in catalog["settings"]})
    if catalog["settingCount"] != len(unique):
        raise SystemExit(
            f"Catalog migration did not reach full coverage: {catalog['settingCount']}/{len(unique)}"
        )

    CATALOG.write_text(json.dumps(catalog, indent=2, ensure_ascii=True) + "\n", encoding="utf-8")
    print(f"Catalog migrated to {catalog['settingCount']}/{len(unique)} persistent keys")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
