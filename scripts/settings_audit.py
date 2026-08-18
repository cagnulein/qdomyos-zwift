#!/usr/bin/env python3
"""Audit QZ persistent settings and guard their compatibility during UI refactors."""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

SETTINGS_FILES = [
    "src/settings.qml",
    "src/settings-tiles.qml",
    "src/settings-tts.qml",
    "src/settings-shortcuts.qml",
    "src/settings-treadmill-inclination-override.qml",
]
CATALOG_PATH = "src/settings-catalog.json"
PROPERTY_RE = re.compile(r"^\s*property\s+([A-Za-z_][\w<>.]*)\s+([A-Za-z_][\w]*)\s*:\s*(.*?)\s*$")
SETTINGS_BLOCK_RE = re.compile(r"^\s*Settings\s*\{")
SETTINGS_REF_RE = re.compile(r"\bsettings\.([A-Za-z_][\w]*)\b")
SETTING_WRITE_RE = re.compile(r"\bsettings\.([A-Za-z_][\w]*)\s*=")


@dataclass(frozen=True)
class SettingDecl:
    source: str
    line: int
    qml_type: str
    key: str
    default_expression: str

    def abi_tuple(self) -> Tuple[str, str, str]:
        return (self.key, self.qml_type, normalize_expression(self.default_expression))


def normalize_expression(expr: str) -> str:
    return re.sub(r"\s+", " ", expr.strip())


def strip_comments(text: str) -> str:
    """Remove QML comments while preserving newlines and quoted comment markers."""
    result: List[str] = []
    in_single = False
    in_double = False
    in_block_comment = False
    escaped = False
    i = 0
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if in_block_comment:
            if ch == "*" and nxt == "/":
                in_block_comment = False
                result.extend("  ")
                i += 2
                continue
            result.append("\n" if ch == "\n" else " ")
            i += 1
            continue

        if escaped:
            result.append(ch)
            escaped = False
            i += 1
            continue

        if ch == "\\" and (in_single or in_double):
            result.append(ch)
            escaped = True
            i += 1
            continue

        if ch == "'" and not in_double:
            in_single = not in_single
            result.append(ch)
            i += 1
            continue

        if ch == '"' and not in_single:
            in_double = not in_double
            result.append(ch)
            i += 1
            continue

        if not in_single and not in_double:
            if ch == "/" and nxt == "*":
                in_block_comment = True
                result.extend("  ")
                i += 2
                continue
            if ch == "/" and nxt == "/":
                while i < len(text) and text[i] != "\n":
                    result.append(" ")
                    i += 1
                continue

        result.append(ch)
        i += 1

    return "".join(result)


def brace_delta(line: str) -> int:
    """Count structural braces, ignoring braces inside strings."""
    delta = 0
    in_single = False
    in_double = False
    escaped = False
    for ch in line:
        if escaped:
            escaped = False
            continue
        if ch == "\\" and (in_single or in_double):
            escaped = True
            continue
        if ch == "'" and not in_double:
            in_single = not in_single
            continue
        if ch == '"' and not in_single:
            in_double = not in_double
            continue
        if in_single or in_double:
            continue
        if ch == "{":
            delta += 1
        elif ch == "}":
            delta -= 1
    return delta


def parse_declarations(text: str, source: str) -> List[SettingDecl]:
    """Return only properties declared inside Qt.labs.settings Settings blocks."""
    declarations: List[SettingDecl] = []
    cleaned = strip_comments(text)
    in_settings = False
    depth = 0

    for line_no, line in enumerate(cleaned.splitlines(), 1):
        if not in_settings:
            if not SETTINGS_BLOCK_RE.match(line):
                continue
            in_settings = True
            depth = brace_delta(line)
            if depth <= 0:
                in_settings = False
            continue

        match = PROPERTY_RE.match(line)
        if match:
            qml_type, key, default_expr = match.groups()
            declarations.append(SettingDecl(source, line_no, qml_type, key, default_expr))

        depth += brace_delta(line)
        if depth <= 0:
            in_settings = False
            depth = 0

    return declarations


def read_worktree(root: Path, rel_path: str) -> str:
    return (root / rel_path).read_text(encoding="utf-8")


def read_git_blob(root: Path, revision: str, rel_path: str) -> Optional[str]:
    proc = subprocess.run(
        ["git", "show", f"{revision}:{rel_path}"],
        cwd=str(root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )
    if proc.returncode != 0:
        return None
    return proc.stdout


def declarations_from(root: Path, revision: Optional[str] = None) -> Dict[str, List[SettingDecl]]:
    result: Dict[str, List[SettingDecl]] = {}
    for rel_path in SETTINGS_FILES:
        text = read_git_blob(root, revision, rel_path) if revision else read_worktree(root, rel_path)
        if text is None:
            result[rel_path] = []
        else:
            result[rel_path] = parse_declarations(text, rel_path)
    return result


def flatten(decls_by_file: Dict[str, List[SettingDecl]]) -> List[SettingDecl]:
    return [decl for path in SETTINGS_FILES for decl in decls_by_file.get(path, [])]


def duplicate_keys(declarations: Iterable[SettingDecl]) -> Dict[str, List[SettingDecl]]:
    grouped: Dict[str, List[SettingDecl]] = {}
    for decl in declarations:
        grouped.setdefault(decl.key, []).append(decl)
    return {key: items for key, items in grouped.items() if len(items) > 1}


def compare_abi(base: Dict[str, List[SettingDecl]], head: Dict[str, List[SettingDecl]]) -> List[str]:
    errors: List[str] = []
    for path in SETTINGS_FILES:
        old = base.get(path, [])
        new = head.get(path, [])
        if len(new) < len(old):
            errors.append(f"{path}: removed {len(old) - len(new)} persistent declaration(s)")
            continue
        for index, old_decl in enumerate(old):
            new_decl = new[index]
            if old_decl.abi_tuple() != new_decl.abi_tuple():
                errors.append(
                    f"{path}: ABI changed at persistent property #{index + 1}: "
                    f"base={old_decl.abi_tuple()} head={new_decl.abi_tuple()} "
                    f"(base line {old_decl.line}, head line {new_decl.line})"
                )
    return errors


def load_catalog(root: Path) -> dict:
    with (root / CATALOG_PATH).open("r", encoding="utf-8") as handle:
        return json.load(handle)


def catalog_errors(root: Path, declarations: Sequence[SettingDecl]) -> List[str]:
    catalog = load_catalog(root)
    entries = catalog.get("settings", [])
    errors: List[str] = []

    declared_keys = {decl.key for decl in declarations}
    catalog_keys = [entry.get("key") for entry in entries]
    catalog_key_set = set(catalog_keys)

    seen = set()
    duplicates = set()
    for key in catalog_keys:
        if key in seen and key:
            duplicates.add(key)
        seen.add(key)
    if duplicates:
        errors.append("catalog has duplicate keys: " + ", ".join(sorted(duplicates)))

    missing = sorted(declared_keys - catalog_key_set)
    stale = sorted(catalog_key_set - declared_keys)
    if missing:
        errors.append("persistent settings missing from catalog: " + ", ".join(missing))
    if stale:
        errors.append("catalog keys without a persistent declaration: " + ", ".join(stale))

    expected_count = catalog.get("settingCount")
    if expected_count != len(catalog_key_set):
        errors.append(
            f"catalog settingCount={expected_count!r}, but catalog contains {len(catalog_key_set)} unique setting keys"
        )

    qml_types: Dict[str, set] = {}
    for decl in declarations:
        qml_types.setdefault(decl.key, set()).add(decl.qml_type)
    for entry in entries:
        key = entry.get("key")
        if key not in qml_types:
            continue
        catalog_type = entry.get("qmlType")
        if catalog_type and catalog_type not in qml_types[key]:
            errors.append(
                f"catalog qmlType mismatch for {key}: catalog={catalog_type!r}, declarations={sorted(qml_types[key])!r}"
            )

    return errors


def behavior_inventory(root: Path, declarations: Sequence[SettingDecl]) -> dict:
    known = {decl.key for decl in declarations}
    inventory = {
        key: {"references": [], "writes": [], "visibilityOrEnabled": []}
        for key in sorted(known)
    }
    for rel_path in SETTINGS_FILES:
        text = read_worktree(root, rel_path)
        for line_no, line in enumerate(text.splitlines(), 1):
            refs = SETTINGS_REF_RE.findall(line)
            writes = set(SETTING_WRITE_RE.findall(line))
            for key in refs:
                if key not in inventory:
                    continue
                inventory[key]["references"].append({"source": rel_path, "line": line_no})
                if key in writes:
                    inventory[key]["writes"].append({"source": rel_path, "line": line_no, "code": line.strip()})
                if re.search(r"\b(visible|enabled)\s*:", line):
                    inventory[key]["visibilityOrEnabled"].append(
                        {"source": rel_path, "line": line_no, "code": line.strip()}
                    )
    return inventory


def build_report(root: Path, declarations: Sequence[SettingDecl]) -> dict:
    duplicates = duplicate_keys(declarations)
    return {
        "persistentDeclarationCount": len(declarations),
        "uniquePersistentSettingCount": len({decl.key for decl in declarations}),
        "files": {
            path: len(parse_declarations(read_worktree(root, path), path))
            for path in SETTINGS_FILES
        },
        "duplicates": {
            key: [asdict(item) for item in items] for key, items in sorted(duplicates.items())
        },
        "behavior": behavior_inventory(root, declarations),
    }


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="repository root")
    parser.add_argument("--base", help="git revision to compare against; existing settings must remain an ordered prefix")
    parser.add_argument("--report", help="write a JSON audit report")
    parser.add_argument("--skip-catalog", action="store_true")
    args = parser.parse_args(argv)

    root = Path(args.root).resolve()
    head_by_file = declarations_from(root)
    head = flatten(head_by_file)
    errors: List[str] = []

    if not args.skip_catalog:
        errors.extend(catalog_errors(root, head))

    if args.base:
        base_by_file = declarations_from(root, args.base)
        errors.extend(compare_abi(base_by_file, head_by_file))

    if args.report:
        report_path = Path(args.report)
        if not report_path.is_absolute():
            report_path = root / report_path
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(json.dumps(build_report(root, head), indent=2, sort_keys=True) + "\n", encoding="utf-8")

    unique_count = len({decl.key for decl in head})
    print(f"Persistent declarations: {len(head)} ({unique_count} unique keys)")
    if args.base:
        print(f"ABI base revision: {args.base}")
    if errors:
        for error in errors:
            print("ERROR: " + error, file=sys.stderr)
        return 1
    print("Settings audit passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
