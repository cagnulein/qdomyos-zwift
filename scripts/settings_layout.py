#!/usr/bin/env python3
"""Derive display ordering and catalog-page ownership from the legacy QML UI."""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

CATALOG_PATH = Path("src/settings-catalog.json")
SETTINGS_FILES = [
    Path("src/settings.qml"),
    Path("src/settings-tiles.qml"),
    Path("src/settings-tts.qml"),
    Path("src/settings-shortcuts.qml"),
    Path("src/settings-treadmill-inclination-override.qml"),
]
SETTING_REF_RE = re.compile(r"\bsettings\.([A-Za-z_][A-Za-z0-9_]*)\b")
ELEMENT_RE = re.compile(r"^\s*(AccordionElement|StaticAccordionElement|AccordionCheckElement|NewPageElement)\s*\{")
ID_RE = re.compile(r"^\s*id\s*:\s*([A-Za-z_][A-Za-z0-9_]*)")
TITLE_RE = re.compile(r'^\s*title\s*:\s*qsTr\("((?:\\.|[^"\\])*)"\)')
TARGET_RE = re.compile(r'^\s*accordionContent\s*:\s*"([^"]+\.qml)"')
SETTINGS_BLOCK_RE = re.compile(r"^\s*Settings\s*\{")


def strip_comments(text: str) -> str:
    out = []
    i = 0
    single = double = block = escaped = False
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if block:
            if ch == "*" and nxt == "/":
                block = False
                out.extend("  ")
                i += 2
                continue
            out.append("\n" if ch == "\n" else " ")
            i += 1
            continue
        if escaped:
            out.append(ch)
            escaped = False
            i += 1
            continue
        if ch == "\\" and (single or double):
            out.append(ch)
            escaped = True
            i += 1
            continue
        if ch == "'" and not double:
            single = not single
            out.append(ch)
            i += 1
            continue
        if ch == '"' and not single:
            double = not double
            out.append(ch)
            i += 1
            continue
        if not single and not double and ch == "/" and nxt == "*":
            block = True
            out.extend("  ")
            i += 2
            continue
        if not single and not double and ch == "/" and nxt == "/":
            while i < len(text) and text[i] != "\n":
                out.append(" ")
                i += 1
            continue
        out.append(ch)
        i += 1
    return "".join(out)


def brace_delta(line: str) -> int:
    delta = 0
    single = double = escaped = False
    for ch in line:
        if escaped:
            escaped = False
            continue
        if ch == "\\" and (single or double):
            escaped = True
            continue
        if ch == "'" and not double:
            single = not single
            continue
        if ch == '"' and not single:
            double = not double
            continue
        if single or double:
            continue
        if ch == "{": delta += 1
        elif ch == "}": delta -= 1
    return delta


def ui_first_reference_lines(text: str):
    cleaned = strip_comments(text)
    result = {}
    in_settings = False
    settings_depth = 0
    depth = 0
    for line_no, line in enumerate(cleaned.splitlines(), 1):
        if not in_settings and SETTINGS_BLOCK_RE.match(line):
            in_settings = True
            settings_depth = depth + brace_delta(line)
            depth += brace_delta(line)
            continue
        if in_settings:
            depth += brace_delta(line)
            if depth < settings_depth:
                in_settings = False
            continue
        for key in SETTING_REF_RE.findall(line):
            result.setdefault(key, line_no)
        depth += brace_delta(line)
    return result


def parse_navigation(text: str):
    cleaned = strip_comments(text)
    lines = cleaned.splitlines()
    depth = 0
    stack = []
    accordions = []
    pages = []

    for line_no, line in enumerate(lines, 1):
        match = ELEMENT_RE.match(line)
        if match:
            kind = match.group(1)
            parent_accordion = None
            for item in reversed(stack):
                if item["kind"] != "NewPageElement":
                    parent_accordion = item
                    break
            item = {
                "kind": kind,
                "id": None,
                "title": None,
                "target": None,
                "line": line_no,
                "baseDepth": depth,
                "parentAccordionId": parent_accordion.get("id") if parent_accordion else None,
            }
            stack.append(item)

        if stack:
            item = stack[-1]
            if item["id"] is None:
                m = ID_RE.match(line)
                if m:
                    item["id"] = m.group(1)
            if item["title"] is None:
                m = TITLE_RE.match(line)
                if m:
                    item["title"] = bytes(m.group(1), "utf-8").decode("unicode_escape")
            if item["target"] is None:
                m = TARGET_RE.match(line)
                if m:
                    item["target"] = m.group(1)

        depth += brace_delta(line)
        while stack and depth <= stack[-1]["baseDepth"]:
            item = stack.pop()
            if item["kind"] == "NewPageElement":
                if item["title"] and item["target"]:
                    pages.append(item)
            elif item["id"] and item["title"]:
                accordions.append(item)

    return accordions, pages


def source_file_by_key(root: Path):
    from settings_audit import parse_declarations
    result = {}
    for path in SETTINGS_FILES:
        text = (root / path).read_text(encoding="utf-8")
        for decl in parse_declarations(text, path.as_posix()):
            # Prefer the dedicated secondary settings page over the duplicate declaration in settings.qml.
            if decl.key not in result or path.name != "settings.qml":
                result[decl.key] = path.name
    return result


def normalize_name(value: str):
    return (value or "").replace("🔊", "").replace("⌨️", "").strip().casefold()


def derive(root: Path, catalog: dict):
    main_text = (root / SETTINGS_FILES[0]).read_text(encoding="utf-8")
    hierarchy = catalog.get("legacyHierarchy") or {}
    hierarchy_nodes = hierarchy.get("nodes") or []
    node_keys = {n.get("key") for n in hierarchy_nodes}

    refs_by_file = {}
    nav_by_file = {}
    for path in SETTINGS_FILES:
        text = (root / path).read_text(encoding="utf-8")
        refs_by_file[path.name] = ui_first_reference_lines(text)
        nav_by_file[path.name] = parse_navigation(text)

    source_map = source_file_by_key(root)
    item_order = {}
    for key, source_name in source_map.items():
        if key in refs_by_file.get(source_name, {}):
            item_order[key] = refs_by_file[source_name][key]
        elif key in refs_by_file.get("settings.qml", {}):
            item_order[key] = refs_by_file["settings.qml"][key]

    page_parent_node = {}
    page_order = {}
    page_parent_target = {}

    page_entries = catalog.get("pages") or []
    for entry in page_entries:
        wanted_name = normalize_name(entry.get("name"))
        wanted_target = entry.get("target")
        best = None
        best_source = None
        for source_name, (_accordions, pages) in nav_by_file.items():
            for page in pages:
                if page.get("target") == wanted_target or normalize_name(page.get("title")) == wanted_name:
                    best = page
                    best_source = source_name
                    break
            if best:
                break
        if not best:
            continue
        page_order[entry["key"]] = best["line"]
        if best_source == "settings.qml":
            parent_id = best.get("parentAccordionId")
            if parent_id in node_keys:
                page_parent_node[entry["key"]] = parent_id
        else:
            page_parent_target[entry["key"]] = best_source

    external_entry_order = {}
    for key, source_name in source_map.items():
        if source_name != "settings.qml" and key in refs_by_file.get(source_name, {}):
            external_entry_order[key] = refs_by_file[source_name][key]

    return {
        "source": "legacy QML visual order and NewPageElement navigation",
        "sourceFileByKey": dict(sorted(source_map.items())),
        "itemOrderByKey": dict(sorted(item_order.items())),
        "externalEntryOrderByKey": dict(sorted(external_entry_order.items())),
        "pageNodeByKey": dict(sorted(page_parent_node.items())),
        "pageOrderByKey": dict(sorted(page_order.items())),
        "pageParentTargetByKey": dict(sorted(page_parent_target.items())),
    }


def git_show(root: Path, revision: str, path: Path) -> str:
    proc = subprocess.run(["git", "show", f"{revision}:{path.as_posix()}"], cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if proc.returncode:
        raise RuntimeError(proc.stderr.strip())
    return proc.stdout


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()
    root = Path(args.root).resolve()
    catalog_path = root / CATALOG_PATH
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    expected = derive(root, catalog)

    if args.write:
        catalog["legacyLayout"] = expected
        catalog_path.write_text(json.dumps(catalog, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"Wrote legacy layout metadata for {len(expected['sourceFileByKey'])} settings")
        return 0

    if catalog.get("legacyLayout") != expected:
        print("ERROR: settings-catalog.json legacyLayout is stale; run scripts/settings_layout.py --write", file=sys.stderr)
        return 1

    print("Settings legacy layout metadata passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
