#!/usr/bin/env python3
from pathlib import Path

QML = Path(__file__).resolve().parents[1] / "src/settings.qml"
MARKER = "// MODERN_SETTINGS_PREVIEW_V1"


def main():
    text = QML.read_text(encoding="utf-8")
    if MARKER not in text:
        raise SystemExit("Modern settings preview is not installed")

    text = text.replace("color: Material.dialogColor", "color: Material.backgroundColor")

    old_title = '''                            text: modernSettingsParent.length > 0 ?\n                                      settingsPane.modernSettingsCategories.filter(function(item) { return item.key === modernSettingsParent })\n                                          .map(function(item) { return item.name })[0] || qsTr("Settings") :\n                                      qsTr("Settings")'''
    new_title = '''                            text: settingsPane.modernSettingsParentName()'''
    if old_title in text:
        text = text.replace(old_title, new_title, 1)

    if "function modernSettingsParentName()" not in text:
        anchor = "        function openModernSettingsPreview() {"
        helper = '''        function modernSettingsParentName() {\n            if (modernSettingsParent.length === 0)\n                return qsTr("Settings")\n            for (var i = 0; i < modernSettingsCategories.length; i++) {\n                if (modernSettingsCategories[i].key === modernSettingsParent)\n                    return modernSettingsCategories[i].name\n            }\n            return qsTr("Settings")\n        }\n\n'''
        if anchor not in text:
            raise SystemExit("Cannot locate modern settings preview helper anchor")
        text = text.replace(anchor, helper + anchor, 1)

    QML.write_text(text, encoding="utf-8")
    print("Modern settings UI final fixup applied")


if __name__ == "__main__":
    main()
