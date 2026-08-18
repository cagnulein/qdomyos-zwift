#!/usr/bin/env python3
from pathlib import Path

P = Path("src/settings.qml")
text = P.read_text(encoding="utf-8")

if "// MODERN_SETTINGS_IOS_GROUPED_V3" in text:
    print("Modern settings iOS grouped UI already applied")
    raise SystemExit(0)


def replace_function(source, name, replacement):
    needle = "function " + name + "("
    start = source.find(needle)
    if start < 0:
        raise RuntimeError("missing function " + name)
    brace = source.find("{", start)
    depth = 0
    single = double = escaped = False
    end = None
    for i in range(brace, len(source)):
        ch = source[i]
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
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                break
    if end is None:
        raise RuntimeError("unterminated function " + name)
    return source[:start] + replacement + source[end:]


text = text.replace(
    'property string modernSettingsParent: ""\n        // MODERN_SETTINGS_LEGACY_HIERARCHY_V2',
    'property string modernSettingsParent: ""\n        property string modernSettingsExternalTarget: ""\n        property string modernSettingsExternalTitle: ""\n        property string modernSettingsExternalParent: ""\n        // MODERN_SETTINGS_LEGACY_HIERARCHY_V2\n        // MODERN_SETTINGS_IOS_GROUPED_V3'
)

text = replace_function(text, "rebuildModernSettingsItems", '''function rebuildModernSettingsItems(query) {
            var normalized = (query || "").trim().toLowerCase()
            var items = []

            if (normalized.length > 0) {
                for (var s = 0; s < searchableSettings.length; s++) {
                    if (searchableText(searchableSettings[s]).indexOf(normalized) >= 0)
                        items.push(searchableSettings[s])
                }
                items.sort(function(a, b) { return modernItemOrder(a) - modernItemOrder(b) })
                modernSettingsItems = items
                return
            }

            if (modernSettingsExternalTarget.length > 0) {
                var layout = settingsCatalog.legacyLayout || ({})
                var sourceMap = layout.sourceFileByKey || ({})
                var persistent = settingsCatalog.settings || []
                for (var p = 0; p < persistent.length; p++) {
                    var pe = persistent[p]
                    if (!pe.visible || pe.control === "virtualOption")
                        continue
                    if (sourceMap[pe.key] !== modernSettingsExternalTarget)
                        continue
                    pe.catalogKind = "setting"
                    items.push(pe)
                }
                var pages = settingsCatalog.pages || []
                var pageParents = layout.pageParentTargetByKey || ({})
                for (var ep = 0; ep < pages.length; ep++) {
                    if (!pages[ep].visible || pageParents[pages[ep].key] !== modernSettingsExternalTarget)
                        continue
                    pages[ep].catalogKind = "page"
                    items.push(pages[ep])
                }
                items.sort(function(a, b) { return modernItemOrder(a) - modernItemOrder(b) })
                modernSettingsItems = items
                return
            }

            if (modernSettingsParent.length === 0) {
                modernSettingsItems = []
                return
            }

            var hierarchy = settingsCatalog.legacyHierarchy || ({nodes: []})
            var nodes = hierarchy.nodes || []
            for (var n = 0; n < nodes.length; n++) {
                if (nodes[n].parent === modernSettingsParent) {
                    items.push({
                        key: "__category__" + nodes[n].key,
                        name: nodes[n].name,
                        description: null,
                        catalogKind: "page",
                        target: "__category__:" + nodes[n].key,
                        legacySourceLine: nodes[n].sourceLine
                    })
                }
            }

            for (var i = 0; i < searchableSettings.length; i++) {
                var entry = searchableSettings[i]
                if (settingsPane.modernEntryNodeKey(entry) === modernSettingsParent)
                    items.push(entry)
            }
            items.sort(function(a, b) { return modernItemOrder(a) - modernItemOrder(b) })
            modernSettingsItems = items
        }''')

text = replace_function(text, "modernEntryNodeKey", '''function modernEntryNodeKey(entry) {
            var hierarchy = settingsCatalog.legacyHierarchy || ({})
            var layout = settingsCatalog.legacyLayout || ({})
            var mapping = hierarchy.settingNodeByKey || ({})
            if (entry.catalogKind === "virtual")
                mapping = hierarchy.virtualNodeByKey || ({})
            else if (entry.catalogKind === "page") {
                var layoutPages = layout.pageNodeByKey || ({})
                if (layoutPages[entry.key])
                    return layoutPages[entry.key]
                mapping = hierarchy.pageNodeByKey || ({})
            }
            return mapping[entry.key] || ""
        }''')

insert_anchor = '        function legacyRootIcon(name) {'
helpers = '''        function modernItemOrder(entry) {
            if (entry.legacySourceLine !== undefined)
                return entry.legacySourceLine
            var layout = settingsCatalog.legacyLayout || ({})
            if (entry.catalogKind === "page") {
                var pageOrder = layout.pageOrderByKey || ({})
                return pageOrder[entry.key] === undefined ? 999999 : pageOrder[entry.key]
            }
            var map = modernSettingsExternalTarget.length > 0 ? (layout.externalEntryOrderByKey || ({})) : (layout.itemOrderByKey || ({}))
            return map[entry.key] === undefined ? 999999 : map[entry.key]
        }

        function modernCardColor() {
            return Material.theme === Material.Dark ? "#2c2c2e" : "#ffffff"
        }

        function modernPageColor() {
            return Material.theme === Material.Dark ? "#1c1c1e" : "#f2f2f7"
        }

        function legacyRootIconColor(name) {
            var lower = (name || "").toLowerCase()
            if (lower.indexOf("general") >= 0) return "#8e8e93"
            if (lower.indexOf("heart") >= 0) return "#ff3b30"
            if (lower.indexOf("bike") >= 0) return "#34c759"
            if (lower.indexOf("treadmill") >= 0 || lower.indexOf("running") >= 0) return "#ff9500"
            if (lower.indexOf("ant+") >= 0 || lower.indexOf("bluetooth") >= 0) return "#007aff"
            if (lower.indexOf("peloton") >= 0) return "#ff2d55"
            if (lower.indexOf("zwift") >= 0) return "#ff9500"
            if (lower.indexOf("garmin") >= 0) return "#00a7e1"
            if (lower.indexOf("training") >= 0) return "#5856d6"
            if (lower.indexOf("advanced") >= 0 || lower.indexOf("experimental") >= 0) return "#8e8e93"
            return "#636366"
        }

        function openModernCatalogPage(entry) {
            if (!entry || !entry.target)
                return
            if (entry.target.indexOf("__category__:") === 0) {
                openModernSettingsCategory(entry.target.substring("__category__:".length))
                return
            }
            var layout = settingsCatalog.legacyLayout || ({})
            var sourceMap = layout.sourceFileByKey || ({})
            var hasEntries = false
            for (var key in sourceMap) {
                if (sourceMap[key] === entry.target) {
                    hasEntries = true
                    break
                }
            }
            var pageParents = layout.pageParentTargetByKey || ({})
            if (!hasEntries) {
                for (var pageKey in pageParents) {
                    if (pageParents[pageKey] === entry.target) {
                        hasEntries = true
                        break
                    }
                }
            }
            if (hasEntries) {
                modernSettingsExternalParent = modernSettingsParent
                modernSettingsExternalTarget = entry.target
                modernSettingsExternalTitle = entry.name || qsTr("Settings")
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
            } else {
                modernSettingsDrawer.close()
                stackView.push(entry.target)
            }
        }

'''
if helpers not in text:
    text = text.replace(insert_anchor, helpers + insert_anchor)

text = replace_function(text, "legacyRootIcon", '''function legacyRootIcon(name) {
            var lower = (name || "").toLowerCase()
            if (lower.indexOf("general") >= 0) return "⚙"
            if (lower.indexOf("heart") >= 0) return "♥"
            if (lower.indexOf("bike") >= 0) return "🚲"
            if (lower.indexOf("treadmill") >= 0 || lower.indexOf("running") >= 0) return "🏃"
            if (lower.indexOf("ant+") >= 0) return "⌁"
            if (lower.indexOf("peloton") >= 0) return "P"
            if (lower.indexOf("zwift") >= 0) return "Z"
            if (lower.indexOf("garmin") >= 0) return "G"
            if (lower.indexOf("training") >= 0) return "▶"
            if (lower.indexOf("advanced") >= 0) return "⚙"
            if (lower.indexOf("experimental") >= 0) return "⚗"
            return "•"
        }''')

text = replace_function(text, "modernSettingsParentName", '''function modernSettingsParentName() {
            if (modernSettingsExternalTarget.length > 0)
                return modernSettingsExternalTitle
            if (modernSettingsParent.length === 0)
                return qsTr("Settings")
            var node = modernHierarchyNode(modernSettingsParent)
            return node ? node.name : qsTr("Settings")
        }''')

text = replace_function(text, "openModernSettingsPreview", '''function openModernSettingsPreview() {
            loadSettingsCatalog()
            modernSettingsParent = ""
            modernSettingsExternalTarget = ""
            modernSettingsExternalTitle = ""
            modernSettingsExternalParent = ""
            modernSettingsSearch.text = ""
            rebuildModernSettingsCategories()
            modernSettingsDrawer.open()
        }''')

text = replace_function(text, "modernSettingsBack", '''function modernSettingsBack() {
            if (modernSettingsSearch.text.length > 0) {
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
                return
            }
            if (modernSettingsExternalTarget.length > 0) {
                modernSettingsExternalTarget = ""
                modernSettingsExternalTitle = ""
                modernSettingsParent = modernSettingsExternalParent
                modernSettingsExternalParent = ""
                rebuildModernSettingsItems("")
                return
            }
            if (modernSettingsParent.length > 0) {
                var node = modernHierarchyNode(modernSettingsParent)
                modernSettingsParent = node && node.parent ? node.parent : ""
                rebuildModernSettingsItems("")
                return
            }
            modernSettingsDrawer.close()
        }''')

# Make the modern drawer use an iOS-like grouped background.
text = text.replace(
    'id: modernSettingsDrawer\n            edge: Qt.RightEdge',
    'id: modernSettingsDrawer\n            background: Rectangle { color: settingsPane.modernPageColor() }\n            edge: Qt.RightEdge'
)

# Large left-aligned root title, compact centered title on subpages.
text = text.replace(
    'font.pixelSize: Qt.application.font.pixelSize + 5\n                            horizontalAlignment: Text.AlignHCenter',
    'font.pixelSize: Qt.application.font.pixelSize + (modernSettingsParent.length === 0 && modernSettingsExternalTarget.length === 0 ? 11 : 5)\n                            horizontalAlignment: modernSettingsParent.length === 0 && modernSettingsExternalTarget.length === 0 ? Text.AlignLeft : Text.AlignHCenter'
)

# Search field as a rounded iOS-style search box.
text = text.replace(
    'id: modernSettingsSearch\n                        Layout.fillWidth: true',
    'id: modernSettingsSearch\n                        Layout.fillWidth: true\n                        leftPadding: 14\n                        rightPadding: 14\n                        background: Rectangle { radius: 10; color: settingsPane.modernCardColor() }'
)

# Root list: grouped, compact rows rather than floating cards.
text = text.replace('id: modernCategoryList\n                    visible:', 'id: modernCategoryList\n                    visible:')
text = text.replace('clip: true\n                    spacing: 8\n                    leftMargin: 12\n                    rightMargin: 12\n                    topMargin: 12',
                    'clip: true\n                    spacing: 2\n                    leftMargin: 20\n                    rightMargin: 20\n                    topMargin: 18', 1)
text = text.replace('height: 56\n                        radius: 12\n                        color: Material.backgroundColor',
                    'height: 58\n                        radius: 10\n                        color: settingsPane.modernCardColor()', 1)

old_root_label = '''                            Label {
                                Layout.fillWidth: true
                                text: settingsPane.legacyRootIcon(modelData.name) + "  " + modelData.name
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                elide: Text.ElideRight
                            }'''
new_root_label = '''                            Rectangle {
                                Layout.preferredWidth: 34
                                Layout.preferredHeight: 34
                                radius: 8
                                color: settingsPane.legacyRootIconColor(modelData.name)
                                Label {
                                    anchors.centerIn: parent
                                    text: settingsPane.legacyRootIcon(modelData.name)
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: Qt.application.font.pixelSize + 2
                                }
                            }
                            Label {
                                Layout.fillWidth: true
                                text: modelData.name
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                elide: Text.ElideRight
                            }'''
if old_root_label not in text:
    raise RuntimeError("root category label pattern not found")
text = text.replace(old_root_label, new_root_label, 1)

# Modern setting rows get the same grouped-card background and tighter spacing.
text = text.replace('id: modernItemList\n                    visible:', 'id: modernItemList\n                    visible:')
# replace the next list spacing after modernItemList by scoped substring
idx = text.find('id: modernItemList')
if idx >= 0:
    tail = text[idx:]
    tail = tail.replace('spacing: 8', 'spacing: 2', 1)
    tail = tail.replace('radius: 12\n                        color: Material.backgroundColor', 'radius: 10\n                        color: settingsPane.modernCardColor()', 1)
    text = text[:idx] + tail

# Route both modern page buttons through the in-drawer catalog page renderer when possible.
old_block = '''onClicked: {
                                        if (entry.target && entry.target.indexOf("__category__:") === 0) {
                                            settingsPane.openModernSettingsCategory(entry.target.substring("__category__:".length))
                                        } else {
                                            modernSettingsDrawer.close()
                                            stackView.push(entry.target)
                                        }
                                    }'''
text = text.replace(old_block, 'onClicked: settingsPane.openModernCatalogPage(entry)')
old_inline = 'onClicked: { if (entry.target && entry.target.indexOf("__category__:") === 0) settingsPane.openModernSettingsCategory(entry.target.substring("__category__:".length)); else stackView.push(entry.target) }'
text = text.replace(old_inline, 'onClicked: settingsPane.openModernCatalogPage(entry)')

P.write_text(text, encoding="utf-8")
print("Applied modern settings iOS grouped UI v3")
