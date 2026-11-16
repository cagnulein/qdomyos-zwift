"""Prepare the generated Xcode project for iOS app builds in CI.

usage: python3 ios_simulator_project.py <project.pbxproj>

The app embeds a WatchKit 2 app (product types watchapp2 and watchkit2-extension). Xcode cannot
resolve these targets while building the app for iOS or the iOS Simulator in this CI setup, so
the app target stops depending on and embedding the watch app. Everything else stays as Xcode
Cloud builds it.
"""
import sys

path = sys.argv[1]
# newline="": the line ends stay as they are
with open(path, encoding="utf-8", newline="") as f:
    lines = f.read().split("\n")

# The PBXTargetDependency blocks that point at the watch app target
watch_dependencies = []
block_id = None
for line in lines:
    text = line.strip()
    if text.endswith("/* PBXTargetDependency */ = {"):
        block_id = text.split(" ")[0]
    elif block_id and text.startswith("target = ") and text.endswith("/* watchkit */;"):
        watch_dependencies.append(block_id)
    elif text == "};":
        block_id = None

drop = set(dep + " /* PBXTargetDependency */," for dep in watch_dependencies)
kept = []
removed = []
for line in lines:
    text = line.strip()
    if text in drop or text.endswith("/* Embed Watch Content */,"):
        removed.append(text)
        continue
    kept.append(line)

if not watch_dependencies or len(removed) < 2:
    sys.exit("the watch app dependency or its embed phase was not found: " + str(removed))
with open(path, "w", encoding="utf-8", newline="") as f:
    f.write("\n".join(kept))
for text in removed:
    print("removed:", text)
