#!/usr/bin/env python3
from pathlib import Path
import json
import shutil
import subprocess
import tempfile

BRANCH = "feature/custom-incline-resistance-table"
REPO = Path(__file__).resolve().parent.parent
WORKTREE = Path(tempfile.gettempdir()) / "qz-fix-catalog-4961"
DEFAULT_TABLE_VALUE = "0|4\n1|6\n2|8\n3|10\n4|11\n5|11.5\n6|12\n8|13\n10|14\n12|15\n15|16"
DEFAULT_TABLE_EXPRESSION = '"0|4\\n1|6\\n2|8\\n3|10\\n4|11\\n5|11.5\\n6|12\\n8|13\\n10|14\\n12|15\\n15|16"'


def run(*args, cwd=REPO):
    print("+", " ".join(args), flush=True)
    subprocess.run(args, cwd=str(cwd), check=True)


run("git", "fetch", "origin", "master", BRANCH)
if WORKTREE.exists():
    shutil.rmtree(WORKTREE)
run("git", "worktree", "add", "--detach", str(WORKTREE), f"origin/{BRANCH}")

try:
    catalog_path = WORKTREE / "src/settings-catalog.json"
    data = json.loads(catalog_path.read_text())
    master_catalog = json.loads(subprocess.check_output(
        ["git", "show", "origin/master:src/settings-catalog.json"], cwd=str(WORKTREE), text=True
    ))

    data["settingCount"] = master_catalog["settingCount"] + 2

    replacements = {
        "custom_inclination_resistance_table_enabled": {
            "key": "custom_inclination_resistance_table_enabled",
            "name": "Enable Custom Inclination to Resistance Table",
            "description": "Use a custom mapping when QZ converts virtual-app inclination into resistance.",
            "parent": "Custom Inclination to Resistance Table",
            "type": "boolean",
            "qmlType": "bool",
            "control": "switch",
            "visible": True,
            "defaultValue": False,
            "defaultExpression": "false",
            "options": None
        },
        "custom_inclination_resistance_table": {
            "key": "custom_inclination_resistance_table",
            "name": "Custom Inclination to Resistance Table",
            "description": "One inclination|resistance pair per line. QZ interpolates between points and clamps outside the configured range.",
            "parent": "Custom Inclination to Resistance Table",
            "type": "string",
            "qmlType": "string",
            "control": "text",
            "visible": True,
            "defaultValue": DEFAULT_TABLE_VALUE,
            "defaultExpression": DEFAULT_TABLE_EXPRESSION,
            "options": None
        }
    }

    seen = set()
    for index, entry in enumerate(data["settings"]):
        key = entry.get("key")
        if key in replacements:
            data["settings"][index] = replacements[key]
            seen.add(key)
    missing = set(replacements) - seen
    if missing:
        raise RuntimeError(f"Missing custom catalog entries: {sorted(missing)}")

    catalog_path.write_text(json.dumps(data, indent=2, ensure_ascii=True) + "\n")

    root_pro = subprocess.check_output(
        ["git", "show", "origin/master:qdomyos-zwift.pro"], cwd=str(WORKTREE), text=True
    )
    (WORKTREE / "qdomyos-zwift.pro").write_text(root_pro)
    helper = WORKTREE / "src/oneoff_fix_catalog_4961.py"
    if helper.exists():
        helper.unlink()

    run("git", "config", "user.name", "github-actions[bot]", cwd=WORKTREE)
    run("git", "config", "user.email", "41898282+github-actions[bot]@users.noreply.github.com", cwd=WORKTREE)
    run("git", "add", "-A", cwd=WORKTREE)
    run("git", "diff", "--cached", "--check", cwd=WORKTREE)
    run("git", "commit", "-m", "Fix custom incline settings catalog metadata", cwd=WORKTREE)
    run("git", "push", "origin", f"HEAD:{BRANCH}", cwd=WORKTREE)
finally:
    subprocess.run(["git", "worktree", "remove", "--force", str(WORKTREE)], cwd=str(REPO), check=False)
