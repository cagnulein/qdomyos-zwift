from pathlib import Path

path = Path("src/settings.qml")
text = path.read_text(encoding="utf-8")


def replace_once(old, new):
    global text
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"expected one match, found {count}: {old[:80]!r}")
    text = text.replace(old, new, 1)


replace_once(
'''        property string modernSettingsExternalParent: ""
        // MODERN_SETTINGS_LEGACY_HIERARCHY_V2
''',
'''        property string modernSettingsExternalParent: ""
        // MODERN_SETTINGS_NAVIGATION_STATE_V5
        property var modernSettingsHistory: []
        property bool modernSettingsAwaitingExternalReturn: false
        property var modernSettingsExternalReturnState: null

        StackView.onActivated: {
            if (modernSettingsAwaitingExternalReturn)
                Qt.callLater(function() { settingsPane.resumeModernSettingsAfterExternalPage() })
        }

        // MODERN_SETTINGS_LEGACY_HIERARCHY_V2
''')

replace_once(
'''        function openModernCatalogPage(entry) {
''',
'''        function modernCurrentScrollY() {
            if (modernSettingsParent.length === 0 && modernSettingsExternalTarget.length === 0 && modernSettingsSearch.text.length === 0)
                return modernCategoryList.contentY
            return modernItemList.contentY
        }

        function modernCaptureNavigationState() {
            return {
                parent: modernSettingsParent,
                externalTarget: modernSettingsExternalTarget,
                externalTitle: modernSettingsExternalTitle,
                externalParent: modernSettingsExternalParent,
                searchText: modernSettingsSearch.text,
                scrollY: modernCurrentScrollY()
            }
        }

        function modernPushNavigationState() {
            var history = modernSettingsHistory.slice(0)
            history.push(modernCaptureNavigationState())
            modernSettingsHistory = history
        }

        function modernRestoreScroll(state) {
            if (!state)
                return
            Qt.callLater(function() {
                var rootList = state.parent.length === 0 && state.externalTarget.length === 0 && state.searchText.length === 0
                if (rootList)
                    modernCategoryList.contentY = state.scrollY
                else
                    modernItemList.contentY = state.scrollY
            })
        }

        function modernResetCurrentScroll() {
            Qt.callLater(function() {
                if (modernSettingsParent.length === 0 && modernSettingsExternalTarget.length === 0 && modernSettingsSearch.text.length === 0)
                    modernCategoryList.positionViewAtBeginning()
                else
                    modernItemList.positionViewAtBeginning()
            })
        }

        function modernApplyNavigationState(state) {
            if (!state)
                return
            modernSettingsParent = state.parent || ""
            modernSettingsExternalTarget = state.externalTarget || ""
            modernSettingsExternalTitle = state.externalTitle || ""
            modernSettingsExternalParent = state.externalParent || ""
            modernSettingsSearch.text = state.searchText || ""
            rebuildModernSettingsItems(modernSettingsSearch.text)
            modernRestoreScroll(state)
        }

        function resumeModernSettingsAfterExternalPage() {
            if (!modernSettingsAwaitingExternalReturn)
                return
            var state = modernSettingsExternalReturnState
            modernSettingsAwaitingExternalReturn = false
            modernSettingsExternalReturnState = null
            modernApplyNavigationState(state)
            modernSettingsDrawer.open()
        }

        function openModernCatalogPage(entry) {
''')

replace_once(
'''            if (hasEntries) {
                modernSettingsExternalParent = modernSettingsParent
                modernSettingsExternalTarget = entry.target
                modernSettingsExternalTitle = entry.name || qsTr("Settings")
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
            } else {
                modernSettingsDrawer.close()
                stackView.push(entry.target)
            }
''',
'''            if (hasEntries) {
                modernPushNavigationState()
                modernSettingsExternalParent = modernSettingsParent
                modernSettingsExternalTarget = entry.target
                modernSettingsExternalTitle = entry.name || qsTr("Settings")
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
                modernResetCurrentScroll()
            } else {
                modernSettingsExternalReturnState = modernCaptureNavigationState()
                modernSettingsAwaitingExternalReturn = true
                modernSettingsDrawer.close()
                stackView.push(entry.target)
            }
''')

replace_once(
'''        function openModernSettingsPreview() {
            loadSettingsCatalog()
            modernSettingsParent = ""
            modernSettingsExternalTarget = ""
            modernSettingsExternalTitle = ""
            modernSettingsExternalParent = ""
            modernSettingsSearch.text = ""
            rebuildModernSettingsCategories()
            modernSettingsDrawer.open()
        }

        function openModernSettingsCategory(parentKey) {
            modernSettingsParent = parentKey
            modernSettingsSearch.text = ""
            rebuildModernSettingsItems("")
        }

        function modernSettingsBack() {
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
        }
''',
'''        function openModernSettingsPreview() {
            loadSettingsCatalog()
            modernSettingsHistory = []
            modernSettingsAwaitingExternalReturn = false
            modernSettingsExternalReturnState = null
            modernSettingsParent = ""
            modernSettingsExternalTarget = ""
            modernSettingsExternalTitle = ""
            modernSettingsExternalParent = ""
            modernSettingsSearch.text = ""
            rebuildModernSettingsCategories()
            modernResetCurrentScroll()
            modernSettingsDrawer.open()
        }

        function openModernSettingsCategory(parentKey) {
            modernPushNavigationState()
            modernSettingsParent = parentKey
            modernSettingsExternalTarget = ""
            modernSettingsExternalTitle = ""
            modernSettingsExternalParent = ""
            modernSettingsSearch.text = ""
            rebuildModernSettingsItems("")
            modernResetCurrentScroll()
        }

        function modernSettingsBack() {
            if (modernSettingsHistory.length > 0) {
                var history = modernSettingsHistory.slice(0)
                var state = history.pop()
                modernSettingsHistory = history
                modernApplyNavigationState(state)
                return
            }
            if (modernSettingsSearch.text.length > 0) {
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
                modernResetCurrentScroll()
                return
            }
            modernSettingsDrawer.close()
        }
''')

path.write_text(text, encoding="utf-8")
print("patched modern settings navigation state")
