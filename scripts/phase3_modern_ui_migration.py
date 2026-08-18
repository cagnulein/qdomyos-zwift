#!/usr/bin/env python3
"""Install the catalog-driven modern settings preview into settings.qml.

The preview is opt-in and keeps the legacy renderer as the default. It deliberately reuses
existing catalog/search helpers and the shared behavior controller, so Phase 3 does not
introduce a second settings mutation implementation.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QML = ROOT / "src/settings.qml"
MARKER = "// MODERN_SETTINGS_PREVIEW_V1"

FUNCTIONS = r'''
        // MODERN_SETTINGS_PREVIEW_V1
        property var modernSettingsCategories: []
        property var modernSettingsItems: []
        property string modernSettingsParent: ""

        function rebuildModernSettingsCategories() {
            var categories = []
            var seen = ({})
            for (var i = 0; i < searchableSettings.length; i++) {
                var entry = searchableSettings[i]
                var key = entry.parent || "__general__"
                if (seen[key])
                    continue
                seen[key] = true
                categories.push({
                    key: key,
                    name: entry.parent ? parentDisplayName(entry) : qsTr("General")
                })
            }
            categories.sort(function(a, b) { return a.name.localeCompare(b.name) })
            modernSettingsCategories = categories
            rebuildModernSettingsItems("")
        }

        function rebuildModernSettingsItems(query) {
            var normalized = (query || "").trim().toLowerCase()
            var items = []
            for (var i = 0; i < searchableSettings.length; i++) {
                var entry = searchableSettings[i]
                var parentKey = entry.parent || "__general__"
                var parentMatches = modernSettingsParent.length === 0 || parentKey === modernSettingsParent
                var searchMatches = normalized.length === 0 || searchableText(entry).indexOf(normalized) >= 0
                if ((normalized.length > 0 || parentMatches) && searchMatches)
                    items.push(entry)
            }
            modernSettingsItems = items
        }

        function openModernSettingsPreview() {
            loadSettingsCatalog()
            modernSettingsParent = ""
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
            } else if (modernSettingsParent.length > 0) {
                modernSettingsParent = ""
                rebuildModernSettingsItems("")
            } else {
                modernSettingsDrawer.close()
            }
        }

'''

DRAWER = r'''
        Drawer {
            id: modernSettingsDrawer
            edge: Qt.RightEdge
            width: Math.min(settingsPane.width, 680)
            height: settingsPane.height
            modal: true
            interactive: true

            background: Rectangle {
                color: Material.backgroundColor
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: modernHeader.implicitHeight + 18
                    color: Material.backgroundColor

                    RowLayout {
                        id: modernHeader
                        anchors.fill: parent
                        anchors.margins: 9
                        spacing: 8

                        ToolButton {
                            text: modernSettingsParent.length > 0 || modernSettingsSearch.text.length > 0 ? "‹" : "×"
                            font.pixelSize: Qt.application.font.pixelSize + 8
                            onClicked: settingsPane.modernSettingsBack()
                        }

                        Label {
                            Layout.fillWidth: true
                            text: modernSettingsParent.length > 0 ?
                                      settingsPane.modernSettingsCategories.filter(function(item) { return item.key === modernSettingsParent })
                                          .map(function(item) { return item.name })[0] || qsTr("Settings") :
                                      qsTr("Settings")
                            font.bold: true
                            font.pixelSize: Qt.application.font.pixelSize + 5
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                        }

                        Item { Layout.preferredWidth: 44 }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: modernSettingsSearch.implicitHeight + 16
                    color: Material.backgroundColor

                    TextField {
                        id: modernSettingsSearch
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.margins: 8
                        placeholderText: qsTr("Search settings")
                        selectByMouse: true
                        inputMethodHints: Qt.ImhNoPredictiveText
                        onTextChanged: settingsPane.rebuildModernSettingsItems(text)
                    }
                }

                ListView {
                    id: modernCategoryList
                    visible: modernSettingsParent.length === 0 && modernSettingsSearch.text.length === 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 8
                    leftMargin: 12
                    rightMargin: 12
                    topMargin: 12
                    bottomMargin: 24
                    model: modernSettingsCategories

                    delegate: Rectangle {
                        width: modernCategoryList.width - modernCategoryList.leftMargin - modernCategoryList.rightMargin
                        height: 56
                        radius: 12
                        color: Material.dialogColor

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 12

                            Label {
                                Layout.fillWidth: true
                                text: modelData.name
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                elide: Text.ElideRight
                            }
                            Label {
                                text: "›"
                                color: Material.color(Material.Grey)
                                font.pixelSize: Qt.application.font.pixelSize + 8
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: settingsPane.openModernSettingsCategory(modelData.key)
                        }
                    }
                }

                ListView {
                    id: modernItemList
                    visible: modernSettingsParent.length > 0 || modernSettingsSearch.text.length > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 8
                    leftMargin: 12
                    rightMargin: 12
                    topMargin: 12
                    bottomMargin: 24
                    model: modernSettingsItems

                    delegate: Rectangle {
                        id: modernSettingRow
                        property var entry: modelData
                        width: modernItemList.width - modernItemList.leftMargin - modernItemList.rightMargin
                        implicitHeight: modernSettingContent.implicitHeight + 24
                        radius: 12
                        color: Material.dialogColor

                        ColumnLayout {
                            id: modernSettingContent
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.margins: 12
                            spacing: 6

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Label {
                                        Layout.fillWidth: true
                                        text: entry._translatedName || entry.name || entry.key
                                        font.bold: true
                                        wrapMode: Text.WordWrap
                                    }
                                    Label {
                                        Layout.fillWidth: true
                                        visible: modernSettingsSearch.text.length > 0
                                        text: settingsPane.parentDisplayName(entry)
                                        color: Material.color(Material.Grey)
                                        font.pixelSize: Qt.application.font.pixelSize - 2
                                        elide: Text.ElideRight
                                    }
                                }

                                Switch {
                                    visible: entry.catalogKind === "setting" && entry.type === "boolean"
                                    checked: visible ? settingsPane.settingValue(entry) : false
                                    onClicked: settingsPane.setSettingValue(entry, checked)
                                }

                                Button {
                                    visible: entry.catalogKind === "page"
                                    text: qsTr("Open")
                                    onClicked: {
                                        modernSettingsDrawer.close()
                                        stackView.push(entry.target)
                                    }
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                visible: entry.description !== null && entry.description !== undefined && entry.description.length > 0
                                text: entry.description || ""
                                color: Material.color(Material.Grey)
                                wrapMode: Text.WordWrap
                                font.pixelSize: Qt.application.font.pixelSize - 1
                            }

                            RowLayout {
                                visible: entry.catalogKind === "setting" && entry.type !== "boolean" && settingsPane.optionValues(entry).length === 0
                                Layout.fillWidth: true
                                spacing: 8
                                TextField {
                                    id: modernValueField
                                    Layout.fillWidth: true
                                    text: visible ? settingsPane.settingValue(entry) : ""
                                    horizontalAlignment: Text.AlignRight
                                    inputMethodHints: entry.type === "string" ? Qt.ImhNoPredictiveText : Qt.ImhFormattedNumbersOnly
                                    onAccepted: settingsPane.setSettingValue(entry, text)
                                }
                                Button {
                                    text: qsTr("Save")
                                    onClicked: settingsPane.setSettingValue(entry, modernValueField.text)
                                }
                            }

                            ComboBox {
                                id: modernOptionCombo
                                visible: entry.catalogKind === "setting" && settingsPane.optionValues(entry).length > 0
                                Layout.fillWidth: true
                                model: visible ? settingsPane.optionValues(entry) : []
                                currentIndex: visible ? settingsPane.optionIndex(entry) : 0
                                onActivated: {
                                    var selectedValue = currentValue
                                    if (entry.options && entry.options.expression && entry.options.expression.indexOf("bluetoothDevices") >= 0)
                                        selectedValue = settingsPane.stripRssi(selectedValue)
                                    settingsPane.setSettingValue(entry, selectedValue)
                                }
                            }

                            ComboBox {
                                id: modernVirtualCombo
                                visible: entry.catalogKind === "virtual"
                                Layout.fillWidth: true
                                model: visible ? settingsPane.virtualOptionLabels(entry) : []
                                currentIndex: visible ? settingsPane.virtualSelectedIndex(entry) : 0
                                onActivated: settingsPane.setVirtualSelection(entry, currentIndex)
                            }
                        }
                    }
                }
            }
        }

'''


def main() -> int:
    text = QML.read_text(encoding="utf-8")
    if MARKER in text:
        print("Modern settings preview already installed")
        return 0

    function_anchor = "        function showSettingsSearch() {"
    pos = text.find(function_anchor)
    if pos < 0:
        raise SystemExit("Cannot locate settings helper insertion point")
    text = text[:pos] + FUNCTIONS + text[pos:]

    drawer_anchor = "        ColumnLayout {\n            id: column1"
    pos = text.find(drawer_anchor)
    if pos < 0:
        raise SystemExit("Cannot locate legacy settings layout")
    text = text[:pos] + DRAWER + text[pos:]

    clear_block = '''                Button {\n                    text: qsTr("Clear")\n                    onClicked: settingsPane.hideSettingsSearch()\n                }'''
    if clear_block not in text:
        raise SystemExit("Cannot locate settings search Clear button")
    modern_button = '''                Button {\n                    text: qsTr("New UI")\n                    onClicked: settingsPane.openModernSettingsPreview()\n                }\n\n'''
    text = text.replace(clear_block, modern_button + clear_block, 1)

    # Keep the modern category model synchronized when async catalog loading completes.
    build_end = '''            updateFilteredSettings()\n        }'''
    replacement = '''            updateFilteredSettings()\n            rebuildModernSettingsCategories()\n        }'''
    if build_end not in text:
        raise SystemExit("Cannot locate buildSearchableSettings tail")
    text = text.replace(build_end, replacement, 1)

    QML.write_text(text, encoding="utf-8")
    print("Modern settings preview installed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
