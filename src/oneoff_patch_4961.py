#!/usr/bin/env python3
from pathlib import Path
import json
import shutil
import subprocess
import tempfile

BRANCH = "feature/custom-incline-resistance-table"
REPO = Path(__file__).resolve().parent.parent
WORKTREE = Path(tempfile.gettempdir()) / "qz-patch-4961"


def run(*args, cwd=REPO):
    print("+", " ".join(args), flush=True)
    subprocess.run(args, cwd=str(cwd), check=True)


# Work on the real feature head, never on GitHub's synthetic PR merge commit.
run("git", "fetch", "origin", BRANCH)
if WORKTREE.exists():
    shutil.rmtree(WORKTREE)
run("git", "worktree", "add", "--detach", str(WORKTREE), f"origin/{BRANCH}")

try:
    qml = WORKTREE / "src/settings.qml"
    text = qml.read_text()

    gear = '''                    NewPageElement {
                        title: qsTr("Custom Gear Table")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: "customgears.qml"
                    }
'''
    page = '''
                    NewPageElement {
                        title: qsTr("Custom Inclination to Resistance Table")
                        indicatRectColor: Material.color(Material.Grey)
                        textColor: Material.color(Material.Yellow)
                        color: Material.backgroundColor
                        accordionContent: "custominclinationresistance.qml"
                    }
'''
    if 'accordionContent: "custominclinationresistance.qml"' not in text:
        if text.count(gear) != 1:
            raise RuntimeError(f"Expected one Custom Gear Table page, found {text.count(gear)}")
        text = text.replace(gear, gear + page, 1)

    tail = '''            property bool simulate_moving_when_paused: false
            property bool kickr_core_3: false
        }
'''
    new_tail = '''            property bool simulate_moving_when_paused: false
            property bool kickr_core_3: false
            property bool custom_inclination_resistance_table_enabled: false
            property string custom_inclination_resistance_table: "0|4\\n1|6\\n2|8\\n3|10\\n4|11\\n5|11.5\\n6|12\\n8|13\\n10|14\\n12|15\\n15|16"
        }
'''
    if 'property bool custom_inclination_resistance_table_enabled' not in text:
        if text.count(tail) != 1:
            raise RuntimeError(f"Expected one Settings tail, found {text.count(tail)}")
        text = text.replace(tail, new_tail, 1)
    qml.write_text(text)

    catalog_path = WORKTREE / "src/settings-catalog.json"
    data = json.loads(catalog_path.read_text())

    if not any(p.get("key") == "page_custom_inclination_resistance_table" for p in data["pages"]):
        idx = next(i for i, p in enumerate(data["pages"]) if p.get("key") == "page_custom_gear_table")
        data["pages"].insert(idx + 1, {
            "key": "page_custom_inclination_resistance_table",
            "name": "Custom Inclination to Resistance Table",
            "description": "Configure how virtual-app inclination maps to bike resistance.",
            "parent": "Bike Options",
            "type": "page",
            "control": "page",
            "target": "custominclinationresistance.qml",
            "visible": True
        })

    keys = {s.get("key") for s in data["settings"]}
    if "custom_inclination_resistance_table_enabled" not in keys:
        data["settings"].append({
            "key": "custom_inclination_resistance_table_enabled",
            "name": "Enable Custom Inclination to Resistance Table",
            "description": "Use a custom mapping when QZ converts virtual-app inclination into resistance.",
            "parent": "Bike Options",
            "type": "boolean",
            "control": "switch",
            "defaultValue": False,
            "options": None,
            "visible": True
        })
    if "custom_inclination_resistance_table" not in keys:
        data["settings"].append({
            "key": "custom_inclination_resistance_table",
            "name": "Custom Inclination to Resistance Table",
            "description": "One inclination|resistance pair per line. QZ interpolates between points and clamps outside the configured range.",
            "parent": "custom_inclination_resistance_table_enabled",
            "type": "string",
            "control": "text",
            "defaultValue": "0|4\n1|6\n2|8\n3|10\n4|11\n5|11.5\n6|12\n8|13\n10|14\n12|15\n15|16",
            "options": None,
            "visible": True
        })
    data["settingCount"] = len(data["settings"])
    catalog_path.write_text(json.dumps(data, indent=2, ensure_ascii=True) + "\n")

    # Remove the one-shot mechanism from the final tree.
    src_pro = WORKTREE / "src/qdomyos-zwift.pro"
    src_pro.write_text('include(qdomyos-zwift.pri)\n\nQMAKE_IOS_DEPLOYMENT_TARGET = 12.0\nQMAKE_DEVELOPMENT_TEAM = 6335M7T29D\nQMAKE_CODE_SIGN_IDENTITY = "iPhone Developer"\nQMAKE_CODE_SIGN_STYLE = Automatic')

    root_pro = WORKTREE / "qdomyos-zwift.pro"
    root_pro.write_text('TEMPLATE = subdirs\nCONFIG+=ordered\n\n!ios: !android: {\nSUBDIRS = \\\n    src/qdomyos-zwift-lib.pro \\\n    src/qdomyos-zwift.pro \\\n    tst/qdomyos-zwift-tests.pro\n    \ntst.depends = src/qdomyos-zwift-lib.pro\n}\n\nandroid:  {\n    SUBDIRS = \\\n        src/qdomyos-zwift.pro\n}\n\nios: {\n    SUBDIRS = \\\n        src/qdomyos-zwift-lib.pro \\\n        src/qdomyos-zwift.pro\n    \n    # Team signing configuration\n    QMAKE_IOS_DEPLOYMENT_TARGET = 12.0\n    QMAKE_DEVELOPMENT_TEAM = 6335M7T29D\n    QMAKE_CODE_SIGN_IDENTITY = "iPhone Developer"\n    QMAKE_CODE_SIGN_STYLE = Automatic\n    \n    # Output directory configuration\n    DESTDIR = $$PWD/build-qdomyos-zwift-Qt_5_15_2_for_iOS-Debug\n}\n\n \n')

    helper = WORKTREE / "src/oneoff_patch_4961.py"
    if helper.exists():
        helper.unlink()

    run("git", "config", "user.name", "github-actions[bot]", cwd=WORKTREE)
    run("git", "config", "user.email", "41898282+github-actions[bot]@users.noreply.github.com", cwd=WORKTREE)
    run("git", "add", "-A", cwd=WORKTREE)
    run("git", "diff", "--cached", "--check", cwd=WORKTREE)

    changed = subprocess.run(["git", "diff", "--cached", "--quiet"], cwd=str(WORKTREE)).returncode != 0
    if changed:
        run("git", "commit", "-m", "Integrate custom incline table into Bike Options", cwd=WORKTREE)
        run("git", "push", "origin", f"HEAD:{BRANCH}", cwd=WORKTREE)
finally:
    subprocess.run(["git", "worktree", "remove", "--force", str(WORKTREE)], cwd=str(REPO), check=False)
