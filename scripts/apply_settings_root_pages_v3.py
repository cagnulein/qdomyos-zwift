#!/usr/bin/env python3
from pathlib import Path

layout = Path('scripts/settings_layout.py')
text = layout.read_text(encoding='utf-8')
if '"rootPages": sorted(root_pages' not in text:
    text = text.replace('    page_parent_target = {}\n\n    page_entries = catalog.get("pages") or []', '    page_parent_target = {}\n    root_pages = []\n\n    page_entries = catalog.get("pages") or []')
    text = text.replace('''        if best_source == "settings.qml":
            parent_id = best.get("parentAccordionId")
            if parent_id in node_keys:
                page_parent_node[entry["key"]] = parent_id
        else:
            page_parent_target[entry["key"]] = best_source
''', '''        if best_source == "settings.qml":
            parent_id = best.get("parentAccordionId")
            if parent_id in node_keys:
                page_parent_node[entry["key"]] = parent_id
            else:
                root_pages.append({
                    "key": entry["key"],
                    "name": entry.get("name") or best.get("title"),
                    "target": entry.get("target"),
                    "sourceLine": best["line"],
                })
        else:
            page_parent_target[entry["key"]] = best_source
''')
    text = text.replace('''        "pageParentTargetByKey": dict(sorted(page_parent_target.items())),
    }
''', '''        "pageParentTargetByKey": dict(sorted(page_parent_target.items())),
        "rootPages": sorted(root_pages, key=lambda item: item["sourceLine"]),
    }
''')
layout.write_text(text, encoding='utf-8')

qml = Path('src/settings.qml')
q = qml.read_text(encoding='utf-8')

def replace_function(source, name, replacement):
    needle = 'function ' + name + '('
    start = source.find(needle)
    if start < 0:
        raise RuntimeError('missing function ' + name)
    brace = source.find('{', start)
    depth = 0
    single = double = escaped = False
    for i in range(brace, len(source)):
        ch = source[i]
        if escaped:
            escaped = False
            continue
        if ch == '\\' and (single or double):
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
        if ch == '{': depth += 1
        elif ch == '}':
            depth -= 1
            if depth == 0:
                return source[:start] + replacement + source[i+1:]
    raise RuntimeError('unterminated function ' + name)

q = replace_function(q, 'rebuildModernSettingsCategories', '''function rebuildModernSettingsCategories() {
            var categories = []
            var hierarchy = settingsCatalog.legacyHierarchy || ({nodes: []})
            var nodes = hierarchy.nodes || []
            for (var i = 0; i < nodes.length; i++) {
                if (nodes[i].parent === null || nodes[i].parent === undefined || nodes[i].parent === "")
                    categories.push({key: nodes[i].key, name: nodes[i].name, legacySourceLine: nodes[i].sourceLine, catalogKind: "category"})
            }
            var layout = settingsCatalog.legacyLayout || ({rootPages: []})
            var rootPages = layout.rootPages || []
            for (var p = 0; p < rootPages.length; p++) {
                categories.push({
                    key: rootPages[p].key,
                    name: rootPages[p].name,
                    target: rootPages[p].target,
                    legacySourceLine: rootPages[p].sourceLine,
                    catalogKind: "page"
                })
            }
            categories.sort(function(a, b) { return a.legacySourceLine - b.legacySourceLine })
            modernSettingsCategories = categories
            rebuildModernSettingsItems("")
        }''')

old = 'onClicked: settingsPane.openModernSettingsCategory(modelData.key)'
new = '''onClicked: {
                                if (modelData.catalogKind === "page")
                                    settingsPane.openModernCatalogPage(modelData)
                                else
                                    settingsPane.openModernSettingsCategory(modelData.key)
                            }'''
if old in q:
    q = q.replace(old, new, 1)
elif new not in q:
    raise RuntimeError('root category click pattern not found')
qml.write_text(q, encoding='utf-8')
print('Added root NewPageElement entries to modern settings hierarchy')
