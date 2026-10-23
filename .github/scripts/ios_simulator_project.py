"""Let the committed Xcode project build for the iOS simulator.

usage: python3 ios_simulator_project.py <project.pbxproj>

The app embeds a WatchKit 2 app (product types watchapp2 and watchkit2-extension), and Xcode
has no such product types for the "iOS Simulator" platform. The app target stops depending on
the watch app and stops embedding it; everything else stays as Xcode Cloud builds it.
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
