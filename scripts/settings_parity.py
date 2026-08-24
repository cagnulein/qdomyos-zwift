#!/usr/bin/env python3
"""Compare legacy QML control types with the catalog-driven settings UI.

The report is semantic rather than pixel-perfect: visual redesign is allowed,
but a legacy selector must not silently become a free-form text/number field.
It can also sync simple literal ComboBox models into the settings catalog.
"""
from __future__ import annotations

import argparse
import csv
import html
import json
import re
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / "src/settings-catalog.json"
QML_FILES = [
    ROOT / "src/settings.qml",
    ROOT / "src/settings-tiles.qml",
    ROOT / "src/settings-tts.qml",
    ROOT / "src/settings-shortcuts.qml",
    ROOT / "src/settings-treadmill-inclination-override.qml",
]
CONTROL_TYPES = {
    "ComboBox": "select",
    "Switch": "switch",
    "CheckBox": "switch",
    "AccordionCheckElement": "switch",
    "TextField": "text",
    "TextInput": "text",
    "SpinBox": "number",
    "Slider": "slider",
}
COMPONENT_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*\{")
SETTING_REF_RE = re.compile(r"\bsettings\.([A-Za-z_][A-Za-z0-9_]*)\b")


def strip_comments(line: str) -> str:
    return line.split("//", 1)[0]


def legacy_controls() -> tuple[dict[str, set[str]], dict[str, list[dict]]]:
    controls: dict[str, set[str]] = defaultdict(set)
    evidence: dict[str, list[dict]] = defaultdict(list)
    for path in QML_FILES:
        if not path.exists():
            continue
        stack: list[tuple[str, int]] = []
        depth = 0
        for lineno, raw in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            line = strip_comments(raw)
            m = COMPONENT_RE.match(line)
            if m:
                stack.append((m.group(1).split(".")[-1], depth))
            refs = SETTING_REF_RE.findall(line)
            if refs:
                enclosing = [name for name, _ in stack if name in CONTROL_TYPES]
                if enclosing:
                    ctl = CONTROL_TYPES[enclosing[-1]]
                    for key in refs:
                        controls[key].add(ctl)
                        evidence[key].append({
                            "file": path.name,
                            "line": lineno,
                            "component": enclosing[-1],
                            "control": ctl,
                        })
            depth += line.count("{") - line.count("}")
            while stack and depth <= stack[-1][1]:
                stack.pop()
    return controls, evidence


def component_blocks(text: str, component: str):
    pattern = re.compile(r"\b" + re.escape(component) + r"\s*\{")
    for match in pattern.finditer(text):
        open_pos = text.find("{", match.start())
        depth = 0
        in_string = False
        escaped = False
        quote = ""
        for pos in range(open_pos, len(text)):
            ch = text[pos]
            if in_string:
                if escaped:
                    escaped = False
                elif ch == "\\":
                    escaped = True
                elif ch == quote:
                    in_string = False
                continue
            if ch in ('"', "'"):
                in_string = True
                quote = ch
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    yield text[match.start():pos + 1]
                    break


def literal_combobox_options() -> dict[str, list[str]]:
    """Return settings key -> display labels for direct literal ComboBoxes.

    Matches controls such as:
      model: ["Disabled", "Gear Up", "Gear Down"]
      currentIndex: settings.zwiftplay_gear_lb
    """
    result: dict[str, list[str]] = {}
    for path in QML_FILES:
        if not path.exists():
            continue
        text = path.read_text(encoding="utf-8")
        for block in component_blocks(text, "ComboBox"):
            key_match = re.search(r"currentIndex\s*:\s*settings\.([A-Za-z_][A-Za-z0-9_]*)", block)
            model_match = re.search(r"model\s*:\s*\[([^\]]*)\]", block, re.S)
            if not key_match or not model_match:
                continue
            labels = re.findall(r'"((?:\\.|[^"\\])*)"', model_match.group(1))
            if not labels:
                continue
            labels = [bytes(label, "utf-8").decode("unicode_escape") if "\\" in label else label for label in labels]
            result[key_match.group(1)] = labels
    return result


def sync_literal_selects() -> int:
    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
    extracted = literal_combobox_options()
    changed = 0
    for section in ("settings", "virtualSettings"):
        for entry in catalog.get(section, []):
            key = entry.get("key")
            labels = extracted.get(key)
            if not labels:
                continue
            # Direct currentIndex bindings store the selected numeric index.
            if entry.get("type") not in ("integer", "number"):
                continue
            desired = {"values": list(range(len(labels))), "labels": labels}
            if entry.get("control") != "select" or entry.get("options") != desired:
                entry["control"] = "select"
                entry["options"] = desired
                changed += 1
    if changed:
        CATALOG.write_text(json.dumps(catalog, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    return changed


def modern_control(entry: dict) -> str:
    if entry.get("type") == "page" or entry.get("control") == "page":
        return "page"
    if entry.get("control") in ("select", "virtualOption"):
        return "select"
    options = entry.get("options")
    if options and ((isinstance(options, dict) and options.get("values") is not None) or isinstance(options, list)):
        return "select"
    if entry.get("type") == "boolean":
        return "switch"
    if entry.get("type") in ("integer", "number"):
        return "number"
    return "text"


def preferred_legacy(values: set[str]) -> str | None:
    for ctl in ("select", "switch", "number", "slider", "text"):
        if ctl in values:
            return ctl
    return None


def parity_status(entry: dict, legacy: str | None, modern: str) -> str:
    if legacy is None:
        return "unknown"
    if legacy == modern:
        return "ok"
    if {legacy, modern} <= {"text", "number"}:
        return "ok"
    if legacy == "slider" and modern in ("number", "text"):
        return "ok"

    # High-confidence failures only. An enclosing AccordionCheckElement can make
    # dependent numeric/text properties look like switches, so only trust switch
    # evidence for boolean catalog entries. ComboBox evidence is strong enough to
    # reject a free-form modern control.
    if legacy == "select" and modern != "select":
        return "mismatch"
    if legacy == "switch" and entry.get("type") == "boolean" and modern != "switch":
        return "mismatch"
    return "unknown"


def build_report() -> dict:
    catalog = json.loads(CATALOG.read_text(encoding="utf-8"))
    legacy, evidence = legacy_controls()
    rows = []
    mismatches = []
    for section in ("settings", "virtualSettings"):
        for entry in catalog.get(section, []):
            if not entry.get("visible", True) or entry.get("control") in ("internal", "virtualOption"):
                continue
            key = entry.get("key")
            lv = preferred_legacy(legacy.get(key, set()))
            mv = modern_control(entry)
            status = parity_status(entry, lv, mv)
            row = {
                "key": key,
                "name": entry.get("name", key),
                "parent": entry.get("parent") or "General",
                "legacy": lv or "unknown",
                "modern": mv,
                "status": status,
                "evidence": evidence.get(key, []),
            }
            rows.append(row)
            if status == "mismatch":
                mismatches.append(row)
    return {
        "settingCount": catalog.get("settingCount"),
        "visibleChecked": len(rows),
        "matched": sum(r["status"] == "ok" for r in rows),
        "mismatches": len(mismatches),
        "unknown": sum(r["status"] == "unknown" for r in rows),
        "rows": rows,
    }


def write_outputs(report: dict, out: Path) -> None:
    out.mkdir(parents=True, exist_ok=True)
    (out / "settings-parity.json").write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    with (out / "settings-parity.csv").open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["status", "parent", "key", "name", "legacy", "modern"])
        w.writeheader()
        for row in report["rows"]:
            w.writerow({k: row[k] for k in w.fieldnames})

    groups: dict[str, list[dict]] = defaultdict(list)
    for row in report["rows"]:
        groups[row["parent"]].append(row)
    sections = []
    for parent in sorted(groups):
        trs = []
        for r in groups[parent]:
            cls = r["status"]
            trs.append(
                f"<tr class='{cls}'><td>{html.escape(r['key'])}</td><td>{html.escape(r['name'])}</td>"
                f"<td>{r['legacy']}</td><td>{r['modern']}</td><td>{r['status']}</td></tr>"
            )
        sections.append(
            f"<h2>{html.escape(parent)}</h2><table><tr><th>Key</th><th>Name</th><th>Legacy</th>"
            f"<th>Modern</th><th>Status</th></tr>{''.join(trs)}</table>"
        )
    page = f"""<!doctype html><meta charset='utf-8'><title>QZ settings parity</title>
<style>body{{font:14px -apple-system,BlinkMacSystemFont,Segoe UI,sans-serif;margin:24px;background:#f2f2f7}}table{{border-collapse:collapse;width:100%;background:white;margin-bottom:28px}}th,td{{padding:8px 10px;border-bottom:1px solid #ddd;text-align:left}}.mismatch{{background:#ffe5e5}}.unknown{{background:#fff7d6}}h1{{margin-bottom:4px}}</style>
<h1>QZ Settings Parity</h1><p>{report['settingCount']} persistent settings · {report['visibleChecked']} visible checked · {report['matched']} matched · <b>{report['mismatches']} high-confidence mismatches</b> · {report['unknown']} not yet classified</p>{''.join(sections)}"""
    (out / "index.html").write_text(page, encoding="utf-8")

    try:
        from PIL import Image, ImageDraw, ImageFont
        font = ImageFont.load_default()
        imgdir = out / "contact-sheets"
        imgdir.mkdir(exist_ok=True)
        for parent, rows in groups.items():
            width = 1100
            height = 44 + 28 * min(len(rows), 120)
            image = Image.new("RGB", (width, height), "white")
            d = ImageDraw.Draw(image)
            d.text((12, 12), parent, fill="black", font=font)
            y = 42
            for r in rows[:120]:
                marker = "OK" if r["status"] == "ok" else ("!!" if r["status"] == "mismatch" else "??")
                text = f"{marker:2}  {r['key'][:38]:38}  legacy={r['legacy']:<8}  modern={r['modern']:<8}  {r['name'][:48]}"
                d.text((12, y), text, fill="black", font=font)
                y += 28
            safe = re.sub(r"[^A-Za-z0-9_.-]+", "_", parent).strip("_") or "General"
            image.save(imgdir / f"{safe}.png")
    except Exception as exc:
        (out / "contact-sheets-skipped.txt").write_text(str(exc), encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="settings-parity-report")
    ap.add_argument("--fail-on-mismatch", action="store_true")
    ap.add_argument("--write-literal-selects", action="store_true")
    args = ap.parse_args()
    if args.write_literal_selects:
        changed = sync_literal_selects()
        print(f"Synced literal ComboBox options for {changed} catalog entries")
    report = build_report()
    write_outputs(report, ROOT / args.out)
    print(json.dumps({k: report[k] for k in ("settingCount", "visibleChecked", "matched", "mismatches", "unknown")}, indent=2))
    if report["mismatches"]:
        print("MISMATCHES:")
        for r in report["rows"]:
            if r["status"] == "mismatch":
                print(f"  {r['key']}: legacy={r['legacy']} modern={r['modern']} ({r['parent']})")
    return 1 if args.fail_on_mismatch and report["mismatches"] else 0


if __name__ == "__main__":
    raise SystemExit(main())
