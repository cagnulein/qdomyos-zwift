#!/usr/bin/env python3
import argparse
import html
import re
from pathlib import Path


def key_for(path: Path):
    m = re.match(r"(modern|legacy)-(.+?)-(\d+)\.png$", path.name)
    if not m:
        return None
    return m.group(1), m.group(2), int(m.group(3))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--raw", default="settings-qml-visual-report/raw")
    ap.add_argument("--out", default="settings-qml-visual-report")
    args = ap.parse_args()
    raw = Path(args.raw)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    groups = {}
    for p in sorted(raw.glob("*.png")):
        parsed = key_for(p)
        if not parsed:
            continue
        side, key, segment = parsed
        groups.setdefault(key, {"legacy": {}, "modern": {}})[side][segment] = p

    rows = []
    for key in sorted(groups):
        legacy = groups[key]["legacy"]
        modern = groups[key]["modern"]
        max_seg = max([*legacy.keys(), *modern.keys(), 0])
        for seg in range(max_seg + 1):
            l = legacy.get(seg)
            m = modern.get(seg)
            def img(p):
                if not p:
                    return '<div class="missing">not captured</div>'
                rel = p.relative_to(out) if out in p.parents else Path("raw") / p.name
                return f'<a href="{html.escape(str(rel))}"><img src="{html.escape(str(rel))}"></a>'
            rows.append(f"<tr><td>{html.escape(key)} #{seg}</td><td>{img(l)}</td><td>{img(m)}</td></tr>")

    index = f'''<!doctype html>
<meta charset="utf-8">
<title>QZ settings QML visual parity</title>
<style>
body{{font-family:-apple-system,BlinkMacSystemFont,Segoe UI,sans-serif;margin:20px;background:#f2f2f7;color:#111}}
h1{{margin-bottom:4px}} p{{color:#555}} table{{border-collapse:separate;border-spacing:0 12px;width:100%}}
th{{text-align:left}} td{{vertical-align:top;background:white;padding:12px}} td:first-child{{width:170px;font-weight:600}}
img{{width:min(100%,560px);height:auto;border:1px solid #ddd}} .missing{{padding:40px;color:#999;text-align:center}}
</style>
<h1>QZ settings QML visual parity</h1>
<p>Runtime screenshots from the real QZ Qt/QML application under Xvfb. Pixel equality is intentionally not enforced because the modern UI is expected to look different.</p>
<table><thead><tr><th>Section</th><th>Legacy</th><th>Modern</th></tr></thead><tbody>{''.join(rows)}</tbody></table>
'''
    (out / "index.html").write_text(index, encoding="utf-8")
    print(f"visual report: {len(groups)} sections, {sum(len(v['legacy']) + len(v['modern']) for v in groups.values())} screenshots")
    if not groups:
        raise SystemExit("no runtime screenshots found")

if __name__ == "__main__":
    main()
