#!/usr/bin/env python3

import json
import subprocess
import sys
from pathlib import Path

from check_settings_property_order import settings_properties


CATALOG_PATH = Path("src/settings-catalog.json")
SETTINGS_PATH = Path("src/settings.qml")
EXPECTED_FORMAT = "qdomyos-zwift-settings-catalog"
SUPPORTED_TYPES = {"boolean", "integer", "number", "string", "enum", "page"}
SUPPORTED_CONTROLS = {"button", "internal", "select", "switch", "text", "textfield", "virtualOption", "page"}
EXPECTED_QML_TYPES = {
    "boolean": {"bool"},
    "integer": {"int"},
    "number": {"double", "real"},
    "string": {"string", "var"},
}


def read_git_file(ref: str, path: Path) -> str:
    try:
        return subprocess.check_output(
            ["git", "show", f"{ref}:{path.as_posix()}"],
            text=True,
        )
    except subprocess.CalledProcessError as exc:
        raise RuntimeError(f"Unable to read {path} from base ref {ref}") from exc


def load_json(text: str, source: str) -> dict:
    try:
        value = json.loads(text)
    except json.JSONDecodeError as exc:
        raise RuntimeError(f"{source} is not valid JSON: {exc}") from exc

    if not isinstance(value, dict):
        raise RuntimeError(f"{source} must contain a JSON object")
    return value


def require_string(entry: dict, field: str, location: str, errors: list[str]) -> None:
    if not isinstance(entry.get(field), str) or not entry[field].strip():
        errors.append(f"{location}: '{field}' must be a non-empty string")


def validate_options(options: object, location: str, errors: list[str]) -> None:
    if options is None:
        return

    if isinstance(options, list):
        for index, option in enumerate(options):
            option_location = f"{location}.options[{index}]"
            if not isinstance(option, dict):
                errors.append(f"{option_location}: must be an object")
                continue
            require_string(option, "label", option_location, errors)
            if "sets" in option and not isinstance(option["sets"], str):
                errors.append(f"{option_location}: 'sets' must be a string")
        return

    if not isinstance(options, dict):
        errors.append(f"{location}.options: must be null, an object, or an array")
        return

    if "values" in options and not isinstance(options["values"], list):
        errors.append(f"{location}.options.values: must be an array")
    if "labels" in options and not isinstance(options["labels"], list):
        errors.append(f"{location}.options.labels: must be an array")
    if "expression" in options and not isinstance(options["expression"], str):
        errors.append(f"{location}.options.expression: must be a string")
    if "source" in options and not isinstance(options["source"], str):
        errors.append(f"{location}.options.source: must be a string")

    values = options.get("values")
    labels = options.get("labels")
    if isinstance(values, list) and isinstance(labels, list) and len(values) != len(labels):
        errors.append(f"{location}.options: 'values' and 'labels' must have the same length")

    if not any(field in options for field in ("values", "expression")):
        errors.append(f"{location}.options: expected 'values' or 'expression'")


def validate_entry(entry: object, location: str, kind: str, errors: list[str]) -> None:
    if not isinstance(entry, dict):
        errors.append(f"{location}: must be an object")
        return

    common_fields = ["key", "name", "description", "parent", "type", "control"]
    if kind == "setting":
        common_fields += [
            "qmlType",
            "visible",
            "defaultValue",
            "defaultExpression",
            "options",
            "restartRequired",
        ]
    elif kind == "virtual":
        common_fields += ["defaultValue", "options", "restartRequired"]
        # The two legacy model selectors predate the complete catalog format.
        if entry.get("type") != "enum":
            common_fields += ["defaultExpression", "visible"]
    else:
        common_fields += ["target", "visible"]

    for field in common_fields:
        if field not in entry:
            errors.append(f"{location}: missing required field '{field}'")

    for field in ("key", "name", "parent"):
        require_string(entry, field, location, errors)

    if "description" in entry and entry["description"] is not None and not isinstance(entry["description"], str):
        errors.append(f"{location}: 'description' must be a string or null")

    if entry.get("type") not in SUPPORTED_TYPES:
        errors.append(f"{location}: unsupported type {entry.get('type')!r}")
    if entry.get("control") not in SUPPORTED_CONTROLS:
        errors.append(f"{location}: unsupported control {entry.get('control')!r}")
    if kind == "setting" and entry.get("type") in EXPECTED_QML_TYPES:
        if entry.get("qmlType") not in EXPECTED_QML_TYPES[entry["type"]]:
            errors.append(
                f"{location}: qmlType {entry.get('qmlType')!r} does not match "
                f"type {entry.get('type')!r}"
            )

    if kind == "setting" and not isinstance(entry.get("defaultExpression"), str):
        errors.append(f"{location}: 'defaultExpression' must be a string")
    if kind in ("setting", "virtual") and not isinstance(entry.get("restartRequired"), bool):
        errors.append(f"{location}: 'restartRequired' must be boolean")
    if kind == "setting" and not isinstance(entry.get("visible"), bool):
        errors.append(f"{location}: 'visible' must be boolean")
    if kind == "virtual" and entry.get("type") != "enum" and not isinstance(entry.get("visible"), bool):
        errors.append(f"{location}: 'visible' must be boolean")
    if kind == "page" and not isinstance(entry.get("visible"), bool):
        errors.append(f"{location}: 'visible' must be boolean")
    if kind == "page":
        require_string(entry, "target", location, errors)

    if kind != "page":
        if "options" in entry:
            validate_options(entry["options"], location, errors)
        else:
            errors.append(f"{location}: missing required field 'options'")


def validate_catalog(catalog: dict, qml_text: str) -> tuple[list[str], set[str]]:
    errors: list[str] = []

    if catalog.get("schemaVersion") != 1:
        errors.append("top level: 'schemaVersion' must be 1")
    if catalog.get("format") != EXPECTED_FORMAT:
        errors.append(f"top level: 'format' must be {EXPECTED_FORMAT!r}")

    hierarchy = catalog.get("legacyHierarchy")
    if not isinstance(hierarchy, dict):
        errors.append("top level: 'legacyHierarchy' must be an object")
    else:
        for field in ("nodes", "settingNodeByKey", "virtualNodeByKey", "pageNodeByKey"):
            expected_type = list if field == "nodes" else dict
            if not isinstance(hierarchy.get(field), expected_type):
                errors.append(f"legacyHierarchy: '{field}' has the wrong type")

    layout = catalog.get("legacyLayout")
    if not isinstance(layout, dict):
        errors.append("top level: 'legacyLayout' must be an object")
    else:
        for field in ("rootPages",):
            if not isinstance(layout.get(field), list):
                errors.append(f"legacyLayout: '{field}' has the wrong type")
        for field in (
            "sourceFileByKey",
            "itemOrderByKey",
            "pageOrderByKey",
            "pageParentTargetByKey",
            "pageNodeByKey",
            "externalEntryOrderByKey",
        ):
            if not isinstance(layout.get(field), dict):
                errors.append(f"legacyLayout: '{field}' has the wrong type")

    for collection_name in ("settings", "virtualSettings", "pages"):
        if not isinstance(catalog.get(collection_name), list):
            errors.append(f"top level: '{collection_name}' must be an array")

    settings = catalog.get("settings", [])
    virtual_settings = catalog.get("virtualSettings", [])
    pages = catalog.get("pages", [])
    if not isinstance(settings, list) or not isinstance(virtual_settings, list) or not isinstance(pages, list):
        return errors, set()

    if catalog.get("settingCount") != len(settings):
        errors.append(
            f"top level: 'settingCount' is {catalog.get('settingCount')!r}, expected {len(settings)}"
        )

    all_keys: dict[str, str] = {}
    for collection_name, entries, kind in (
        ("settings", settings, "setting"),
        ("virtualSettings", virtual_settings, "virtual"),
        ("pages", pages, "page"),
    ):
        for index, entry in enumerate(entries):
            location = f"{collection_name}[{index}]"
            validate_entry(entry, location, kind, errors)
            if isinstance(entry, dict) and isinstance(entry.get("key"), str):
                key = entry["key"]
                if key in all_keys:
                    errors.append(f"{location}: duplicate key {key!r}; already used by {all_keys[key]}")
                else:
                    all_keys[key] = location

    try:
        qml_keys = set(settings_properties(qml_text))
    except RuntimeError as exc:
        errors.append(str(exc))
        return errors, {
            entry["key"]
            for entry in settings
            if isinstance(entry, dict) and isinstance(entry.get("key"), str)
        }

    catalog_keys = {
        entry["key"]
        for entry in settings
        if isinstance(entry, dict) and isinstance(entry.get("key"), str)
    }
    missing_from_catalog = sorted(qml_keys - catalog_keys)
    missing_from_qml = sorted(catalog_keys - qml_keys)
    if missing_from_catalog:
        errors.append("settings.qml properties missing from catalog: " + ", ".join(missing_from_catalog))
    if missing_from_qml:
        errors.append("catalog settings missing from settings.qml: " + ", ".join(missing_from_qml))

    return errors, catalog_keys


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <base-git-ref>", file=sys.stderr)
        return 2

    base_ref = sys.argv[1]
    current_catalog_text = CATALOG_PATH.read_text(encoding="utf-8")
    current_catalog = load_json(current_catalog_text, CATALOG_PATH.as_posix())
    current_qml_text = SETTINGS_PATH.read_text(encoding="utf-8")
    errors, current_keys = validate_catalog(current_catalog, current_qml_text)

    base_catalog = load_json(read_git_file(base_ref, CATALOG_PATH), f"{base_ref}:{CATALOG_PATH}")
    base_settings = base_catalog.get("settings", [])
    base_keys = {
        entry["key"]
        for entry in base_settings
        if isinstance(entry, dict) and isinstance(entry.get("key"), str)
    }
    new_keys = sorted(current_keys - base_keys)

    if current_catalog_text == read_git_file(base_ref, CATALOG_PATH):
        print("OK: src/settings-catalog.json is unchanged in this PR.")
    else:
        print(f"Catalog changed: {len(new_keys)} new setting(s).")
        for key in new_keys:
            print(f"  + {key}")

    if errors:
        print("ERROR: settings catalog validation failed:", file=sys.stderr)
        for error in errors:
            print(f"  - {error}", file=sys.stderr)
        return 1

    print(f"OK: validated {len(current_catalog['settings'])} persistent settings, "
          f"{len(current_catalog['virtualSettings'])} virtual settings, and "
          f"{len(current_catalog['pages'])} pages.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError, KeyError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(2)
