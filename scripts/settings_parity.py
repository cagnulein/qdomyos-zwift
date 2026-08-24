#!/usr/bin/env python3
"""Compare legacy QML control types with the catalog-driven settings UI.

Produces machine-readable JSON, CSV, HTML and PNG contact sheets.  The report is
intentionally semantic: visual redesign is allowed, but a legacy ComboBox must
not silently become a free-form text field in the modern renderer.
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
                        evidence[key].append({"file": path.name, "line": lineno, "component": enclosing[-1], "control": ctl})
            depth += line.count("{") - line.count("}")
            while stack and depth <= stack[-1][1]:
                stack.pop()
    return controls, evidence


def modern_control(entry: dict) -> str:
    if entry.get("type") == "page" or entry.get("control") == "page":
        return "page"
    if entry.get("control") in ("select", "virtualOption"):
        return "select"
    options = entry.get("options")
    if options and ((isinstance(options, dict) and options.get("values")) or isinstance(options, list)):
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
            status = "unknown" if lv is None else ("ok" if lv == mv or (lv == "slider" and mv == "number") else "mismatch")
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
            trs.append(f"<tr class='{cls}'><td>{html.escape(r['key'])}</td><td>{html.escape(r['name'])}</td><td>{r['legacy']}</td><td>{r['modern']}</td><td>{r['status']}</td></tr>")
        sections.append(f"<h2>{html.escape(parent)}</h2><table><tr><th>Key</th><th>Name</th><th>Legacy</th><th>Modern</th><th>Status</th></tr>{''.join(trs)}</table>")
    page = f"""<!doctype html><meta charset='utf-8'><title>QZ settings parity</title>
<style>body{{font:14px -apple-system,BlinkMacSystemFont,Segoe UI,sans-serif;margin:24px;background:#f2f2f7}}table{{border-collapse:collapse;width:100%;background:white;margin-bottom:28px}}th,td{{padding:8px 10px;border-bottom:1px solid #ddd;text-align:left}}.mismatch{{background:#ffe5e5}}.unknown{{background:#fff7d6}}h1{{margin-bottom:4px}}</style>
<h1>QZ Settings Parity</h1><p>{report['settingCount']} persistent settings · {report['visibleChecked']} visible checked · {report['matched']} matched · <b>{report['mismatches']} mismatches</b> · {report['unknown']} unknown</p>{''.join(sections)}"""
    (out / "index.html").write_text(page, encoding="utf-8")

    # Contact sheets are deliberately simple and dependency-light.  If Pillow is
    # available, make one PNG per category showing legacy vs modern controls.
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
    args = ap.parse_args()
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
