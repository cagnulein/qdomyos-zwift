#!/usr/bin/env python3
"""Generate and validate the QML settings catalog.

The catalog is intentionally JSON so it can be consumed by QML, Android,
iOS, documentation generators, and lightweight test tooling.
"""

from __future__ import annotations

import argparse
import ast
import json
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
QML_SOURCES = [
    Path("src/settings.qml"),
    Path("src/settings-tiles.qml"),
    Path("src/settings-tts.qml"),
    Path("src/settings-shortcuts.qml"),
    Path("src/settings-treadmill-inclination-override.qml"),
]
CATALOG_PATH = Path("src/settings-catalog.json")
SOURCE_DEFAULT_PARENTS = {
    "src/settings-tiles.qml": "Tiles",
    "src/settings-tts.qml": "TTS",
    "src/settings-shortcuts.qml": "Keyboard Shortcuts",
    "src/settings-treadmill-inclination-override.qml": "Treadmill Inclination Overrides",
}

PROPERTY_RE = re.compile(
    r"^\s*property\s+(bool|int|real|double|string)\s+([A-Za-z_][A-Za-z0-9_]*)\s*:\s*(.*?)\s*(?://.*)?$"
)
QSTR_RE = re.compile(r'qsTr\("((?:\\.|[^"\\])*)"\)')
STRING_RE = re.compile(r'"((?:\\.|[^"\\])*)"')
SETTINGS_REF_RE = re.compile(r"\bsettings\.([A-Za-z_][A-Za-z0-9_]*)\b")
SETTING_NAME_RE = re.compile(r'\bsettingName\s*:\s*"([A-Za-z_][A-Za-z0-9_]*)"')
LINKED_BOOL_SETTING_RE = re.compile(r'\blinkedBoolSetting\s*:\s*"([A-Za-z_][A-Za-z0-9_]*)"')
COMBO_ID_RE = re.compile(r"\bid\s*:\s*([A-Za-z_][A-Za-z0-9_]*)")
CASE_TRUE_RE = re.compile(r"\bcase\s+(\d+)\s*:\s*settings\.([A-Za-z_][A-Za-z0-9_]*)\s*=\s*true\s*;")


def read_lines(path: Path) -> list[str]:
    return (REPO_ROOT / path).read_text(encoding="utf-8", errors="replace").splitlines()


def strip_string_literals(line: str) -> str:
    result: list[str] = []
    quote = None
    escaped = False
    for char in line:
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            result.append(" ")
        else:
            if char in ('"', "'"):
                quote = char
                result.append(" ")
            else:
                result.append(char)
    return "".join(result)


def qml_code_for_braces(line: str) -> str:
    return strip_string_literals(re.sub(r"//.*", "", line))


def settings_blocks(lines: list[str]) -> list[tuple[int, int]]:
    blocks: list[tuple[int, int]] = []
    line_index = 0
    while line_index < len(lines):
        if re.search(r"\bSettings\s*\{", lines[line_index]):
            start = line_index
            depth = 0
            seen_open = False
            while line_index < len(lines):
                code = qml_code_for_braces(lines[line_index])
                depth += code.count("{") - code.count("}")
                seen_open = seen_open or "{" in code
                if seen_open and depth == 0:
                    blocks.append((start, line_index))
                    break
                line_index += 1
        line_index += 1
    return blocks


def parse_literal(type_name: str, expression: str) -> Any:
    value = expression.strip()
    if type_name == "bool":
        if value == "true":
            return True
        if value == "false":
            return False
        return None
    if type_name in {"int", "real", "double"}:
        try:
            if value.lower().startswith("0x"):
                return int(value, 16)
            if type_name == "int":
                return int(float(value))
            return float(value)
        except ValueError:
            return None
    if type_name == "string":
        try:
            return ast.literal_eval(value)
        except (SyntaxError, ValueError):
            return None
    return None


def qml_type_to_catalog_type(type_name: str) -> str:
    return {
        "bool": "boolean",
        "int": "integer",
        "real": "number",
        "double": "number",
        "string": "string",
    }[type_name]


def extract_declarations() -> dict[str, list[dict[str, Any]]]:
    declarations: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for source in QML_SOURCES:
        lines = read_lines(source)
        for start, end in settings_blocks(lines):
            for line_no in range(start, end + 1):
                line = lines[line_no]
                match = PROPERTY_RE.match(line)
                if not match or line.lstrip().startswith("//"):
                    continue
                qml_type, key, default_expr = match.groups()
                declarations[key].append(
                    {
                        "file": source.as_posix(),
                        "line": line_no + 1,
                        "qmlType": qml_type,
                        "type": qml_type_to_catalog_type(qml_type),
                        "defaultExpression": default_expr.strip(),
                        "defaultValue": parse_literal(qml_type, default_expr),
                    }
                )
    return dict(declarations)


def decode_qml_string(value: str) -> str:
    try:
        return ast.literal_eval(f'"{value}"')
    except (SyntaxError, ValueError):
        return value


def first_qstr(line: str) -> str | None:
    match = QSTR_RE.search(line)
    if match:
        return decode_qml_string(match.group(1)).strip()
    return None


def clean_title(text: str | None) -> str | None:
    if not text:
        return None
    cleaned = re.sub(r"\s+", " ", text).strip()
    cleaned = cleaned.rstrip(":")
    if not cleaned or cleaned in {"OK", "Setting saved!"}:
        return None
    if len(cleaned) > 90:
        return None
    return cleaned


def clean_description(text: str | None) -> str | None:
    if not text:
        return None
    cleaned = re.sub(r"\s+", " ", text).strip()
    if len(cleaned) < 35:
        return None
    if cleaned in {"Setting saved!"}:
        return None
    return cleaned


def qml_blocks(lines: list[str], opener_pattern: str) -> list[tuple[int, int]]:
    blocks: list[tuple[int, int]] = []
    line_index = 0
    while line_index < len(lines):
        if re.search(opener_pattern, lines[line_index]):
            start = line_index
            depth = 0
            seen_open = False
            while line_index < len(lines):
                code = qml_code_for_braces(lines[line_index])
                depth += code.count("{") - code.count("}")
                seen_open = seen_open or "{" in code
                if seen_open and depth == 0:
                    blocks.append((start, line_index))
                    break
                line_index += 1
            line_index = start
        line_index += 1
    return blocks


def containing_block(blocks: list[tuple[int, int]], line_no: int) -> tuple[int, int] | None:
    best: tuple[int, int] | None = None
    for start, end in blocks:
        if start <= line_no <= end and (best is None or start > best[0]):
            best = (start, end)
    return best


def block_title(lines: list[str], bounds: tuple[int, int]) -> str | None:
    start, end = bounds
    for cursor in range(start, min(end + 1, start + 25)):
        stripped = lines[cursor].strip()
        if stripped.startswith("title:"):
            title = clean_title(first_qstr(stripped))
            if title:
                return title
    return None


def block_linked_bool_setting(lines: list[str], bounds: tuple[int, int]) -> str | None:
    start, end = bounds
    for cursor in range(start, min(end + 1, start + 35)):
        match = LINKED_BOOL_SETTING_RE.search(lines[cursor])
        if match:
            return match.group(1)
    return None


def parent_linked_bool_setting(
    lines: list[str], blocks: list[tuple[int, int]], bounds: tuple[int, int]
) -> str | None:
    start, end = bounds
    best: tuple[int, int] | None = None
    for candidate_start, candidate_end in blocks:
        if candidate_start < start and candidate_end >= end:
            if best is None or candidate_start > best[0]:
                best = (candidate_start, candidate_end)
    if best:
        return block_linked_bool_setting(lines, best)
    return None


def parent_for_containing_block(
    lines: list[str],
    titled_blocks: list[tuple[str, int, int]],
    bounds: tuple[int, int],
    current_key: str | None,
) -> str | None:
    start, end = bounds
    best: tuple[str, int, int] | None = None
    for kind, candidate_start, candidate_end in titled_blocks:
        if candidate_start < start and candidate_end >= end:
            if best is None or candidate_start > best[1]:
                best = (kind, candidate_start, candidate_end)
    if not best:
        return None

    kind, parent_start, parent_end = best
    if kind == "accordionCheck":
        parent_key = block_linked_bool_setting(lines, (parent_start, parent_end))
        if parent_key and parent_key != current_key:
            return parent_key
        return parent_for_containing_block(lines, titled_blocks, (parent_start, parent_end), current_key)

    return block_title(lines, (parent_start, parent_end))


def parent_for_line(
    lines: list[str],
    titled_blocks: list[tuple[str, int, int]],
    line_no: int,
    current_key: str | None,
) -> str | None:
    best: tuple[str, int, int] | None = None
    for kind, start, end in titled_blocks:
        if start <= line_no <= end:
            if best is None or start > best[1]:
                best = (kind, start, end)
    if not best:
        return None

    kind, start, end = best
    if kind == "accordionCheck":
        linked_key = block_linked_bool_setting(lines, (start, end))
        if linked_key and linked_key != current_key:
            return linked_key
        return parent_for_containing_block(lines, titled_blocks, (start, end), current_key)

    return block_title(lines, (start, end))


def nearest_group(lines: list[str], line_no: int) -> str | None:
    for cursor in range(line_no, max(-1, line_no - 220), -1):
        stripped = lines[cursor].strip()
        if stripped.startswith("title:"):
            title = clean_title(first_qstr(stripped))
            if title:
                return title
    return None


def infer_prefix_group(key: str) -> str:
    prefix_groups = [
        ("tile_", "Tiles"),
        ("tts_", "TTS"),
        ("shortcuts_", "Keyboard Shortcuts"),
        ("shortcut_", "Keyboard Shortcuts"),
        ("treadmill_inclination_override", "Treadmill Inclination Overrides"),
        ("treadmill_inclination_ovveride", "Treadmill Inclination Overrides"),
        ("peloton_", "Peloton"),
        ("garmin_", "Garmin"),
        ("intervalsicu_", "Intervals.icu"),
        ("strava_", "Strava"),
        ("zwift_", "Zwift"),
        ("ant_", "ANT+"),
        ("heart_", "Heart Rate"),
        ("power_", "Power"),
        ("cadence_", "Cadence"),
        ("proform_", "ProForm"),
        ("nordictrack_", "NordicTrack"),
    ]
    for prefix, group in prefix_groups:
        if key.startswith(prefix):
            return group
    return "General"


def extract_model(lines: list[str], bounds: tuple[int, int] | None) -> dict[str, Any] | None:
    if not bounds:
        return None
    start, end = bounds
    model_parts: list[str] = []
    collecting = False
    bracket_depth = 0
    for cursor in range(start, end + 1):
        line = lines[cursor].strip()
        if not collecting and line.startswith("model:"):
            value = line.split("model:", 1)[1].strip()
            model_parts.append(value)
            bracket_depth += value.count("[") - value.count("]")
            collecting = bracket_depth > 0
            if not collecting:
                break
        elif collecting:
            model_parts.append(line)
            bracket_depth += line.count("[") - line.count("]")
            if bracket_depth <= 0:
                break
    if not model_parts:
        return None
    expression = " ".join(model_parts).rstrip(";")
    if expression.startswith("[") and expression.endswith("]"):
        return {
            "source": "inline",
            "values": [decode_qml_string(value) for value in STRING_RE.findall(expression)],
        }
    return {"source": "expression", "expression": expression}


def find_title(lines: list[str], line_no: int, bounds: tuple[int, int] | None) -> str | None:
    if bounds:
        for cursor in range(line_no, bounds[0] - 1, -1):
            title = clean_title(first_qstr(lines[cursor]))
            if title:
                return title
    for cursor in range(line_no, max(-1, line_no - 30), -1):
        title = clean_title(first_qstr(lines[cursor]))
        if title:
            return title
    return None


def combo_id(lines: list[str], bounds: tuple[int, int]) -> str | None:
    start, end = bounds
    for cursor in range(start, min(end + 1, start + 20)):
        match = COMBO_ID_RE.search(lines[cursor])
        if match:
            return match.group(1)
    return None


def camel_to_snake(value: str) -> str:
    value = re.sub(r"ComboBox$", "", value)
    value = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", value)
    value = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", value)
    value = re.sub(r"[^A-Za-z0-9]+", "_", value)
    return value.strip("_").lower()


def virtual_setting_key(raw_id: str, title: str | None, parent: str | None) -> str:
    base = camel_to_snake(raw_id)
    normalized_parent = (parent or "").lower()
    normalized_title = (title or "").lower()

    if base in {"bike_model", "treadmill_model"} and "nord" in normalized_parent:
        return f"nordictrack_{base}"
    if base == "specific_model" and "bike" in normalized_title and "nord" in normalized_parent:
        return "nordictrack_bike_model"
    if base == "specific_model" and "treadmill" in normalized_title and "nord" in normalized_parent:
        return "nordictrack_treadmill_model"
    return base


def virtual_setting_title(lines: list[str], bounds: tuple[int, int]) -> str | None:
    start, _ = bounds
    for cursor in range(start - 1, max(-1, start - 18), -1):
        title = clean_title(first_qstr(lines[cursor]))
        if title:
            return title
    return None


def extract_virtual_settings(
    keys: set[str], declarations: dict[str, list[dict[str, Any]]]
) -> tuple[list[dict[str, Any]], dict[str, str]]:
    virtual_settings: list[dict[str, Any]] = []
    option_key_to_virtual_key: dict[str, str] = {}

    for source in QML_SOURCES:
        lines = read_lines(source)
        combo_blocks = qml_blocks(lines, r"\bComboBox\s*\{")
        titled_blocks = (
            [("accordion", start, end) for start, end in qml_blocks(lines, r"\bAccordionElement\s*\{")]
            + [
                ("staticAccordion", start, end)
                for start, end in qml_blocks(lines, r"\bStaticAccordionElement\s*\{")
            ]
            + [
                ("accordionCheck", start, end)
                for start, end in qml_blocks(lines, r"\bAccordionCheckElement\s*\{")
            ]
        )
        for bounds in combo_blocks:
            model = extract_model(lines, bounds)
            if not model or model.get("source") != "inline":
                continue

            start, end = bounds
            case_map: dict[int, str] = {}
            for cursor in range(start, end + 1):
                for case_match in CASE_TRUE_RE.finditer(lines[cursor]):
                    case_index = int(case_match.group(1))
                    case_key = case_match.group(2)
                    if case_key in keys:
                        case_map[case_index] = case_key

            boolean_cases = [
                key
                for key in case_map.values()
                if choose_primary_declaration(declarations[key])["type"] == "boolean"
            ]
            if len(boolean_cases) < 2:
                continue

            raw_id = combo_id(lines, bounds) or (virtual_setting_title(lines, bounds) or "virtual_setting")
            title = virtual_setting_title(lines, bounds)
            parent = parent_for_line(lines, titled_blocks, start, None) or infer_prefix_group(camel_to_snake(raw_id))
            virtual_key = virtual_setting_key(raw_id, title, parent)
            values = model["values"]
            options: list[dict[str, Any]] = []
            for index, label in enumerate(values):
                option: dict[str, Any] = {"label": label}
                if index in case_map:
                    option["sets"] = case_map[index]
                    option_key_to_virtual_key[case_map[index]] = virtual_key
                options.append(option)

            virtual_settings.append(
                {
                    "key": virtual_key,
                    "name": title or raw_id,
                    "description": None,
                    "parent": parent,
                    "type": "enum",
                    "control": "select",
                    "defaultValue": 0,
                    "options": options,
                }
            )

    return virtual_settings, option_key_to_virtual_key


def find_description(lines: list[str], line_no: int) -> str | None:
    for cursor in range(line_no + 1, min(len(lines), line_no + 55)):
        description = clean_description(first_qstr(lines[cursor]))
        if description:
            return description
        if re.search(r"\b(AccordionElement|SwitchDelegate|RowLayout|ComboBox|TextField)\s*\{", lines[cursor]):
            if cursor > line_no + 4:
                break
    return None


def extract_references(keys: set[str], virtual_option_keys: set[str]) -> dict[str, list[dict[str, Any]]]:
    references: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for source in QML_SOURCES:
        lines = read_lines(source)
        switch_blocks = qml_blocks(lines, r"\bSwitchDelegate\s*\{")
        combo_blocks = qml_blocks(lines, r"\bComboBox\s*\{")
        text_blocks = qml_blocks(lines, r"\bTextField\s*\{")
        button_blocks = qml_blocks(lines, r"\bButton\s*\{")
        row_blocks = qml_blocks(lines, r"\bRowLayout\s*\{")
        accordion_check_blocks = qml_blocks(lines, r"\bAccordionCheckElement\s*\{")
        accordion_blocks = qml_blocks(lines, r"\bAccordionElement\s*\{")
        static_accordion_blocks = qml_blocks(lines, r"\bStaticAccordionElement\s*\{")
        titled_blocks = (
            [("accordion", start, end) for start, end in accordion_blocks]
            + [("staticAccordion", start, end) for start, end in static_accordion_blocks]
            + [("accordionCheck", start, end) for start, end in accordion_check_blocks]
        )
        for index, line in enumerate(lines):
            linked_bool_keys = set(LINKED_BOOL_SETTING_RE.findall(line))
            found_keys = (
                set(SETTINGS_REF_RE.findall(line))
                | set(SETTING_NAME_RE.findall(line))
                | linked_bool_keys
            )
            for key in sorted(found_keys & keys):
                combo_bounds = containing_block(combo_blocks, index)
                row_bounds = containing_block(row_blocks, index)
                switch_bounds = containing_block(switch_blocks, index)
                text_bounds = containing_block(text_blocks, index)
                button_bounds = containing_block(button_blocks, index)
                accordion_check_bounds = containing_block(accordion_check_blocks, index)
                control = None
                title_bounds = row_bounds
                group = parent_for_line(lines, titled_blocks, index, key)
                title = None
                if switch_bounds:
                    control = "switch"
                    title_bounds = switch_bounds
                elif key in linked_bool_keys and accordion_check_bounds:
                    control = "switch"
                    title = block_title(lines, accordion_check_bounds)
                    linked_parent = parent_linked_bool_setting(
                        lines, accordion_check_blocks, accordion_check_bounds
                    )
                    if linked_parent and linked_parent in keys:
                        group = linked_parent
                    else:
                        group = parent_for_containing_block(lines, titled_blocks, accordion_check_bounds, key)
                elif accordion_check_bounds:
                    containing_linked_setting = block_linked_bool_setting(lines, accordion_check_bounds)
                    if containing_linked_setting and containing_linked_setting in keys and containing_linked_setting != key:
                        group = containing_linked_setting
                elif combo_bounds:
                    control = "select"
                elif text_bounds:
                    control = "text"
                elif button_bounds:
                    control = "button"
                model = extract_model(lines, combo_bounds)
                reference: dict[str, Any] = {
                    "file": source.as_posix(),
                    "line": index + 1,
                    "control": control,
                    "group": group or SOURCE_DEFAULT_PARENTS.get(source.as_posix()),
                    "title": title or find_title(lines, index, title_bounds),
                    "description": None if key in linked_bool_keys else find_description(lines, index),
                }
                if model:
                    if key not in virtual_option_keys:
                        reference["options"] = model
                        reference["control"] = "select"
                    else:
                        reference["control"] = "virtualOption"
                references[key].append({k: v for k, v in reference.items() if v is not None})
    return dict(references)


def choose_primary_declaration(declarations: list[dict[str, Any]]) -> dict[str, Any]:
    for declaration in declarations:
        if declaration["file"] == "src/settings.qml":
            return declaration
    return declarations[0]


def choose_first(references: list[dict[str, Any]], field: str) -> Any:
    for reference in references:
        value = reference.get(field)
        if value:
            return value
    return None


def build_catalog() -> dict[str, Any]:
    declarations = extract_declarations()
    keys = set(declarations)
    virtual_settings, virtual_option_map = extract_virtual_settings(keys, declarations)
    references = extract_references(keys, set(virtual_option_map))

    settings: list[dict[str, Any]] = []
    for key in sorted(keys, key=lambda item: choose_primary_declaration(declarations[item])["line"]):
        primary = choose_primary_declaration(declarations[key])
        refs = references.get(key, [])
        title = choose_first(refs, "title") or key.replace("_", " ").title()
        group = choose_first(refs, "group") or infer_prefix_group(key)
        options = choose_first(refs, "options")
        control = choose_first(refs, "control")
        if not control:
            control = "switch" if primary["type"] == "boolean" else "text"

        default_signatures = {
            (declaration["qmlType"], declaration["defaultExpression"]) for declaration in declarations[key]
        }
        setting: dict[str, Any] = {
            "key": key,
            "name": title,
            "description": choose_first(refs, "description"),
            "parent": group,
            "type": primary["type"],
            "qmlType": primary["qmlType"],
            "control": "virtualOption" if key in virtual_option_map else control,
            "defaultValue": primary["defaultValue"],
            "defaultExpression": primary["defaultExpression"],
            "options": None if key in virtual_option_map else options,
        }
        if key in virtual_option_map:
            setting["virtualParent"] = virtual_option_map[key]
        if len(default_signatures) > 1:
            setting["defaultConflicts"] = [
                {
                    "qmlType": declaration["qmlType"],
                    "type": declaration["type"],
                    "defaultExpression": declaration["defaultExpression"],
                    "defaultValue": declaration["defaultValue"],
                }
                for declaration in declarations[key]
            ]
        settings.append(setting)

    return {
        "$schema": "https://qdomyos-zwift.local/settings-catalog.schema.json",
        "schemaVersion": 1,
        "format": "qdomyos-zwift-settings-catalog",
        "settingCount": len(settings),
        "virtualSettings": virtual_settings,
        "settings": settings,
    }


def load_catalog() -> dict[str, Any]:
    with (REPO_ROOT / CATALOG_PATH).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def validate_catalog(catalog: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    declarations = extract_declarations()
    expected_keys = set(declarations)
    settings = catalog.get("settings", [])
    catalog_keys = [setting.get("key") for setting in settings]
    duplicate_keys = [key for key, count in Counter(catalog_keys).items() if count > 1]
    actual_keys = set(catalog_keys)

    if duplicate_keys:
        errors.append(f"Duplicate setting keys in catalog: {', '.join(sorted(duplicate_keys))}")
    missing = sorted(expected_keys - actual_keys)
    extra = sorted(actual_keys - expected_keys)
    if missing:
        errors.append(f"Missing settings in catalog: {', '.join(missing)}")
    if extra:
        errors.append(f"Settings not declared in QML sources: {', '.join(extra)}")
    if catalog.get("settingCount") != len(expected_keys):
        errors.append(
            f"settingCount is {catalog.get('settingCount')}, expected {len(expected_keys)}"
        )

    virtual_settings = catalog.get("virtualSettings", [])
    virtual_keys = [virtual_setting.get("key") for virtual_setting in virtual_settings]
    duplicate_virtual_keys = [key for key, count in Counter(virtual_keys).items() if count > 1]
    if duplicate_virtual_keys:
        errors.append(f"Duplicate virtual setting keys: {', '.join(sorted(duplicate_virtual_keys))}")

    virtual_key_set = set(virtual_keys)
    for virtual_setting in virtual_settings:
        for option in virtual_setting.get("options", []):
            target_key = option.get("sets")
            if target_key and target_key not in expected_keys:
                errors.append(f"{virtual_setting.get('key')}: option sets undeclared setting {target_key}")

    required_fields = {
        "key",
        "name",
        "description",
        "parent",
        "type",
        "qmlType",
        "control",
        "defaultValue",
        "defaultExpression",
        "options",
    }
    for setting in settings:
        key = setting.get("key", "<missing>")
        missing_fields = sorted(required_fields - set(setting))
        if missing_fields:
            errors.append(f"{key}: missing fields {', '.join(missing_fields)}")
        if "children" in setting:
            errors.append(f"{key}: children is redundant and should not be stored")
        if setting.get("virtualParent") and setting.get("virtualParent") not in virtual_key_set:
            errors.append(f"{key}: virtualParent {setting.get('virtualParent')} is not declared")
        if key in declarations:
            primary = choose_primary_declaration(declarations[key])
            if setting.get("qmlType") != primary["qmlType"]:
                errors.append(f"{key}: qmlType is {setting.get('qmlType')}, expected {primary['qmlType']}")
            if setting.get("defaultExpression") != primary["defaultExpression"]:
                errors.append(
                    f"{key}: defaultExpression is {setting.get('defaultExpression')}, "
                    f"expected {primary['defaultExpression']}"
                )

    expected_catalog = build_catalog()
    expected_settings = {setting["key"]: setting for setting in expected_catalog["settings"]}
    metadata_fields = [
        "name",
        "description",
        "parent",
        "type",
        "qmlType",
        "control",
        "defaultValue",
        "defaultExpression",
        "options",
        "virtualParent",
        "defaultConflicts",
    ]
    for setting in settings:
        key = setting.get("key")
        if key not in expected_settings:
            continue
        expected_setting = expected_settings[key]
        for field in metadata_fields:
            actual_value = setting.get(field)
            expected_value = expected_setting.get(field)
            if actual_value != expected_value:
                errors.append(f"{key}: {field} is {actual_value!r}, expected {expected_value!r}")

    expected_virtual_settings = expected_catalog.get("virtualSettings", [])
    if catalog.get("virtualSettings", []) != expected_virtual_settings:
        errors.append("virtualSettings do not match QML-derived catalog")
    return errors


def write_catalog(catalog: dict[str, Any]) -> None:
    output = json.dumps(catalog, indent=2, ensure_ascii=False)
    (REPO_ROOT / CATALOG_PATH).write_text(output + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate or validate src/settings-catalog.json")
    parser.add_argument("--check", action="store_true", help="validate the existing catalog")
    args = parser.parse_args()

    if args.check:
        errors = validate_catalog(load_catalog())
        if errors:
            for error in errors:
                print(error, file=sys.stderr)
            return 1
        print(f"{CATALOG_PATH.as_posix()} covers {len(extract_declarations())} settings")
        return 0

    catalog = build_catalog()
    write_catalog(catalog)
    print(f"Wrote {CATALOG_PATH.as_posix()} with {catalog['settingCount']} settings")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
