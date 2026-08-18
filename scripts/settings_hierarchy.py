#!/usr/bin/env python3
"""Derive the modern settings hierarchy from the existing legacy settings.qml UI."""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path

QML_PATH = Path("src/settings.qml")
CATALOG_PATH = Path("src/settings-catalog.json")
ACC_RE = re.compile(r"^\s*AccordionElement\s*\{")
ID_RE = re.compile(r"^\s*id\s*:\s*([A-Za-z_][A-Za-z0-9_]*)")
TITLE_RE = re.compile(r'^\s*title\s*:\s*qsTr\("((?:\\.|[^"\\])*)"\)')
SETTING_REF_RE = re.compile(r"\bsettings\.([A-Za-z_][A-Za-z0-9_]*)\b")
SETTING_WRITE_RE = re.compile(r"\bsettings\.([A-Za-z_][A-Za-z0-9_]*)\s*=(?!=)")


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


def parse_hierarchy(text: str):
    cleaned = strip_comments(text)
    nodes = []
    stack = []
    depth = 0
    refs = defaultdict(Counter)
    writes = defaultdict(Counter)

    for line_no, line in enumerate(cleaned.splitlines(), 1):
        if ACC_RE.match(line):
            parent = stack[-1]["index"] if stack else None
            node = {
                "key": None,
                "name": None,
                "parentIndex": parent,
                "depth": len(stack),
                "line": line_no,
                "baseDepth": depth,
            }
            nodes.append(node)
            stack.append({"index": len(nodes) - 1, "baseDepth": depth})

        if stack:
            node = nodes[stack[-1]["index"]]
            if node["key"] is None:
                m = ID_RE.match(line)
                if m:
                    node["key"] = m.group(1)
            if node["name"] is None:
                m = TITLE_RE.match(line)
                if m:
                    node["name"] = bytes(m.group(1), "utf-8").decode("unicode_escape")

            current_node = stack[-1]["index"]
            for key in SETTING_REF_RE.findall(line):
                refs[key][current_node] += 1
            for key in SETTING_WRITE_RE.findall(line):
                writes[key][current_node] += 1

        depth += brace_delta(line)
        while stack and depth <= stack[-1]["baseDepth"]:
            stack.pop()

    # Keep only valid accordion nodes and translate parent indexes to stable keys.
    valid_old_indexes = [i for i, n in enumerate(nodes) if n["key"] and n["name"]]
    valid_set = set(valid_old_indexes)
    output_nodes = []
    for i in valid_old_indexes:
        n = nodes[i]
        parent_i = n["parentIndex"]
        while parent_i is not None and parent_i not in valid_set:
            parent_i = nodes[parent_i]["parentIndex"]
        output_nodes.append({
            "key": n["key"],
            "name": n["name"],
            "parent": nodes[parent_i]["key"] if parent_i is not None else None,
            "depth": n["depth"],
            "sourceLine": n["line"],
        })

    valid_key_by_index = {i: nodes[i]["key"] for i in valid_old_indexes}
    setting_nodes = {}
    write_only = set()
    for key, counts in refs.items():
        candidates = [(count, nodes[idx]["depth"], idx) for idx, count in counts.items() if idx in valid_key_by_index]
        if not candidates:
            continue
        _, _, best = max(candidates)
        setting_nodes[key] = valid_key_by_index[best]
        non_write_refs = sum(counts.values()) - sum(writes.get(key, {}).values())
        if non_write_refs <= 0:
            write_only.add(key)

    return output_nodes, setting_nodes, write_only


def node_for_parent(parent_name: str, nodes):
    if not parent_name:
        return None
    exact = [n for n in nodes if n["name"].strip().casefold() == parent_name.strip().casefold()]
    if not exact:
        return None
    return sorted(exact, key=lambda n: (n["depth"], n["sourceLine"]))[-1]["key"]


def serialized(value) -> bool:
    if value is None:
        return False
    text = str(value)
    return "|" in text and ";" in text


def derive(catalog: dict, qml_text: str):
    nodes, detected_setting_nodes, write_only = parse_hierarchy(qml_text)
    entry_nodes = {}

    for entry in catalog.get("settings", []):
        key = entry.get("key")
        node = detected_setting_nodes.get(key) or node_for_parent(entry.get("parent"), nodes)
        if node:
            entry_nodes[key] = node

        # A write-only persistent string is storage/state, not a generic user-editable field.
        if entry.get("qmlType") == "string" and key in write_only:
            entry["visible"] = False
            entry["control"] = "internal"

        if serialized(entry.get("defaultValue")) or serialized(entry.get("defaultExpression")):
            entry["visible"] = False
            entry["control"] = "internal"

    virtual_nodes = {}
    for entry in catalog.get("virtualSettings", []):
        node = node_for_parent(entry.get("parent"), nodes)
        if node:
            virtual_nodes[entry["key"]] = node

    page_nodes = {}
    for entry in catalog.get("pages", []):
        node = node_for_parent(entry.get("parent"), nodes)
        if node:
            page_nodes[entry["key"]] = node

    hierarchy = {
        "source": "src/settings.qml AccordionElement nesting",
        "nodes": nodes,
        "settingNodeByKey": dict(sorted(entry_nodes.items())),
        "virtualNodeByKey": dict(sorted(virtual_nodes.items())),
        "pageNodeByKey": dict(sorted(page_nodes.items())),
    }
    return hierarchy


def git_show(root: Path, revision: str, path: Path) -> str:
    proc = subprocess.run(["git", "show", f"{revision}:{path.as_posix()}"], cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if proc.returncode:
        raise RuntimeError(proc.stderr.strip())
    return proc.stdout


def persistent_keys(text: str):
    from settings_audit import parse_declarations
    return {d.key for d in parse_declarations(text, QML_PATH.as_posix())}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    ap.add_argument("--write", action="store_true")
    ap.add_argument("--master", help="latest master revision/ref to compare for newly added persistent settings")
    args = ap.parse_args()
    root = Path(args.root).resolve()
    qml_path = root / QML_PATH
    catalog_path = root / CATALOG_PATH
    qml = qml_path.read_text(encoding="utf-8")
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    expected = derive(json.loads(json.dumps(catalog)), qml)

    errors = []
    current = catalog.get("legacyHierarchy")
    if current != expected:
        errors.append("settings-catalog.json legacyHierarchy is stale; run scripts/settings_hierarchy.py --write")

    # Generic visible string fields must not have a serialized default.
    bad_serialized = [e["key"] for e in catalog.get("settings", []) if e.get("visible") and (serialized(e.get("defaultValue")) or serialized(e.get("defaultExpression")))]
    if bad_serialized:
        errors.append("serialized string settings must not be visible: " + ", ".join(sorted(bad_serialized)))

    if args.master:
        master_qml = git_show(root, args.master, QML_PATH)
        master_keys = persistent_keys(master_qml)
        pr_keys = persistent_keys(qml)
        missing = sorted(master_keys - pr_keys)
        if missing:
            errors.append("PR settings layout is stale vs latest master; persistent settings missing from PR: " + ", ".join(missing))
        catalog_keys = {e.get("key") for e in catalog.get("settings", [])}
        missing_catalog = sorted(master_keys - catalog_keys)
        if missing_catalog:
            errors.append("latest master persistent settings missing from PR catalog/layout: " + ", ".join(missing_catalog))

    if args.write:
        updated = json.loads(json.dumps(catalog))
        updated["legacyHierarchy"] = derive(updated, qml)
        catalog_path.write_text(json.dumps(updated, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"Wrote {len(updated['legacyHierarchy']['nodes'])} legacy hierarchy nodes")
        return 0

    if errors:
        for e in errors:
            print("ERROR: " + e, file=sys.stderr)
        return 1
    roots = [n for n in expected["nodes"] if n["parent"] is None]
    print(f"Settings hierarchy passed: {len(expected['nodes'])} nodes, {len(roots)} roots")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
