from pathlib import Path

path = Path("src/settings.qml")
text = path.read_text(encoding="utf-8")

old = '''        function modernSettingsBack() {
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
'''

new = '''        function modernSettingsBack() {
            if (modernSettingsSearch.text.length > 0) {
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
                modernResetCurrentScroll()
                return
            }
            if (modernSettingsHistory.length > 0) {
                var history = modernSettingsHistory.slice(0)
                var state = history.pop()
                modernSettingsHistory = history
                modernApplyNavigationState(state)
                return
            }
            modernSettingsDrawer.close()
        }
'''

if old in text:
    text = text.replace(old, new, 1)
    path.write_text(text, encoding="utf-8")
    print("updated modern settings back behavior")
elif new in text:
    print("modern settings back behavior already updated")
else:
    raise SystemExit("modernSettingsBack block not found")
