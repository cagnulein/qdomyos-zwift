from pathlib import Path
import subprocess
import sys

branch = sys.argv[1]
settings_path = Path("src/settings.qml")
text = settings_path.read_text()

if text.count("contentWidth: -1") != 1:
    raise SystemExit(f"Expected exactly one ScrollView contentWidth guard, found {text.count('contentWidth: -1')}")
text = text.replace(
    "contentWidth: -1",
    "// Settings is a vertical page: never let a wide translated child enlarge the viewport.\n        contentWidth: availableWidth",
    1,
)

old_buttons = '''                    RowLayout {
                        Layout.fillWidth: true

                        Button {
                            id: clearLogs
                            text: qsTr("Clear History")
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            onClicked: rootItem.clearFiles();
                        }

                        Button {
                            text: qsTr("Show Logs Folder")
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            onClicked: {
                                toast.show(rootItem.getProfileDir())
                            }
                        }
                    }
'''
new_buttons = '''                    GridLayout {
                        id: logsButtonsLayout
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        columns: width >= clearLogs.implicitWidth + showLogs.implicitWidth + columnSpacing ? 2 : 1

                        Button {
                            id: clearLogs
                            text: qsTr("Clear History")
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            onClicked: rootItem.clearFiles();
                        }

                        Button {
                            id: showLogs
                            text: qsTr("Show Logs Folder")
                            Layout.fillWidth: true
                            Layout.minimumWidth: 0
                            onClicked: {
                                toast.show(rootItem.getProfileDir())
                            }
                        }
                    }
'''
if text.count(old_buttons) != 1:
    raise SystemExit(f"Expected exactly one log actions row, found {text.count(old_buttons)}")
text = text.replace(old_buttons, new_buttons, 1)
settings_path.write_text(text)

test_path = Path("tst/ToolTests/testsettingstestsuite.cpp")
test = test_path.read_text()
anchor = '''    EXPECT_LT(titlePosition - delegatePosition, 600);
}'''
replacement = '''    EXPECT_LT(titlePosition - delegatePosition, 600);

    // No translated control may enlarge the vertical settings page beyond the
    // visible viewport. This protects labels, buttons, combo boxes and future
    // controls with a large implicit width.
    EXPECT_TRUE(settingsSource.contains("contentWidth: availableWidth"));

    // Concrete regression case from the German UI: the two log buttons used to
    // make the entire settings page wider than a phone screen. Keep them in two
    // columns when they fit and stack them when the translated labels do not.
    EXPECT_TRUE(settingsSource.contains("id: logsButtonsLayout"));
    EXPECT_TRUE(settingsSource.contains(
        "columns: width >= clearLogs.implicitWidth + showLogs.implicitWidth + columnSpacing ? 2 : 1"));
    EXPECT_TRUE(settingsSource.contains("id: showLogs"));
}'''
if test.count(anchor) != 1:
    raise SystemExit(f"Expected one regression-test anchor, found {test.count(anchor)}")
test_path.write_text(test.replace(anchor, replacement, 1))

# Audit remaining multi-button RowLayouts. The global ScrollView guard prevents
# them from widening the whole page; this reports rows that may still benefit
# from local presentation changes in the future.
lines = settings_path.read_text().splitlines()
candidates = []
i = 0
while i < len(lines):
    if lines[i].strip() != "RowLayout {":
        i += 1
        continue
    start = i
    depth = lines[i].count("{") - lines[i].count("}")
    i += 1
    while i < len(lines) and depth > 0:
        depth += lines[i].count("{") - lines[i].count("}")
        i += 1
    block = "\n".join(lines[start:i])
    count = block.count("Button {")
    if count >= 2:
        candidates.append((start + 1, count))

print("Remaining RowLayout blocks containing 2+ Button controls:")
if not candidates:
    print("  none")
else:
    for line, count in candidates:
        print(f"  line {line}: {count} buttons")

subprocess.run(["git", "fetch", "origin", "master"], check=True)
subprocess.run(["git", "checkout", "origin/master", "--", ".github/workflows/settings-catalog.yml"], check=True)
for temporary in (
    Path(".github/workflows/patch-settings-responsive.yml"),
    Path(".github/scripts/apply-responsive-settings.py"),
    Path(".settings-layout-trigger"),
):
    temporary.unlink(missing_ok=True)

subprocess.run(["git", "config", "user.name", "Roberto Viola"], check=True)
subprocess.run(["git", "config", "user.email", "Cagnulein@gmail.com"], check=True)
subprocess.run(["git", "add", "-A"], check=True)
subprocess.run(["git", "commit", "-m", "fix: make settings layouts width-safe"], check=True)
subprocess.run(["git", "push", "origin", f"HEAD:{branch}"], check=True)
