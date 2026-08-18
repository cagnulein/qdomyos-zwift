#!/usr/bin/env python3
from pathlib import Path

P = Path("src/settings.qml")
text = P.read_text(encoding="utf-8")

if "// MODERN_SETTINGS_LEGACY_HIERARCHY_V2" in text:
    print("Settings hierarchy navigation UI already applied")
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
        if ch == "{": depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = i + 1
                break
    if end is None:
        raise RuntimeError("unterminated function " + name)
    return source[:start] + replacement + source[end:]


text = text.replace('property string modernSettingsParent: ""', 'property string modernSettingsParent: ""\n        // MODERN_SETTINGS_LEGACY_HIERARCHY_V2')

text = replace_function(text, "rebuildModernSettingsCategories", '''function rebuildModernSettingsCategories() {
            var categories = []
            var hierarchy = settingsCatalog.legacyHierarchy || ({nodes: []})
            var nodes = hierarchy.nodes || []
            for (var i = 0; i < nodes.length; i++) {
                if (nodes[i].parent === null || nodes[i].parent === undefined || nodes[i].parent === "")
                    categories.push({key: nodes[i].key, name: nodes[i].name})
            }
            modernSettingsCategories = categories
            rebuildModernSettingsItems("")
        }''')

text = replace_function(text, "rebuildModernSettingsItems", '''function rebuildModernSettingsItems(query) {
            var normalized = (query || "").trim().toLowerCase()
            var items = []

            if (normalized.length > 0) {
                for (var s = 0; s < searchableSettings.length; s++) {
                    if (searchableText(searchableSettings[s]).indexOf(normalized) >= 0)
                        items.push(searchableSettings[s])
                }
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
                        target: "__category__:" + nodes[n].key
                    })
                }
            }

            for (var i = 0; i < searchableSettings.length; i++) {
                var entry = searchableSettings[i]
                if (settingsPane.modernEntryNodeKey(entry) === modernSettingsParent)
                    items.push(entry)
            }
            modernSettingsItems = items
        }''')

insert_anchor = '        function modernSettingsParentName() {'
helper = '''        function modernHierarchyNode(nodeKey) {
            var hierarchy = settingsCatalog.legacyHierarchy || ({nodes: []})
            var nodes = hierarchy.nodes || []
            for (var i = 0; i < nodes.length; i++) {
                if (nodes[i].key === nodeKey)
                    return nodes[i]
            }
            return null
        }

        function modernEntryNodeKey(entry) {
            var hierarchy = settingsCatalog.legacyHierarchy || ({})
            var mapping = hierarchy.settingNodeByKey || ({})
            if (entry.catalogKind === "virtual")
                mapping = hierarchy.virtualNodeByKey || ({})
            else if (entry.catalogKind === "page")
                mapping = hierarchy.pageNodeByKey || ({})
            return mapping[entry.key] || ""
        }

        function legacyRootIcon(name) {
            var lower = (name || "").toLowerCase()
            if (lower.indexOf("general") >= 0) return "⚙"
            if (lower.indexOf("heart") >= 0) return "♥"
            if (lower.indexOf("bike") >= 0) return "🚲"
            if (lower.indexOf("treadmill") >= 0 || lower.indexOf("running") >= 0) return "🏃"
            if (lower.indexOf("bluetooth") >= 0 || lower.indexOf("ant+") >= 0) return "⌁"
            if (lower.indexOf("peloton") >= 0 || lower.indexOf("zwift") >= 0) return "↔"
            if (lower.indexOf("advanced") >= 0) return "⚙"
            return "●"
        }

'''
if helper not in text:
    text = text.replace(insert_anchor, helper + insert_anchor)

text = replace_function(text, "modernSettingsParentName", '''function modernSettingsParentName() {
            if (modernSettingsParent.length === 0)
                return qsTr("Settings")
            var node = modernHierarchyNode(modernSettingsParent)
            return node ? node.name : qsTr("Settings")
        }''')

text = replace_function(text, "modernSettingsBack", '''function modernSettingsBack() {
            if (modernSettingsSearch.text.length > 0) {
                modernSettingsSearch.text = ""
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

text = text.replace('text: modelData.name\n                                font.pixelSize: Qt.application.font.pixelSize + 1',
                    'text: settingsPane.legacyRootIcon(modelData.name) + "  " + modelData.name\n                                font.pixelSize: Qt.application.font.pixelSize + 1', 1)

old1 = '''onClicked: {
                                        modernSettingsDrawer.close()
                                        stackView.push(entry.target)
                                    }'''
new1 = '''onClicked: {
                                        if (entry.target && entry.target.indexOf("__category__:") === 0) {
                                            settingsPane.openModernSettingsCategory(entry.target.substring("__category__:".length))
                                        } else {
                                            modernSettingsDrawer.close()
                                            stackView.push(entry.target)
                                        }
                                    }'''
text = text.replace(old1, new1)
text = text.replace('text: qsTr("Open")\n                                    onClicked:', 'text: entry.target && entry.target.indexOf("__category__:") === 0 ? "›" : qsTr("Open")\n                                    onClicked:')

old2 = 'onClicked: stackView.push(entry.target)'
new2 = 'onClicked: { if (entry.target && entry.target.indexOf("__category__:") === 0) settingsPane.openModernSettingsCategory(entry.target.substring("__category__:".length)); else stackView.push(entry.target) }'
text = text.replace(old2, new2)

P.write_text(text, encoding="utf-8")
print("Applied settings hierarchy navigation UI")
