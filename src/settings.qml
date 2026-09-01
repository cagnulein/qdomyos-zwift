import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.0
import Qt.labs.settings 1.0
import QtQuick.Dialogs 1.0
import Qt.labs.platform 1.1
import AndroidStatusBar 1.0

//Page {
    ScrollView {
        objectName: "settingsPage"
        contentWidth: -1
        focus: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.fill: parent
        anchors.leftMargin: (Qt.platform.os === "android" && AndroidStatusBar.hasWaterfallDisplay) ?
                            AndroidStatusBar.waterfallLeftInset : 0
        anchors.rightMargin: (Qt.platform.os === "android" && AndroidStatusBar.hasWaterfallDisplay) ?
                             AndroidStatusBar.waterfallRightInset : 0
        //anchors.bottom: footerSettings.top
        //anchors.bottomMargin: footerSettings.height + 10
        id: settingsPane

        signal peloton_connect_clicked()

        property var settingsCatalog: ({ "settings": [], "virtualSettings": [], "pages": [] })
        property var searchableSettings: []
        property bool settingsCatalogLoaded: false
        property bool settingsCatalogLoading: false
        property string settingsCatalogError: ""


        // MODERN_SETTINGS_PREVIEW_V1
        property var modernSettingsCategories: []
        property var modernSettingsItems: []
        property string modernSettingsParent: ""
        property string modernSettingsExternalTarget: ""
        property string modernSettingsExternalTitle: ""
        property string modernSettingsExternalParent: ""
        property string modernSettingsPendingCategory: ""
        // MODERN_SETTINGS_NAVIGATION_STATE_V5
        property var modernSettingsHistory: []
        property bool modernSettingsAwaitingExternalReturn: false
        property var modernSettingsExternalReturnState: null

        StackView.onActivated: {
            if (modernSettingsAwaitingExternalReturn)
                Qt.callLater(function() { settingsPane.resumeModernSettingsAfterExternalPage() })
        }

        // MODERN_SETTINGS_LEGACY_HIERARCHY_V2
        // MODERN_SETTINGS_IOS_GROUPED_V3
        // MODERN_SETTINGS_ROOT_TEXT_ONLY_V4

        function rebuildModernSettingsCategories() {
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
            categories.push({
                key: "__settings_actions__",
                name: qsTr("Service Actions"),
                target: "__settings_actions__",
                legacySourceLine: 2147483647,
                catalogKind: "page"
            })
            categories.sort(function(a, b) { return a.legacySourceLine - b.legacySourceLine })
            modernSettingsCategories = categories
            rebuildModernSettingsItems("")
        }

        function rebuildModernSettingsItems(query) {
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
        }

        function modernHierarchyNode(nodeKey) {
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
        }

        function modernItemOrder(entry) {
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



        function modernCurrentScrollY() {
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
            modernSettingsDrawer.visible = true
        }

        function openModernCatalogPage(entry) {
            if (!entry || !entry.target)
                return
            if (entry.target === "__settings_actions__") {
                openSettingsActions()
                return
            }
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
                modernSettingsDrawer.visible = false
                stackView.push(entry.target)
            }
        }



        function modernSettingsParentName() {
            if (modernSettingsExternalTarget.length > 0)
                return modernSettingsExternalTitle
            if (modernSettingsParent.length === 0)
                return qsTr("Settings")
            var node = modernHierarchyNode(modernSettingsParent)
            return node ? node.name : qsTr("Settings")
        }

        function openModernSettingsPreview() {
            var pendingCategory = modernSettingsPendingCategory
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
            modernSettingsDrawer.visible = true
            if (pendingCategory.length > 0 && settingsCatalogLoaded && openModernSettingsCategoryByName(pendingCategory))
                modernSettingsPendingCategory = ""
        }

        function openModernSettingsCategoryByName(categoryName) {
            var nodes = (settingsCatalog.legacyHierarchy || {}).nodes || []
            for (var i = 0; i < nodes.length; i++) {
                if (nodes[i].name !== categoryName)
                    continue
                modernSettingsHistory = []
                modernSettingsParent = nodes[i].key
                modernSettingsExternalTarget = ""
                modernSettingsExternalTitle = ""
                modernSettingsExternalParent = ""
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
                modernResetCurrentScroll()
                return true
            }
            return false
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

        function modernSettingsBackFromHeader() {
            if (settingsActionsView.visible) {
                closeSettingsActions()
                return true
            }
            if (modernSettingsSearch.text.length > 0) {
                modernSettingsSearch.text = ""
                rebuildModernSettingsItems("")
                modernResetCurrentScroll()
                return true
            }
            if (modernSettingsHistory.length > 0) {
                var history = modernSettingsHistory.slice(0)
                var state = history.pop()
                modernSettingsHistory = history
                modernApplyNavigationState(state)
                return true
            }
            return false
        }

        function modernSettingsBack() {
            if (modernSettingsBackFromHeader())
                return
            stackView.pop()
        }

        function openGarminSection() {
            modernSettingsPendingCategory = "Garmin Options"
            modernSettingsDrawer.visible = true
            if (settingsCatalogLoaded && openModernSettingsCategoryByName(modernSettingsPendingCategory))
                modernSettingsPendingCategory = ""
            else
                loadSettingsCatalog()
        }

        function openSettingsActions() {
            modernSettingsDrawer.visible = false
            settingsActionsView.visible = true
            settingsActionsView.forceActiveFocus()
        }

        function closeSettingsActions() {
            settingsActionsView.visible = false
            modernSettingsDrawer.visible = true
        }

        // Strip the RSSI proximity suffix (e.g. " (75%)") before saving device names
        function stripRssi(deviceName) {
            return deviceName.replace(/ \(\d+%\)$/, "")
        }

        function loadSettingsCatalog() {
            if (settingsCatalogLoaded || settingsCatalogLoading)
                return

            settingsCatalogLoading = true
            settingsCatalogError = ""

            var xhr = new XMLHttpRequest()
            xhr.open("GET", "qrc:/settings-catalog.json")
            xhr.onreadystatechange = function() {
                if (xhr.readyState !== XMLHttpRequest.DONE)
                    return

                settingsCatalogLoading = false
                if (xhr.status === 200 || xhr.status === 0) {
                    try {
                        settingsCatalog = JSON.parse(xhr.responseText)
                        settingsCatalogLoaded = true
                        settingsCatalogError = ""
                        buildSearchableSettings()
                    } catch (e) {
                        settingsCatalogError = "Unable to parse settings catalog"
                        console.log(settingsCatalogError + ": " + e)
                    }
                } else {
                    settingsCatalogError = "Unable to load settings catalog"
                    console.log(settingsCatalogError + ": " + xhr.status)
                }
            }
            xhr.send()
        }

        function buildSearchableSettings() {
            var items = []
            var virtualSettings = settingsCatalog.virtualSettings || []
            var persistentSettings = settingsCatalog.settings || []
            var pages = settingsCatalog.pages || []

            for (var i = 0; i < virtualSettings.length; i++) {
                virtualSettings[i].catalogKind = "virtual"
                items.push(virtualSettings[i])
            }

            for (var j = 0; j < persistentSettings.length; j++) {
                if (persistentSettings[j].control === "virtualOption" ||
                    !persistentSettings[j].visible ||
                    settingsPane.isTileOrderSetting(persistentSettings[j]))
                    continue
                persistentSettings[j].catalogKind = "setting"
                items.push(persistentSettings[j])
            }

            for (var k = 0; k < pages.length; k++) {
                if (!pages[k].visible)
                    continue
                pages[k].catalogKind = "page"
                items.push(pages[k])
            }

            items.push({
                key: "__settings_actions__",
                name: qsTr("Service Actions"),
                description: qsTr("Authentication and device maintenance actions."),
                target: "__settings_actions__",
                legacySourceLine: 2147483647,
                catalogKind: "page"
            })

            for (var t = 0; t < items.length; t++)
                items[t]._translatedName = computeTranslatedName(items[t])

            searchableSettings = items
            rebuildModernSettingsCategories()
            if (modernSettingsPendingCategory.length > 0 && openModernSettingsCategoryByName(modernSettingsPendingCategory))
                modernSettingsPendingCategory = ""
        }

        function isTileOrderSetting(entry) {
            return entry && entry.key && entry.key.indexOf("tile_") === 0 && entry.key.lastIndexOf("_order") === entry.key.length - 6
        }


        // Try to find a translation for a catalog entry name by probing common QML contexts.
        // Returns the translated string if found, or the original name as fallback.
        function computeTranslatedName(entry) {
            var name = entry.name || entry.key
            if (!name) return name
            var contexts = [
                "settings", "settings-tiles", "settings-tts",
                "settings-shortcuts", "settings-treadmill-inclination-override",
                "homeform"
            ]
            for (var i = 0; i < contexts.length; i++) {
                // Try with trailing colon (common label pattern "Foo:")
                var withColon = qsTranslate(contexts[i], name + ":")
                if (withColon !== name + ":") return withColon.replace(/:$/, "").trim()
                // Try without colon
                var plain = qsTranslate(contexts[i], name)
                if (plain !== name) return plain
            }
            return name
        }

        function searchableText(entry) {
            var parts = [
                entry.key,
                entry.name,
                entry._translatedName,
                entry.description,
                entry.parent,
                parentDisplayName(entry),
                entry.type,
                entry.control,
                entry.target
            ]

            if (entry.options) {
                if (entry.options.values) {
                    for (var i = 0; i < entry.options.values.length; i++)
                        parts.push(entry.options.values[i])
                } else if (entry.options.length !== undefined) {
                    for (var j = 0; j < entry.options.length; j++) {
                        parts.push(entry.options[j].label)
                        parts.push(entry.options[j].sets)
                    }
                }
            }

            return parts.join(" ").toLowerCase()
        }

        function catalogEntryNameByKey(key) {
            var persistentSettings = settingsCatalog.settings || []
            for (var i = 0; i < persistentSettings.length; i++) {
                if (persistentSettings[i].key === key)
                    return persistentSettings[i].name || key
            }

            var virtualSettings = settingsCatalog.virtualSettings || []
            for (var j = 0; j < virtualSettings.length; j++) {
                if (virtualSettings[j].key === key)
                    return virtualSettings[j].name || key
            }

            return key
        }

        function parentDisplayName(entry) {
            if (!entry.parent)
                return qsTr("General")
            var name = catalogEntryNameByKey(entry.parent)
            return computeTranslatedName({name: name, key: entry.parent}) || name
        }


        // SETTINGS_BEHAVIOR_CONTROLLER_V1
        QtObject {
            id: settingsBehavior

            function restartRequired(entry) {
                return entry.restartRequired === undefined ? true : !!entry.restartRequired
            }

            function afterGenericWrite(entry) {
                if (restartRequired(entry))
                    window.settings_restart_to_apply = true
                toast.show("Setting saved!")
            }


            // SETTINGS_BEHAVIOR_PARITY_V2
            function rawValue(entry) {
                var value = settings[entry.key]
                return value === undefined ? entry.defaultValue : value
            }

            function twoDigits(value) {
                var rounded = Math.round(value)
                return rounded < 10 ? "0" + rounded : "" + rounded
            }

            function durationText(seconds) {
                var total = Math.max(0, Math.round(seconds))
                var hours = Math.floor(total / 3600)
                var minutes = Math.floor((total % 3600) / 60)
                var secs = total % 60
                return twoDigits(hours) + ":" + twoDigits(minutes) + ":" + twoDigits(secs)
            }

            function paceDistance(key) {
                if (key === "pacef_1mile") return 1.60934
                if (key === "pacef_5km") return 5
                if (key === "pacef_10km") return 10
                if (key === "pacef_halfmarathon") return 21
                if (key === "pacef_marathon") return 42
                return 0
            }

            function displayValue(entry) {
                var value = rawValue(entry)
                var key = entry.key

                if ((key === "weight" || key === "bike_weight") && settings.miles_unit && !settings.weight_kg_unit)
                    return value * 2.20462

                if (key === "height" && settings.miles_unit) {
                    var feet = Math.floor(value / 30.48)
                    var inches = Math.round((value % 30.48) / 2.54)
                    return feet + "'" + inches + '"'
                }

                if ((key === "autolap_distance" || key === "treadmill_speed_min" ||
                     key === "treadmill_speed_max" || key === "treadmill_step_speed" ||
                     key === "peloton_treadmill_running_min_speed" || key === "peloton_treadmill_walking_min_speed") && settings.miles_unit)
                    return value * 0.621371

                var distance = paceDistance(key)
                if (distance > 0)
                    return durationText(value * distance)

                return value
            }

            function setExclusive(keys, selectedKey, checked) {
                for (var i = 0; i < keys.length; i++)
                    settings[keys[i]] = keys[i] === selectedKey ? !!checked : false
            }

            function parseDuration(value) {
                var parts = ("" + value).split(":")
                if (parts.length !== 3)
                    return -1
                var h = parseInt(parts[0])
                var m = parseInt(parts[1])
                var s = parseInt(parts[2])
                if (isNaN(h) || isNaN(m) || isNaN(s) || m < 0 || m > 59 || s < 0 || s > 59)
                    return -1
                return h * 3600 + m * 60 + s
            }

            function setSettingValue(entry, value) {
                var key = entry.key

                if (key === "garmin_email" || key === "garmin_password" || key === "garmin_domain")
                    rootItem.garmin_connect_logout()

                if (key === "garmin_domain") {
                    settings.garmin_domain = ("" + value).indexOf("China") >= 0 ? "garmin.cn" : "garmin.com"
                    afterGenericWrite(entry)
                    return
                }

                if (key === "weight" || key === "bike_weight") {
                    var weightValue = parseFloat(value)
                    if (settings.miles_unit && !settings.weight_kg_unit)
                        weightValue = weightValue / 2.20462
                    settings[key] = weightValue
                    afterGenericWrite(entry)
                    return
                }

                if (key === "height") {
                    if (settings.miles_unit) {
                        var match = ("" + value).match(/(\d+)[\s''\u2018\u2019]*(\d+)/)
                        if (!match) {
                            toast.show(qsTr("Invalid format! Use feet'inches (e.g., 6'2\")"))
                            return
                        }
                        settings.height = parseInt(match[1]) * 30.48 + parseInt(match[2]) * 2.54
                    } else {
                        settings.height = parseFloat(value)
                    }
                    afterGenericWrite(entry)
                    return
                }

                if (key === "autolap_distance" || key === "treadmill_speed_min" || key === "treadmill_speed_max" || key === "treadmill_step_speed") {
                    var metricValue = parseFloat(value)
                    if (settings.miles_unit)
                        metricValue = metricValue * 1.60934
                    settings[key] = metricValue
                    afterGenericWrite(entry)
                    return
                }

                if (key === "peloton_treadmill_running_min_speed" || key === "peloton_treadmill_walking_min_speed") {
                    var pelotonSpeed = parseFloat(value)
                    if (settings.miles_unit)
                        pelotonSpeed = pelotonSpeed / 0.621371
                    settings[key] = pelotonSpeed
                    afterGenericWrite(entry)
                    return
                }

                var distance = paceDistance(key)
                if (distance > 0) {
                    var seconds = parseDuration(value)
                    if (seconds < 0) {
                        toast.show(qsTr("Invalid time format! Use hh:mm:ss"))
                        return
                    }
                    settings[key] = seconds / distance
                    afterGenericWrite(entry)
                    return
                }

                if (key === "domyos_bike_500_profile_v1" || key === "domyos_bike_500_profile_v2") {
                    setExclusive(["domyos_bike_500_profile_v1", "domyos_bike_500_profile_v2"], key, value)
                    afterGenericWrite(entry)
                    return
                }

                if (key === "kingsmith_encrypt_v2" || key === "kingsmith_encrypt_v3" || key === "kingsmith_encrypt_v4" ||
                    key === "kingsmith_encrypt_v5" || key === "kingsmith_encrypt_g1_walking_pad") {
                    setExclusive(["kingsmith_encrypt_v2", "kingsmith_encrypt_v3", "kingsmith_encrypt_v4", "kingsmith_encrypt_v5", "kingsmith_encrypt_g1_walking_pad"], key, value)
                    afterGenericWrite(entry)
                    return
                }

                if (key === "peloton_auto_start_with_intro" || key === "peloton_auto_start_without_intro") {
                    settings[key] = !!value
                    if (value)
                        settings[key === "peloton_auto_start_with_intro" ? "peloton_auto_start_without_intro" : "peloton_auto_start_with_intro"] = false
                    afterGenericWrite(entry)
                    return
                }

                if (key === "zwift_ocr" || key === "zwift_ocr_climb_portal" || key === "zwift_workout_ocr") {
                    setExclusive(["zwift_ocr", "zwift_ocr_climb_portal", "zwift_workout_ocr"], key, value)
                    settings.android_notification = true
                    afterGenericWrite(entry)
                    return
                }

                if (key === "zwift_play_emulator") {
                    if (!!value && !settings.zwift_play_emulator) {
                        if (settings.zwift_play || settings.zwift_click)
                            zwiftPlaySettingsDialog.visible = true
                        settings.watt_bike_emulator = false
                    }
                    settings.zwift_play_emulator = !!value
                    afterGenericWrite(entry)
                    return
                }

                if (key === "watt_bike_emulator") {
                    settings.watt_bike_emulator = !!value
                    if (value)
                        settings.zwift_play_emulator = false
                    afterGenericWrite(entry)
                    return
                }

                if (entry.type === "boolean") {
                    settings[key] = !!value
                } else if (entry.type === "integer") {
                    settings[key] = parseInt(value)
                } else if (entry.type === "number") {
                    settings[key] = parseFloat(value)
                } else {
                    settings[key] = value
                }

                if (key === "watt_offset" || key === "watt_gain" || key === "power_sensor_name") {
                    settings.treadmillDataPoints = ""
                    settings.ergDataPoints = ""
                }

                afterGenericWrite(entry)
            }

            function setVirtualSelection(entry, index) {
                if (!entry.options)
                    return

                for (var i = 0; i < entry.options.length; i++) {
                    if (entry.options[i].sets)
                        settings[entry.options[i].sets] = false
                }

                if (entry.options[index] && entry.options[index].sets)
                    settings[entry.options[index].sets] = true

                afterGenericWrite(entry)
            }
        }

        function settingValue(entry) {
            var value = settings[entry.key]
            return value === undefined ? entry.defaultValue : value
        }

        function displaySettingValue(entry) {
            return settingsBehavior.displayValue(entry)
        }

        function setSettingValue(entry, value) {
            settingsBehavior.setSettingValue(entry, value)
        }

        function optionValues(entry) {
            if (!entry.options)
                return []

            if (entry.options.values)
                return entry.options.values

            if (entry.options.expression && entry.options.expression.indexOf("rootItem.") === 0) {
                var propertyName = entry.options.expression.substring("rootItem.".length)
                if (typeof rootItem !== "undefined" && rootItem && rootItem[propertyName] !== undefined)
                    return rootItem[propertyName]
            }

            return []
        }

        function optionLabels(entry) {
            var values = optionValues(entry)
            if (entry.options && entry.options.labels && entry.options.labels.length === values.length)
                return entry.options.labels
            return values
        }

        function optionIndex(entry) {
            var values = optionValues(entry)
            var value = settingValue(entry)
            for (var i = 0; i < values.length; i++) {
                if (values[i] === value)
                    return i
            }
            return 0
        }

        function virtualOptionLabels(entry) {
            var labels = []
            for (var i = 0; entry.options && i < entry.options.length; i++)
                labels.push(entry.options[i].label)
            return labels
        }

        function virtualSelectedIndex(entry) {
            if (!entry.options)
                return 0

            for (var i = 0; i < entry.options.length; i++) {
                if (entry.options[i].sets && settings[entry.options[i].sets])
                    return i
            }
            return 0
        }

        function setVirtualSelection(entry, index) {
            settingsBehavior.setVirtualSelection(entry, index)
        }

        // always add a property at the end of the file to avoid corruption of the settings when loading old versions
        Settings {
            id: settings
            property real ui_zoom: 100.0
            property bool bike_heartrate_service: false
            property int bike_resistance_offset: 4
            property real bike_resistance_gain_f: 1.0
            property bool zwift_erg: false
            property real zwift_erg_filter: 10.0
            property real zwift_erg_filter_down: 10.0
            property bool zwift_negative_inclination_x2: false
            property real zwift_inclination_offset: 0
            property real zwift_inclination_gain: 1.0
            property real echelon_resistance_offset: 0
            property real echelon_resistance_gain: 1.0

            property bool speed_power_based: false
            property int bike_resistance_start: 1
            property int age: 35.0
            property real weight: 75.0
            property real ftp: 200.0
            property string user_email: ""
            property string user_nickname: ""
            property bool miles_unit: false
            property bool pause_on_start: false
            property bool treadmill_force_speed: false
            property bool pause_on_start_treadmill: false
            property bool continuous_moving: false
            property bool bike_cadence_sensor: false
            property bool run_cadence_sensor: false
            property bool bike_power_sensor: false
            property string heart_rate_belt_name: "Disabled"
            property bool heart_ignore_builtin: false
            property bool kcal_ignore_builtin: false

            property bool ant_cadence: false
            property bool ant_heart: false
            property bool ant_garmin: false

            property bool top_bar_enabled: true

            property string peloton_username: "username"
            property string peloton_password: "password"
            property string peloton_difficulty: "lower"
            property string peloton_cadence_metric: "Cadence"
            property string peloton_heartrate_metric: "Heart Rate"
            property string peloton_date: "Before Title"
            property bool peloton_description_link: true

            property string pzp_username: "username"
            property string pzp_password: "username"

            property bool tile_speed_enabled: true
            property int  tile_speed_order: 0
            property bool tile_inclination_enabled: true
            property int  tile_inclination_order: 1
            property bool tile_cadence_enabled: true
            property int  tile_cadence_order: 2
            property bool tile_elevation_enabled: true
            property int  tile_elevation_order: 3
            property bool tile_calories_enabled: true
            property int  tile_calories_order: 4
            property bool tile_odometer_enabled: true
            property int  tile_odometer_order: 5
            property bool tile_pace_enabled: true
            property int  tile_pace_order: 6
            property bool tile_resistance_enabled: true
            property int  tile_resistance_order: 7
            property bool tile_watt_enabled: true
            property int  tile_watt_order: 8
            property bool tile_weight_loss_enabled: false
            property int  tile_weight_loss_order: 24
            property bool tile_avgwatt_enabled: true
            property int  tile_avgwatt_order: 9
            property bool tile_ftp_enabled: true
            property int  tile_ftp_order: 10
            property bool tile_heart_enabled: true
            property int  tile_heart_order: 11
            property bool tile_fan_enabled: true
            property int  tile_fan_order: 12
            property bool tile_jouls_enabled: true
            property int  tile_jouls_order: 13
            property bool tile_elapsed_enabled: true
            property int  tile_elapsed_order: 14
            property bool tile_lapelapsed_enabled: false
            property int  tile_lapelapsed_order: 17
            property bool tile_moving_time_enabled: false
            property int  tile_moving_time_order: 21
            property bool tile_peloton_offset_enabled: false
            property int  tile_peloton_offset_order: 22
            property bool tile_peloton_difficulty_enabled: false
            property int  tile_peloton_difficulty_order: 32
            property bool tile_peloton_resistance_enabled: true
            property int  tile_peloton_resistance_order: 15
            property bool tile_datetime_enabled: true
            property int  tile_datetime_order: 16
            property bool tile_target_resistance_enabled: true
            property int  tile_target_resistance_order: 15
            property bool tile_target_peloton_resistance_enabled: false
            property int  tile_target_peloton_resistance_order: 21
            property bool tile_target_cadence_enabled: false
            property int  tile_target_cadence_order: 19
            property bool tile_target_power_enabled: false
            property int  tile_target_power_order: 20
            property bool tile_target_zone_enabled: false
            property int  tile_target_zone_order: 24
            property bool tile_target_speed_enabled: false
            property int  tile_target_speed_order: 27
            property bool tile_target_incline_enabled: false
            property int  tile_target_incline_order: 28
            property bool tile_strokes_count_enabled: false
            property int  tile_strokes_count_order: 22
            property bool tile_strokes_length_enabled: false
            property int  tile_strokes_length_order: 23
            property bool tile_watt_kg_enabled: false
            property int  tile_watt_kg_order: 25
            property bool tile_gears_enabled: false
            property int  tile_gears_order: 26
            property bool tile_remainingtimetrainprogramrow_enabled: false
            property int  tile_remainingtimetrainprogramrow_order: 27
            property bool tile_nextrowstrainprogram_enabled: false
            property int  tile_nextrowstrainprogram_order: 31
            property bool tile_mets_enabled: false
            property int  tile_mets_order: 28
            property bool tile_targetmets_enabled: false
            property int  tile_targetmets_order: 29
            property bool tile_steering_angle_enabled: false
            property int  tile_steering_angle_order: 30
            property bool tile_pid_hr_enabled: false
            property int  tile_pid_hr_order: 31

            property real heart_rate_zone1: 70.0
            property real heart_rate_zone2: 80.0
            property real heart_rate_zone3: 90.0
            property real heart_rate_zone4: 100.0
            property bool heart_max_override_enable: false
            property real heart_max_override_value: 195.0
            property int heart_rate_resting: 60

            property real peloton_gain: 1.0
            property real peloton_offset: 0

            property string treadmill_pid_heart_zone: "Disabled"
            property real pacef_1mile: 250
            property real pacef_5km: 300
            property real pacef_10km: 320
            property real pacef_halfmarathon: 340
            property real pacef_marathon: 360
            property string pace_default: "Half Marathon"

            property bool domyos_treadmill_buttons: false
            property bool domyos_treadmill_distance_display: true
            property bool domyos_treadmill_display_invert: false

            property real domyos_bike_cadence_filter: 0.0
            property bool domyos_bike_display_calories: true

            property real domyos_elliptical_speed_ratio: 1.0

            property bool eslinker_cadenza: true

            property string echelon_watttable: "Echelon"

            property real proform_wheel_ratio: 0.33
            property bool proform_tour_de_france_clc: false
            property bool proform_tdf_jonseed_watt: false
            property bool proform_studio: false
            property bool proform_tdf_10: false

            property double horizon_gr7_cadence_multiplier: 1.0

            property int  fitshow_user_id: 0x13AA

            property bool inspire_peloton_formula: false
            property bool inspire_peloton_formula2: false

            property bool hammer_racer_s: false

            property bool pafers_treadmill: false

            property bool yesoul_peloton_formula: false

            property bool nordictrack_10_treadmill: true
            property bool nordictrack_t65s_treadmill: false
            //property bool proform_treadmill_995i: false

            property bool toorx_3_0: false
            property bool toorx_65s_evo: false
            property bool jtx_fitness_sprint_treadmill: false
            property bool dkn_endurun_treadmill: false
            property bool trx_route_key: false
            property bool bh_spada_2: false
            property bool toorx_bike: false
            property bool toorx_ftms: false
            property bool jll_IC400_bike: false
            property bool fytter_ri08_bike: false
            property bool asviva_bike: false
            property bool hertz_xr_770: false

            property int  m3i_bike_id: 256
            property int  m3i_bike_speed_buffsize: 90
            property bool m3i_bike_qt_search: false
            property bool m3i_bike_kcal: true

            property bool snode_bike: false
            property bool fitplus_bike: false
            property bool virtufit_etappe: false

            property int flywheel_filter: 2
            property bool flywheel_life_fitness_ic8: false

            property bool sole_treadmill_inclination: false
            property bool sole_treadmill_miles: true
            property bool sole_treadmill_f65: false
            property bool sole_treadmill_f63: false
            property bool sole_treadmill_tt8: false

            property bool schwinn_bike_resistance: false
            property bool schwinn_bike_resistance_v2: value

            property bool technogym_myrun_treadmill_experimental: false

            property bool trainprogram_random: false
            property int trainprogram_total: 60
            property real trainprogram_period_seconds: 60
            property real trainprogram_speed_min: 8
            property real trainprogram_speed_max: 16
            property real trainprogram_incline_min: 0
            property real trainprogram_incline_max: 15
            property real trainprogram_resistance_min: 1
            property real trainprogram_resistance_max: 32

            property real watt_offset: 0
            property real watt_gain: 1
            property bool power_avg_5s: false
            property bool instant_power_on_pause: false

            property real speed_offset: 0
            property real speed_gain: 1

            property string filter_device: "Disabled"
            property string strava_suffix: "#QZ"

            property string cadence_sensor_name: "Disabled"
            property bool cadence_sensor_as_bike: false
            property real cadence_sensor_speed_ratio: 0.33
            property real power_hr_pwr1: 200
            property real power_hr_hr1: 150
            property real power_hr_pwr2: 230
            property real power_hr_hr2: 170

            property string power_sensor_name: "Disabled"
            property bool power_sensor_as_bike: false
            property bool power_sensor_as_treadmill: false
            property bool powr_sensor_running_cadence_double: false

            property string elite_rizer_name: "Disabled"
            property string elite_sterzo_smart_name: "Disabled"

            property string ftms_accessory_name: "Disabled"
            property real ss2k_shift_step: 900

            property bool fitmetria_fanfit_enable: false
            property string fitmetria_fanfit_mode: "Heart"
            property real fitmetria_fanfit_min: 0
            property real fitmetria_fanfit_max: 100

            property bool virtualbike_forceresistance: true
            property bool bluetooth_relaxed: false
            property bool bluetooth_30m_hangs: false
            property bool battery_service: false
            property bool service_changed: false
            property bool virtual_device_enabled: true
            property bool virtual_device_bluetooth: true
            property bool ios_peloton_workaround: true
            property bool android_wakelock: true
            property bool log_debug: false
            property bool virtual_device_onlyheart: false
            property bool virtual_device_echelon: false
            property bool virtual_device_ifit: false
            property bool virtual_device_rower: false
            property bool virtual_device_force_bike: false
            property bool volume_change_gears: false
            property bool applewatch_fakedevice: false

            // from version 2.10.15
            property real zwift_erg_resistance_down: 0.0
            property real zwift_erg_resistance_up: 999.0

            // from version 2.10.16
            property bool horizon_paragon_x: false

            // from version 2.10.18
            property real treadmill_step_speed: 0.5
            property real treadmill_step_incline: 0.5

            // from version 2.10.19
            property bool  fitshow_anyrun: false

            // from version 2.10.21
            property bool nordictrack_s30_treadmill: false

            // from version 2.10.23
            // not used anymore because it's an elliptical not a treadmill. Don't remove this
            // it will cause corruption in the settings
            property bool nordictrack_fs5i_treadmill: false

            // from version 2.10.26
            property bool renpho_peloton_conversion_v2: false

            // from version 2.10.27
            property real ss2k_resistance_sample_1: 20
            property real ss2k_shift_step_sample_1: 0
            property real ss2k_resistance_sample_2: 30
            property real ss2k_shift_step_sample_2: 0
            property real ss2k_resistance_sample_3: 40
            property real ss2k_shift_step_sample_3: 0
            property real ss2k_resistance_sample_4: 50
            property real ss2k_shift_step_sample_4: 0

            property bool  fitshow_truetimer: false

            // from version 2.10.28
            property real elite_rizer_gain: 1.0
            property bool tile_ext_incline_enabled: false
            property int  tile_ext_incline_order: 32

            // from version 2.10.41
            property bool reebok_fr30_treadmill: false

            // from version 2.10.44
            property bool horizon_treadmill_7_8: false

            // from version 2.10.45
            property string profile_name: "default"

            // from version 2.10.46
            property bool tile_cadence_color_enabled: false
            property bool tile_peloton_remaining_enabled: false
            property int  tile_peloton_remaining_order: 22
            property bool tile_peloton_resistance_color_enabled: false

            // from version 2.10.49
            property bool dircon_yes: true
            property int dircon_server_base_port: 36866

            // from version 2.10.56
            property bool ios_cache_heart_device: true

            // from version 2.10.57
            property int app_opening: 0

            // from version 2.10.62
            property string proformtdf4ip: ""

            // from version 2.10.72
            property bool fitfiu_mc_v460: false
            property real bike_weight: 0

            // from version 2.10.77
            property bool kingsmith_encrypt_v2: false

            // from version 2.10.81
            property bool proform_treadmill_9_0: false

            // from version 2.10.85
            property bool proform_treadmill_1800i: false

            // from version 2.10.91
            property real cadence_offset: 0
            property real cadence_gain: 1
            property bool sp_ht_9600ie: false

            // from version 2.10.92
            property bool tts_enabled: false
            property int tts_summary_sec: 120
            property bool tts_act_speed: false
            property bool tts_avg_speed: true
            property bool tts_max_speed: false
            property bool tts_act_inclination: false
            property bool tts_act_cadence: false
            property bool tts_avg_cadence: true
            property bool tts_max_cadence: false
            property bool tts_act_elevation: true
            property bool tts_act_calories: true
            property bool tts_act_odometer: true
            property bool tts_act_pace: false
            property bool tts_avg_pace: true
            property bool tts_max_pace: false
            property bool tts_act_resistance: true
            property bool tts_avg_resistance: true
            property bool tts_max_resistance: false
            property bool tts_act_watt: false
            property bool tts_avg_watt: true
            property bool tts_max_watt: true
            property bool tts_act_ftp: false
            property bool tts_avg_ftp: true
            property bool tts_max_ftp: false
            property bool tts_act_heart: true
            property bool tts_avg_heart: true
            property bool tts_max_heart: false
            property bool tts_act_jouls: true
            property bool tts_act_elapsed: true
            property bool tts_act_peloton_resistance: false
            property bool tts_avg_peloton_resistance: false
            property bool tts_max_peloton_resistance: false
            property bool tts_act_target_peloton_resistance: true
            property bool tts_act_target_cadence: true
            property bool tts_act_target_power: true
            property bool tts_act_target_zone: true
            property bool tts_act_target_speed: true
            property bool tts_act_target_incline: true
            property bool tts_act_watt_kg: false
            property bool tts_avg_watt_kg: false
            property bool tts_max_watt_kg: false

            // from version 2.10.96
            property bool fakedevice_elliptical: false

            // from version 2.10.99
            property string nordictrack_2950_ip: ""

            // from version 2.10.102
            property bool tile_instantaneous_stride_length_enabled: false
            property int  tile_instantaneous_stride_length_order: 32
            property bool tile_ground_contact_enabled: false
            property int  tile_ground_contact_order: 33
            property bool tile_vertical_oscillation_enabled: false
            property int  tile_vertical_oscillation_order: 34
            property string sex: "Male"

            // from version 2.10.111
            property string maps_type: "3D"

            // from version 2.10.112
            property real ss2k_max_resistance: 100
            property real ss2k_min_resistance: 0

            // from version 2.11.10
            property bool proform_treadmill_se: false

            // from version 2.11.14
            property string proformtreadmillip: ""

            // from version 2.11.22
            property bool kingsmith_encrypt_v3: false

            // from version 2.11.38
            property string tdf_10_ip: ""

            // from version 2.11.41
            property bool fakedevice_treadmill: false

            // from version 2.11.43
            property int video_playback_window_s: 12 // not used

            // from version 2.11.62
            property string horizon_treadmill_profile_user1: "user1"
            property string horizon_treadmill_profile_user2: "user2"
            property string horizon_treadmill_profile_user3: "user3"
            property string horizon_treadmill_profile_user4: "user4"
            property string horizon_treadmill_profile_user5: "user5"

            // from version 2.11.63
            property bool nordictrack_gx_2_7: false

            // from version 2.11.65
            property real rolling_resistance: 0.005

            // from version 2.11.67
            property bool eslinker_ypoo: false

            // from version 2.11.69
            property bool wahoo_rgt_dircon: false            

            // from version 2.11.73
            property bool tts_description_enabled: true

            // from version 2.11.80
            property bool tile_preset_resistance_1_enabled: false
            property int tile_preset_resistance_1_order: 33
            property real tile_preset_resistance_1_value: 1.0
            property string tile_preset_resistance_1_label: "Res. 1"
            property bool tile_preset_resistance_2_enabled: false
            property int tile_preset_resistance_2_order: 34
            property real tile_preset_resistance_2_value: 10.0
            property string tile_preset_resistance_2_label: "Res. 10"
            property bool tile_preset_resistance_3_enabled: false
            property int tile_preset_resistance_3_order: 35
            property real tile_preset_resistance_3_value: 20.0
            property string tile_preset_resistance_3_label: "Res. 20"
            property bool tile_preset_resistance_4_enabled: false
            property int tile_preset_resistance_4_order: 36
            property real tile_preset_resistance_4_value: 25.0
            property string tile_preset_resistance_4_label: "Res. 25"
            property bool tile_preset_resistance_5_enabled: false
            property int tile_preset_resistance_5_order: 37
            property real tile_preset_resistance_5_value: 30.0
            property string tile_preset_resistance_5_label: "Res. 30"
            property bool tile_preset_speed_1_enabled: false
            property int tile_preset_speed_1_order: 38
            property real tile_preset_speed_1_value: 5.0
            property string tile_preset_speed_1_label: "5 km/h"
            property bool tile_preset_speed_2_enabled: false
            property int tile_preset_speed_2_order: 39
            property real tile_preset_speed_2_value: 7.0
            property string tile_preset_speed_2_label: "7 km/h"
            property bool tile_preset_speed_3_enabled: false
            property int tile_preset_speed_3_order: 40
            property real tile_preset_speed_3_value: 10.0
            property string tile_preset_speed_3_label: "10 km/h"
            property bool tile_preset_speed_4_enabled: false
            property int tile_preset_speed_4_order: 41
            property real tile_preset_speed_4_value: 11.0
            property string tile_preset_speed_4_label: "11 km/h"
            property bool tile_preset_speed_5_enabled: false
            property int tile_preset_speed_5_order: 42
            property real tile_preset_speed_5_value: 12.0
            property string tile_preset_speed_5_label: "12 km/h"
            property bool tile_preset_inclination_1_enabled: false
            property int tile_preset_inclination_1_order: 43
            property real tile_preset_inclination_1_value: 0.0
            property string tile_preset_inclination_1_label: "0%"
            property bool tile_preset_inclination_2_enabled: false
            property int tile_preset_inclination_2_order: 44
            property real tile_preset_inclination_2_value: 1.0
            property string tile_preset_inclination_2_label: "1%"
            property bool tile_preset_inclination_3_enabled: false
            property int tile_preset_inclination_3_order: 45
            property real tile_preset_inclination_3_value: 2.0
            property string tile_preset_inclination_3_label: "2%"
            property bool tile_preset_inclination_4_enabled: false
            property int tile_preset_inclination_4_order: 46
            property real tile_preset_inclination_4_value: 3.0
            property string tile_preset_inclination_4_label: "3%"
            property bool tile_preset_inclination_5_enabled: false
            property int tile_preset_inclination_5_order: 47
            property real tile_preset_inclination_5_value: 4.0
            property string tile_preset_inclination_5_label: "4%"

            // from version 2.11.85
				property string tile_preset_resistance_1_color: "grey"
				property string tile_preset_resistance_2_color: "grey"
				property string tile_preset_resistance_3_color: "grey"
				property string tile_preset_resistance_4_color: "grey"
				property string tile_preset_resistance_5_color: "grey"
				property string tile_preset_speed_1_color: "grey"
				property string tile_preset_speed_2_color: "grey"
				property string tile_preset_speed_3_color: "grey"
				property string tile_preset_speed_4_color: "grey"
				property string tile_preset_speed_5_color: "grey"
				property string tile_preset_inclination_1_color: "grey"
				property string tile_preset_inclination_2_color: "grey"
				property string tile_preset_inclination_3_color: "grey"
				property string tile_preset_inclination_4_color: "grey"
				property string tile_preset_inclination_5_color: "grey"

            property bool tile_avg_watt_lap_enabled: false
				property int tile_avg_watt_lap_order: 48

            // from version 2.11.87
            property bool nordictrack_t70_treadmill: false

            // from version 2.11.94
            property real crrGain: 0
            property real cwGain: 0

            // from version 2.12.1
            property bool proform_treadmill_cadence_lt: false

            // from version 2.12.3
            property bool trainprogram_stop_at_end: false

            // from version 2.12.5
            property bool domyos_elliptical_inclination: true
            property bool gpx_loop: false

            // from version 2.12.6
            property bool android_notification: false

            // from version 2.12.8
            property bool kingsmith_encrypt_v4: false

            // from versiomn 2.12.11
            property bool horizon_treadmill_disable_pause: false

            // from version 2.12.13
            property bool domyos_bike_500_profile_v1: false

            // from version 2.12.14
            property bool ss2k_peloton: false

            // from version 2.12.16
            property string computrainer_serialport: ""

            // from version 2.12.18
            property bool strava_virtual_activity: true

            // from version 2.12.29
            property bool powr_sensor_running_cadence_half_on_strava: false
            property bool nordictrack_ifit_adb_remote: false
            property int floating_height: 210
            property int floating_width: 370

            // from version 2.12.32
            property int floating_transparency: 80

            // from version 2.12.34
            property bool floating_startup: false

            // from version 2.12.35
            property bool norditrack_s25i_treadmill: false

            // from version 2.12.36
            property bool toorx_ftms_treadmill: false

            // from version 2.12.38
            property bool nordictrack_t65s_83_treadmill: false
            property bool horizon_treadmill_suspend_stats_pause: false

            // from version 2.12.39
            property bool sportstech_sx600: false

            // from version 2.12.41
            property bool sole_elliptical_inclination: false

            // from version 2.12.43
            property bool proform_hybrid_trainer_xt: false
            property bool gears_restore_value: false
            property int gears_current_value: 0 // unused

            // from version 2.12.44
            property bool tile_pace_last500m_enabled: true
            property int  tile_pace_last500m_order: 49

            // from version 2.12.51
            property bool treadmill_difficulty_gain_or_offset: false
            property bool pafers_treadmill_bh_iboxster_plus: false

            // from version 2.12.52
            property bool proform_cycle_trainer_400: false

            // from version 2.12.58
            property bool fitshow_treadmill_miles: false
            property bool proform_hybrid_trainer_PFEL03815: false
            property int schwinn_resistance_smooth: 0

            // from version 2.12.59
            property bool peloton_workout_ocr: false
            property bool peloton_bike_ocr: false

            // from version 2.12.60
            property double treadmill_inclination_override_0: 0.0
            property double treadmill_inclination_override_05: 0.5
            property double treadmill_inclination_override_10: 1.0
            property double treadmill_inclination_override_15: 1.5
            property double treadmill_inclination_override_20: 2.0
            property double treadmill_inclination_override_25: 2.5
            property double treadmill_inclination_override_30: 3.0
            property double treadmill_inclination_override_35: 3.5
            property double treadmill_inclination_override_40: 4.0
            property double treadmill_inclination_override_45: 4.5
            property double treadmill_inclination_override_50: 5.0
            property double treadmill_inclination_override_55: 5.5
            property double treadmill_inclination_override_60: 6.0
            property double treadmill_inclination_override_65: 6.5
            property double treadmill_inclination_override_70: 7.0
            property double treadmill_inclination_override_75: 7.5
            property double treadmill_inclination_override_80: 8.0
            property double treadmill_inclination_override_85: 8.5
            property double treadmill_inclination_override_90: 9.0
            property double treadmill_inclination_override_95: 9.5
            property double treadmill_inclination_override_100: 10.0
            property double treadmill_inclination_override_105: 10.5
            property double treadmill_inclination_override_110: 11.0
            property double treadmill_inclination_override_115: 11.5
            property double treadmill_inclination_override_120: 12.0
            property double treadmill_inclination_override_125: 12.5
            property double treadmill_inclination_override_130: 13.0
            property double treadmill_inclination_override_135: 13.5
            property double treadmill_inclination_override_140: 14.0
            property double treadmill_inclination_override_145: 14.5
            property double treadmill_inclination_override_150: 15.0

            // from version 2.12.61
            property bool sole_elliptical_e55: false
            property bool horizon_treadmill_force_ftms: false

            // from version 2.12.64
            property int treadmill_pid_heart_min: 0
            property int treadmill_pid_heart_max: 0

            // from version 2.12.65
            property bool nordictrack_elliptical_c7_5: false

            // from version 2.12.66
            property bool renpho_bike_double_resistance: false

            // from version 2.12.69
            property bool nordictrack_incline_trainer_x7i: false

            // from version 2.12.71
            property bool strava_auth_external_webbrowser: false

            // from version 2.12.72
            property bool gears_from_bike: false

            // from version 2.13.4
            property bool peloton_spinups_autoresistance: true

            // from version 2.13.10
            property bool eslinker_costaway: false

            // from version 2.13.14
            property double treadmill_inclination_ovveride_gain: 1.0
            property double treadmill_inclination_ovveride_offset: 0.0

            // from version 2.13.15
            property bool bh_spada_2_watt: false
            property bool tacx_neo2_peloton: false

            // from version 2.13.16
            property bool sole_treadmill_inclination_fast: false

            // from version 2.13.17
            property bool zwift_ocr: false

            // from version 2.13.18
            property bool gem_module_inclination: false

            // from version 2.13.19
            property bool treadmill_simulate_inclination_with_speed: false

            // from version 2.13.26
            property bool garmin_companion: false

            // from version 2.13.27
            property bool peloton_companion_workout_ocr: false

            // from version 2.13.31
            property bool iconcept_elliptical: false

            // from version 2.13.37
            property bool theme_tile_icon_enabled: true
            property string theme_tile_background_color: "#303030"
            property string theme_status_bar_background_color: "#800080"

            // from version 2.13.43
            property string theme_background_color: "#303030"
            property bool theme_tile_shadow_enabled: true
            property string theme_tile_shadow_color: "#9C27B0"

            // from version 2.13.44
            property double gears_gain: 1.0
            property double gears_current_value_f: 0

            // from version 2.13.45
            property bool proform_treadmill_8_0: false

            // from version 2.13.50
            property bool zero_zt2500_treadmill: false            

            // from version 2.13.52
            property bool kingsmith_encrypt_v5: false

            // from version 2.13.58
            property int peloton_rower_level: 1

            // from version 2.13.61
            property bool tile_target_pace_enabled: false
            property int  tile_target_pace_order: 50
            property bool tts_act_target_pace: false

            // from version 2.13.62
            property string csafe_rower: ""

            // from version 2.13.63
            property string ftms_rower: "Disabled"

            // from version 2.13.71
            property int theme_tile_secondline_textsize: 12

            // from version 2.13.80
            property bool fakedevice_rower: false

            // from version 2.13.81
            property bool proform_bike_sb: false            

            // from version 2.13.86
            property bool zwift_workout_ocr: false

            // from version 2.13.96
            property bool zwift_ocr_climb_portal: false
            property int poll_device_time: 200

            // from version 2.13.99
            property bool proform_bike_PFEVEX71316_1: false
            property bool schwinn_bike_resistance_v3: false

            // from version 2.15.2
            property bool watt_ignore_builtin: true

            // from version 2.16.4
            property bool proform_treadmill_z1300i: false

            // from version 2.16.5
            property string ftms_bike: "Disabled"
            property string ftms_treadmill: "Disabled"

            // from version 2.16.6
            property real ant_speed_offset: 0
            property real ant_speed_gain: 1

            // from version 2.16.12
            property bool proform_rower_sport_rl: false

            // from version 2.16.13
            property bool strava_date_prefix: false

            // from version 2.16.17
            property bool race_mode: false

            // from version 2.16.22
            property bool proform_pro_1000_treadmill: false
            property bool saris_trainer: false

            // from version 2.16.23
            property bool proform_studio_NTEX71021: false
            property bool nordictrack_x22i: false

            // from version 2.16.25
            property bool iconsole_elliptical: false            
            property real autolap_distance: 0
            property bool nordictrack_s20_treadmill: false

            // from version 2.16.28
            property bool freemotion_coachbike_b22_7: false

            // from version 2.16.29
            property bool proform_cycle_trainer_300_ci: false
            property bool kingsmith_encrypt_g1_walking_pad: false
            property bool proform_bike_225_csx: false

            // from version 2.16.30
            property bool proform_treadmill_l6_0s: false
            property string proformtdf1ip: ""
            property string zwift_username: ""
            property string zwift_password: ""

            // from version 2.16.31
            property bool garmin_bluetooth_compatibility: false
            property bool norditrack_s25_treadmill: false
            property bool proform_8_5_treadmill: false
            property real treadmill_incline_min: -100
            property real treadmill_incline_max: 100

            // from version 2.16.32
            property bool proform_2000_treadmill: false
            property bool android_documents_folder: false
            property bool zwift_api_autoinclination: true

            // from version 2.16.34
            property real domyos_treadmill_button_5kmh: 5.0
            property real domyos_treadmill_button_10kmh: 10.0
            property real domyos_treadmill_button_16kmh: 16.0
            property real domyos_treadmill_button_22kmh: 22.0

            // from version 2.16.35
            property bool proform_treadmill_sport_8_5: false

            // from version 2.16.37
            property bool domyos_treadmill_t900a: false

            // from version 2.16.38
            property bool enerfit_SPX_9500: false

            // from version 2.16.39
            property bool proform_treadmill_505_cst: false

            // from version 2.16.40
            property bool nordictrack_treadmill_t8_5s: false

            // from version 2.16.41
            property bool proform_treadmill_705_cst: false
            property bool zwift_click: false

            // from version 2.16.42
            property bool hop_sport_hs_090h_bike: false
            property bool zwift_play: false
            property bool nordictrack_treadmill_x14i: false
            property int zwift_api_poll: 5

            // from version 2.16.43
            property bool tile_step_count_enabled: false
            property int  tile_step_count_order: 51            

            // from version 2.16.44
            property bool tile_erg_mode_enabled: false
            property int  tile_erg_mode_order: 52

            // from version 2.16.45
            property bool toorx_srx_3500: false  
            property real inclination_delay_seconds: 0.0

            // from version 2.16.47
            property string ergDataPoints: ""
            property bool proform_tdf_10_0: false
            property bool proform_carbon_tl: false
            property bool proform_proshox2: false            

            // from version 2.16.51
            property bool nordictrack_GX4_5_bike: false            

            // from version 2.16.52
            property real ftp_run: 200.0
            property bool tile_rss_enabled: false
            property int  tile_rss_order: 53
            property string treadmillDataPoints: ""

            // from version 2.16.54
            property bool nordictrack_s20i_treadmill: false
            property bool stryd_speed_instead_treadmill: false
            property bool proform_595i_proshox2: false

            // from version 2.16.55
            property bool proform_treadmill_8_7: false            

            // from version 2.16.56
            property bool proform_bike_325_csx: false            

            // from version 2.16.58
            property string strava_upload_mode: "Always"

            // from version 2.16.59
            property bool proform_treadmill_705_cst_V78_239: false

            // from version 2.16.62
            property bool stryd_add_inclination_gain: false
            property bool toorx_bike_srx_500: false

            // from version 2.16.66
            property bool atletica_lightspeed_treadmill: false

            // from version 2.16.68
            property int peloton_treadmill_level: 1
            property bool nordictrackadbbike_resistance: false
            property bool proform_treadmill_carbon_t7: false
            property bool nordictrack_treadmill_exp_5i: false
            property int dircon_id: 0
            property string proform_elliptical_ip: ""

            // from version 2.16.69
            property bool antbike: false
            property bool domyosbike_notfmts: false

            // from version 2.16.70
            property bool gears_volume_debouncing: false
            property bool tile_biggears_enabled: false
            property int  tile_biggears_order: 54
            property bool domyostreadmill_notfmts: false
            property bool zwiftplay_swap: false
            property bool gears_zwift_ratio: false
            property bool domyos_bike_500_profile_v2: false
            property double gears_offset: 0.0

            property bool proform_carbon_tl_PFTL59720: false

            // from version 2.16.71
            property bool proform_treadmill_sport_70: false
            property string peloton_date_format: "MM/dd/yy"
            property bool force_resistance_instead_inclination: false
            property bool proform_treadmill_575i: false

            // from version 2.18.1
            property bool zwift_play_emulator: false

            // from version 2.18.2
            property string gear_configuration: "1|38|44|true\n2|38|38|true\n3|38|32|true\n4|38|28|true\n5|38|24|true\n6|38|21|true\n7|38|19|true\n8|38|17|true\n9|38|15|true\n10|38|13|true\n11|38|11|true\n12|38|10|true"
            property int gear_crankset_size: 42
            property int gear_cog_size: 14
            property string gear_wheel_size: "700 x 18C"
            property real gear_circumference: 2070

            property bool watt_bike_emulator: false

            property bool restore_specific_gear: false
            property bool skipLocationServicesDialog: false
            property bool trainprogram_pid_pushy: true
            property real min_inclination: -999

            // from version 2.18.3
            property bool proform_performance_400i: false

            // from version 2.18.5
            property bool proform_treadmill_c700: false
            property bool sram_axs_controller: false
            property bool proform_treadmill_c960i: false

            // from version 2.18.6
            property string mqtt_host: ""
            property int mqtt_port: 1883
            property string mqtt_username: ""
            property string mqtt_password: ""
            property string mqtt_deviceid: "default"
            property bool peloton_auto_start_with_intro: false
            property bool peloton_auto_start_without_intro: false

            // from version 2.18.7
            property bool nordictrack_tseries5_treadmill: false
            property bool proform_carbon_tl_PFTL59722c: false

            // from version 2.18.9
            property bool nordictrack_gx_44_pro: false

            // from version 2.18.10
            property string csafe_elliptical_port: ""
            property string osc_ip: ""
            property int osc_port: 9000

            // from version 2.18.11
            property bool strava_treadmill: true
            property bool iconsole_rower: false

            // from version 2.18.14
            property bool proform_treadmill_1500_pro: false

            // from version 2.18.15
            property bool proform_505_cst_80_44: false

            // from version 2.18.16
            property bool proform_trainer_8_0: false

            // from version 2.18.18
            property bool tile_biggears_swap: false
            property bool treadmill_follow_wattage: false
            property bool fit_file_garmin_device_training_effect: false
            property bool proform_treadmill_705_cst_V80_44: false

            // from version 2.18.19
            property string  peloton_accesstoken: ""
            property string  peloton_refreshtoken: ""
            property string  peloton_lastrefresh: ""
            property string  peloton_expires: ""
            property string  peloton_code: ""

            property bool nordictrack_treadmill_1750_adb: false

            property bool tile_preset_powerzone_1_enabled: false
            property int tile_preset_powerzone_1_order: 55
            property real tile_preset_powerzone_1_value: 1.0
            property string tile_preset_powerzone_1_label: "Zone 1"
            property string tile_preset_powerzone_1_color: "white"

            property bool tile_preset_powerzone_2_enabled: false
            property int tile_preset_powerzone_2_order: 56
            property real tile_preset_powerzone_2_value: 2.0
            property string tile_preset_powerzone_2_label: "Zone 2"
            property string tile_preset_powerzone_2_color: "limegreen"

            property bool tile_preset_powerzone_3_enabled: false
            property int tile_preset_powerzone_3_order: 57
            property real tile_preset_powerzone_3_value: 3.0
            property string tile_preset_powerzone_3_label: "Zone 3"
            property string tile_preset_powerzone_3_color: "gold"

            property bool tile_preset_powerzone_4_enabled: false
            property int tile_preset_powerzone_4_order: 58
            property real tile_preset_powerzone_4_value: 4.0
            property string tile_preset_powerzone_4_label: "Zone 4"
            property string tile_preset_powerzone_4_color: "orange"

            property bool tile_preset_powerzone_5_enabled: false
            property int tile_preset_powerzone_5_order: 59
            property real tile_preset_powerzone_5_value: 5.0
            property string tile_preset_powerzone_5_label: "Zone 5"
            property string tile_preset_powerzone_5_color: "darkorange"

            property bool tile_preset_powerzone_6_enabled: false
            property int tile_preset_powerzone_6_order: 60
            property real tile_preset_powerzone_6_value: 6.0
            property string tile_preset_powerzone_6_label: "Zone 6"
            property string tile_preset_powerzone_6_color: "orangered"

            property bool tile_preset_powerzone_7_enabled: false
            property int tile_preset_powerzone_7_order: 61
            property real tile_preset_powerzone_7_value: 7.0
            property string tile_preset_powerzone_7_label: "Zone 7"
            property string tile_preset_powerzone_7_color: "red"  

            property bool proform_bike_PFEVEX71316_0: false
            property bool real_inclination_to_virtual_treamill_bridge: false
            property bool stryd_inclination_instead_treadmill: false

            // 2.18.20
            property bool domyos_elliptical_fmts: false
            property bool proform_xbike: false            
            property bool proform_225_csx_PFEX32925_INT_0: false
            property string peloton_current_user_id: ""

            // 2.18.22
            property bool trainprogram_pid_ignore_inclination: false
            property bool tile_hr_time_in_zone_1_enabled: false
            property int  tile_hr_time_in_zone_1_order: 62
            property bool tile_hr_time_in_zone_2_enabled: false
            property int  tile_hr_time_in_zone_2_order: 63
            property bool tile_hr_time_in_zone_3_enabled: false
            property int  tile_hr_time_in_zone_3_order: 64
            property bool tile_hr_time_in_zone_4_enabled: false
            property int  tile_hr_time_in_zone_4_order: 65
            property bool tile_hr_time_in_zone_5_enabled: false
            property int  tile_hr_time_in_zone_5_order: 66

            // 2.18.25
            property bool zwift_gear_ui_aligned: false
            property bool tacxneo2_disable_negative_inclination: false

            // 2.18.26
            property bool proform_performance_300i: false
            property bool android_antbike: false

            property bool tile_coretemperature_enabled: false
            property int  tile_coretemperature_order: 67

            property bool nordictrack_t65s_treadmill_81_miles: false
            property bool nordictrack_elite_800: false
            property bool ios_btdevice_native: false            
            property string inclinationResistancePoints: ""
            property int floatingwindow_type: 0
            property bool horizon_treadmill_7_0_at_24: false  // not used

            property bool nordictrack_treadmill_ultra_le: false            

            property bool tile_heat_time_in_zone_1_enabled: false
            property int  tile_heat_time_in_zone_1_order: 68
            property bool tile_heat_time_in_zone_2_enabled: false
            property int  tile_heat_time_in_zone_2_order: 69
            property bool tile_heat_time_in_zone_3_enabled: false
            property int  tile_heat_time_in_zone_3_order: 70
            property bool tile_heat_time_in_zone_4_enabled: false
            property int  tile_heat_time_in_zone_4_order: 71

            property bool proform_treadmill_carbon_tls: false

            // 2.19.1            
            property bool proform_treadmill_995i: false
            property bool rogue_echo_bike: false
            property int fit_file_garmin_device_training_effect_device: 3122            

            // 2.19.2
            property bool tile_hr_time_in_zone_individual_mode: false
            property bool wahoo_without_wheel_diameter: false

            // 2.20.3
            property bool technogym_group_cycle: false
            property int ant_bike_device_number: 0
            property int ant_heart_device_number: 0
            property int peloton_treadmill_walk_level: 1
            property int pid_heart_zone_erg_mode_watt_step: 5            

            // Automatic Virtual Shifting settings
            property bool automatic_virtual_shifting_enabled: false
            property int automatic_virtual_shifting_gear_up_cadence: 95
            property real automatic_virtual_shifting_gear_up_time: 2.0
            property int automatic_virtual_shifting_gear_down_cadence: 65
            property real automatic_virtual_shifting_gear_down_time: 2.0
            property int automatic_virtual_shifting_profile: 0
            property int automatic_virtual_shifting_climb_gear_up_cadence: 95
            property real automatic_virtual_shifting_climb_gear_up_time: 2.0
            property int automatic_virtual_shifting_climb_gear_down_cadence: 65
            property real automatic_virtual_shifting_climb_gear_down_time: 2.0
            property int automatic_virtual_shifting_sprint_gear_up_cadence: 95
            property real automatic_virtual_shifting_sprint_gear_up_time: 2.0
            property int automatic_virtual_shifting_sprint_gear_down_cadence: 65
            property real automatic_virtual_shifting_sprint_gear_down_time: 2.0
            property bool tile_auto_virtual_shifting_cruise_enabled: false
            property int tile_auto_virtual_shifting_cruise_order: 55
            property bool tile_auto_virtual_shifting_climb_enabled: false
            property int tile_auto_virtual_shifting_climb_order: 56
            property bool tile_auto_virtual_shifting_sprint_enabled: false
            property int tile_auto_virtual_shifting_sprint_order: 57
            property string proform_rower_ip: ""
            property string ftms_elliptical: "Disabled"
            
            property bool calories_active_only: false
            property real height: 175.0
            property bool calories_from_hr: false
            property int bike_power_offset: 0
            property int chart_display_mode: 0
            property bool zwift_play_vibration: true
            property bool toorxtreadmill_discovery_completed: false
            property bool taurua_ic90: false
            property bool proform_csx210: false
            property bool confirm_stop_workout: false                       
            property bool proform_rower_750r: false             
            property bool virtual_device_force_treadmill: false
            property bool proform_trainer_9_0: false
            property bool iconcept_ftms_treadmill_inclination_table: false
            property bool skandika_wiri_x2000_protocol: true
            property bool nordictrack_series_7: false
            property string kettler_usb_serialport: ""			
            property int kettler_usb_baudrate: 9600
            property bool nordictrack_se7i: false
            property real treadmill_speed_max: 100

            // Intervals.icu settings
            property string intervalsicu_accesstoken: ""
            property string intervalsicu_refreshtoken: ""
            property string intervalsicu_athlete_id: ""
            property bool intervalsicu_upload_enabled: true
            property string intervalsicu_suffix: "#QZ"
            property bool intervalsicu_date_prefix: false            

            property bool proform_treadmill_sport_3_0: false
            property bool rouvy_compatibility: false
            property bool tile_negative_inclination_enabled: false
            property int tile_negative_inclination_order: 75
            property bool tile_avg_pace_enabled: false
            property int  tile_avg_pace_order: 76

            // Garmin connect
            property string garmin_email: ""
            property string garmin_password: ""
            property bool garmin_upload_enabled: false
            property string garmin_access_token: ""
            property string garmin_refresh_token: ""
            property string garmin_token_type: ""
            property var garmin_expires_at: 0
            property var garmin_refresh_token_expires_at: 0
            property string garmin_domain: "garmin.com"
            property string garmin_last_refresh: ""

            property bool power_sensor_cadence_instead_treadmill: false

            property string garmin_oauth1_token: ""
            property string garmin_oauth1_token_secret: ""

			property bool domyos_treadmill_sync_start: false
			property string garmin_device_serial: "3313379353"
			property real treadmill_speed_min: 0
			property real peloton_treadmill_walking_min_speed: 0.0
			property real peloton_treadmill_running_min_speed: 0.0
			property bool trainprogram_auto_lap_on_segment: false

			property bool power_avg_3s: false
			property bool tile_power_avg_enabled: false
			property int tile_power_avg_order: 77
			property bool life_fitness_ic5: false
			property bool technogym_bike: false

			property bool kingsmith_r2_enable_hw_buttons: false
			property bool treadmill_direct_distance: false

			property bool domyos_treadmill_ts100: false
			property bool thinkrider_controller: false			
			property bool weight_kg_unit: false 
			property bool virtual_device_rower_pm5: false
			property bool tile_heart_show_as_percent: false
			property bool tile_hrv_enabled: false
			property int tile_hrv_order: 78                 

            property bool nordictrack_gx_4_5_pro: false            
            property double step_gain: 1.0
            property bool sportstech_esx500: false
            property bool proform_bike_325_csx_PFEX439210INT_0: false
            property bool proform_carbon_tlx_treadmill: false

            property bool nordictrack_vr21: false
            property bool gymstick_gx6_0_elliptical: false
            property bool cadence_sensor_as_treadmill: false
            property bool proform_trainer_8_0_pftl59721_int_0: false
            property bool proform_carbon_tl_PFTL59723_6: false
            property bool toputure_teb1: false
            property string ios_live_activity_compact_leading_metric: "Heart Rate"
            property string ios_live_activity_compact_trailing_metric: "Watt"
            property bool nordictrack_treadmill_commercial_le: false

            property bool umay_s100_treadmill: false            
            property bool gym_mode: false
            property bool tile_grade_adjusted_pace_enabled: false
            property int tile_grade_adjusted_pace_order: 79
            property bool cycplus_bc2_controller: false
      		property bool lifespan_bike: false

            property double power_sensor_speed_inclination_coeff_a: 0.0
            property double power_sensor_speed_inclination_coeff_b: 0.0

            property bool proform_carbon_tlx_v84_314_treadmill: false            
            property bool cscbike_custom_resistance_power_table: false
            property real cscbike_custom_resistance_level_1: 1
            property real cscbike_custom_watt_1: 100
            property real cscbike_custom_resistance_level_2: 15
            property real cscbike_custom_watt_2: 300
            property bool applewatch_as_treadmill_speed: false
            property bool gears_custom_table_enabled: false
            property string gears_custom_table: "1|1\n2|2\n3|3\n4|4\n5|5\n6|6\n7|7\n8|8\n9|9\n10|10\n11|11\n12|12\n13|13\n14|14\n15|15\n16|16\n17|17\n18|18\n19|19\n20|20\n21|21\n22|22\n23|23\n24|24"                        
            property bool proform_treadmill_cst_505_pftl59420_0: false

            property bool domyos_run100e: false

            property bool shortcuts_enabled: false
            property string shortcut_speed_plus: ""
            property string shortcut_speed_minus: ""
            property string shortcut_inclination_plus: ""
            property string shortcut_inclination_minus: ""
            property string shortcut_resistance_plus: ""
            property string shortcut_resistance_minus: ""
            property string shortcut_peloton_resistance_plus: ""
            property string shortcut_peloton_resistance_minus: ""
            property string shortcut_target_resistance_plus: ""
            property string shortcut_target_resistance_minus: ""
            property string shortcut_target_power_plus: ""
            property string shortcut_target_power_minus: ""
            property string shortcut_target_zone_plus: ""
            property string shortcut_target_zone_minus: ""
            property string shortcut_target_speed_plus: ""
            property string shortcut_target_speed_minus: ""
            property string shortcut_target_incline_plus: ""
            property string shortcut_target_incline_minus: ""
            property string shortcut_fan_plus: ""
            property string shortcut_fan_minus: ""
            property string shortcut_peloton_offset_plus: ""
            property string shortcut_peloton_offset_minus: ""
            property string shortcut_peloton_remaining_plus: ""
            property string shortcut_peloton_remaining_minus: ""
            property string shortcut_remaining_time_plus: ""
            property string shortcut_remaining_time_minus: ""
            property string shortcut_gears_plus: ""
            property string shortcut_gears_minus: ""
            property string shortcut_pid_hr_plus: ""
            property string shortcut_pid_hr_minus: ""
            property string shortcut_ext_incline_plus: ""
            property string shortcut_ext_incline_minus: ""
            property string shortcut_biggears_plus: ""
            property string shortcut_biggears_minus: ""
            property string shortcut_avs_cruise: ""
            property string shortcut_avs_climb: ""
            property string shortcut_avs_sprint: ""
            property string shortcut_power_avg: ""
            property string shortcut_erg_mode: ""
            property string shortcut_auto_resistance: ""
            property string shortcut_preset_resistance_1: ""
            property string shortcut_preset_resistance_2: ""
            property string shortcut_preset_resistance_3: ""
            property string shortcut_preset_resistance_4: ""
            property string shortcut_preset_resistance_5: ""
            property string shortcut_preset_speed_1: ""
            property string shortcut_preset_speed_2: ""
            property string shortcut_preset_speed_3: ""
            property string shortcut_preset_speed_4: ""
            property string shortcut_preset_speed_5: ""
            property string shortcut_preset_inclination_1: ""
            property string shortcut_preset_inclination_2: ""
            property string shortcut_preset_inclination_3: ""
            property string shortcut_preset_inclination_4: ""
            property string shortcut_preset_inclination_5: ""
            property string shortcut_preset_powerzone_1: ""
            property string shortcut_preset_powerzone_2: ""
            property string shortcut_preset_powerzone_3: ""
            property string shortcut_preset_powerzone_4: ""
            property string shortcut_preset_powerzone_5: ""
            property string shortcut_preset_powerzone_6: ""
            property string shortcut_preset_powerzone_7: ""
            property string shortcut_lap: ""
            property string shortcut_start_stop: ""
            property string garmin_last_seen_cycling_ftp_create_time: ""
            property string garmin_last_seen_running_ftp_create_time: ""
            property bool horizon_treadmill_omega_z: false

            property string app_language: "auto"

            property bool garmin_download_workouts_on_start: true
            property bool trainprogram_clipboard_workout_enabled: false
            property string shortcut_stop: ""
            property real trainprogram_warmup_speed: 420
            property real trainprogram_cooldown_speed: 420
            property real trainprogram_rest_speed: 420
            property bool trainprogram_sound_on_segment: false
            property bool tile_watt_color_enabled: true
            property bool tile_pace_color_enabled: true
            property bool treadmill_force_running_activity: false
            property bool proform_treadmill_105_cst: false            
            property real trainprogram_pid_hr_pushy_zone_limit: 0.8
            property real trainprogram_pid_hr_recovery_zone_limit: 60.0            
            property bool rpe_feel_popup_enabled: false
            property int zwiftplay_gear_ls1: 2
            property int zwiftplay_gear_ls2: 2
            property int zwiftplay_gear_rs1: 1
            property int zwiftplay_gear_rs2: 1
            property int zwiftplay_gear_paddle_left: 2
            property int zwiftplay_gear_paddle_right: 1
            property int zwiftplay_gear_lb: 0
            property int zwiftplay_gear_rb: 0
            // OpenBikeControl settings
            property bool mywhoosh_link_enabled: false
            property bool mywhoosh_link_override_gears: false
            property int mywhoosh_link_left_up: 1
            property int mywhoosh_link_left_down: 2
            property int mywhoosh_link_left_left: 0
            property int mywhoosh_link_left_right: 0
            property int mywhoosh_link_left_shoulder: 5
            property int mywhoosh_link_left_power: 0
            property int mywhoosh_link_right_y: 6
            property int mywhoosh_link_right_a: 0
            property int mywhoosh_link_right_b: 7
            property int mywhoosh_link_right_z: 0
            property int mywhoosh_link_right_shoulder: 0
            property int mywhoosh_link_right_power: 0
            property int mywhoosh_link_camera_value: 1
            property int mywhoosh_link_emote_value: 1
            property bool waterrower_usb: false
            property string freebeat_serialport: ""
            property bool nordictrack_elliptical_s700: false
            property bool virtual_device_tacx: false
            property bool renpho_bike_knob_gears: false
            property bool nordictrack_incline_trainer_x7i_ntl15010_0: false
            property bool custom_inclination_resistance_table_enabled: false
            property string custom_inclination_resistance_table: "0|4\n1|6\n2|8\n3|10\n4|11\n5|11.5\n6|12\n8|13\n10|14\n12|15\n15|16"
        }


        function paddingZeros(text, limit) {
          if (text.length < limit) {
            return paddingZeros("0" + text, limit);
          } else {
            return text;
          }
        }

        function formatLimitDecimals(value, decimals) {
          const stringValue = value.toString();
          if(stringValue.includes('e')) {
              // TODO: remove exponential notation
              throw 'invald number';
          } else {
            const [integerPart, decimalPart] = stringValue.split('.');
            if(decimalPart) {
              return +[integerPart, decimalPart.slice(0, decimals)].join('.')
            } else {
              return integerPart;
            }
          }
        }

        function paceSecondsToTime(secondsPerKm) {
            return paddingZeros(formatLimitDecimals(secondsPerKm / 3600, 0).toString(), 2) + ":" +
                   paddingZeros(formatLimitDecimals((secondsPerKm / 60) % 60, 0).toString(), 2) + ":" +
                   paddingZeros(formatLimitDecimals(secondsPerKm % 60, 0).toString(), 2)
        }

        function timeToPaceSeconds(text) {
            var pieces = text.split(":")
            return (parseInt(pieces[0]) * 3600) + (parseInt(pieces[1]) * 60) + parseInt(pieces[2])
        }

        Component.onCompleted: {
            window.settings_restart_to_apply = false;
            Qt.callLater(function() { settingsPane.openModernSettingsPreview() })
        }
        MessageDialog {
            id: zwiftPlaySettingsDialog
            text: qsTr("Zwift Play & Click Settings")
            informativeText: qsTr("Would you like to disable Zwift Play and Zwift Click settings? Having them enabled together with 'Get gears from Zwift' may cause conflicts.")
            buttons: (MessageDialog.Yes | MessageDialog.No)
            onYesClicked: {
                settings.zwift_play = false;
                settings.zwift_click = false;
                settings.zwift_play_emulator = true;
                window.settings_restart_to_apply = true;
            }
            visible: false
        }

        Popup {
            id: garminMfaDialog
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape
            anchors.centerIn: Overlay.overlay
            width: Math.min(settingsPane.width * 0.9, 400)
            visible: rootItem.garminMfaRequested

            ColumnLayout {
                anchors.fill: parent
                spacing: 20

                Label {
                    text: qsTr("Garmin MFA Required")
                    font.pixelSize: 18
                    font.bold: true
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    text: qsTr("Garmin has sent a verification code to your email.\\nPlease enter it below:")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    text: qsTr("If you don't receive the code, please enable 2FA in your Garmin profile privacy settings.")
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 12
                    font.italic: true
                    color: Material.color(Material.Grey)
                }

                TextField {
                    id: mfaCodeTextField
                    placeholderText: qsTr("Enter MFA code")
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    font.pixelSize: 16
                    onAccepted: {
                        if (text.length > 0) {
                            rootItem.garmin_submit_mfa_code(text)
                            text = ""
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: qsTr("Cancel")
                        Layout.fillWidth: true
                        onClicked: {
                            mfaCodeTextField.text = ""
                            rootItem.garminMfaRequested = false
                            garminMfaDialog.close()
                        }
                    }

                    Button {
                        text: qsTr("Submit")
                        Layout.fillWidth: true
                        enabled: mfaCodeTextField.text.length > 0
                        onClicked: {
                            rootItem.garmin_submit_mfa_code(mfaCodeTextField.text)
                            mfaCodeTextField.text = ""
                        }
                    }
                }
            }

            onOpened: mfaCodeTextField.forceActiveFocus()
        }

        Item {
            id: modernSettingsDrawer
            anchors.fill: parent
            visible: false

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

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

                Label {
                    visible: settingsCatalogError.length > 0
                    text: settingsCatalogError
                    color: Material.color(Material.Red)
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.leftMargin: 12
                    Layout.rightMargin: 12
                }

                ListView {
                    id: modernCategoryList
                    visible: modernSettingsParent.length === 0 && modernSettingsSearch.text.length === 0 && modernSettingsExternalTarget.length === 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 2
                    leftMargin: 20
                    rightMargin: 20
                    topMargin: 18
                    bottomMargin: 24
                    model: modernSettingsCategories

                    delegate: Rectangle {
                        width: modernCategoryList.width - modernCategoryList.leftMargin - modernCategoryList.rightMargin
                        height: 58
                        radius: 10
                        color: settingsPane.modernCardColor()

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 18
                            anchors.rightMargin: 14
                            spacing: 8

                            Label {
                                Layout.fillWidth: true
                                text: modelData.name
                                font.pixelSize: Qt.application.font.pixelSize + 1
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                            Label {
                                text: "›"
                                color: Material.color(Material.Grey)
                                font.pixelSize: Qt.application.font.pixelSize + 8
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (modelData.catalogKind === "page")
                                    settingsPane.openModernCatalogPage(modelData)
                                else
                                    settingsPane.openModernSettingsCategory(modelData.key)
                            }
                        }
                    }
                }

                ListView {
                    id: modernItemList
                    visible: modernSettingsParent.length > 0 || modernSettingsSearch.text.length > 0 || modernSettingsExternalTarget.length > 0
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 2
                    leftMargin: 12
                    rightMargin: 12
                    topMargin: 12
                    bottomMargin: 24
                    model: modernSettingsItems

                    delegate: Rectangle {
                        id: modernSettingRow
                        property var entry: modelData
                        width: modernItemList.width - modernItemList.leftMargin - modernItemList.rightMargin
                        height: modernSettingContent.childrenRect.height + 24
                        radius: 10
                        color: settingsPane.modernCardColor()

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
                                    text: entry.target && entry.target.indexOf("__category__:") === 0 ? "›" : qsTr("Open")
                                    onClicked: settingsPane.openModernCatalogPage(entry)
                                }

                                Button {
                                    visible: entry.key === "garmin_upload_enabled"
                                    text: qsTr("Test Garmin Login")
                                    onClicked: rootItem.garmin_connect_login()
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
                                    text: visible ? settingsPane.displaySettingValue(entry) : ""
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
                                model: visible ? settingsPane.optionLabels(entry) : []
                                currentIndex: visible ? settingsPane.optionIndex(entry) : 0
                                onActivated: {
                                    var selectedValue = settingsPane.optionValues(entry)[currentIndex]
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

        ColumnLayout {
            id: settingsActionsView
            anchors.fill: parent
            anchors.margins: 16
            visible: false
            focus: true
            spacing: 12

            Label {
                text: qsTr("Service Actions")
                font.pixelSize: Qt.application.font.pixelSize + 4
                font.bold: true
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("These actions are not regular settings and are kept here separately from the settings catalog.")
                color: Material.color(Material.Grey)
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Open Garmin Settings")
                Layout.fillWidth: true
                onClicked: {
                    settingsActionsView.visible = false
                    modernSettingsDrawer.visible = true
                    if (!openModernSettingsCategoryByName("Garmin Options"))
                        modernSettingsPendingCategory = "Garmin Options"
                }
            }

            Button {
                text: qsTr("Connect Peloton")
                Layout.fillWidth: true
                onClicked: {
                    stackView.push("WebPelotonAuth.qml")
                    peloton_connect_clicked()
                }
            }

            Button {
                text: qsTr("Refresh Bluetooth Devices")
                Layout.fillWidth: true
                onClicked: refresh_bluetooth_devices_clicked()
            }

            Button {
                text: qsTr("Open Floating Window in Browser")
                Layout.fillWidth: true
                onClicked: openFloatingWindowBrowser()
            }

            Button {
                text: qsTr("Clear History and Session Files")
                Layout.fillWidth: true
                onClicked: rootItem.clearFiles()
            }

            Button {
                text: qsTr("Show Logs Folder")
                Layout.fillWidth: true
                onClicked: toast.show(rootItem.getProfileDir())
            }

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }


    }
/*##^##
Designer {
    D{i:0;formeditorZoom:0.6600000262260437}
}
##^##*/
