from pathlib import Path


def replace_once(path, old, new):
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected one match, found {count}: {old[:100]!r}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")


main = Path("src/main.qml")
replace_once(main,
'''    property bool lockTiles: false
    property bool settings_restart_to_apply: false
    property bool gymModePopupDismissed: false
''',
'''    property bool lockTiles: false
    property bool settings_restart_to_apply: false
    property bool gymModePopupDismissed: false

    // SETTINGS_QML_VISUAL_PARITY_V1
    property bool settingsVisualTestMode: Qt.application.arguments.indexOf("--settings-visual-test") >= 0
    Timer {
        id: settingsVisualTestOpenTimer
        interval: 1200
        repeat: false
        running: window.settingsVisualTestMode
        onTriggered: {
            console.log("SETTINGS_VISUAL: opening settings.qml")
            stackView.push("settings.qml")
        }
    }
''')

settings = Path("src/settings.qml")
replace_once(settings,
'''        Component.onCompleted: {
            window.settings_restart_to_apply = false;
            Qt.callLater(function() { settingsPane.openModernSettingsPreview() })
        }
''',
'''        // SETTINGS_QML_VISUAL_HARNESS_V1
        property bool visualTestMode: Qt.application.arguments.indexOf("--settings-visual-test") >= 0
        property string visualOutputDir: "settings-qml-visual-report/raw"

        function visualResolveOutputDir() {
            var args = Qt.application.arguments
            var prefix = "--settings-visual-output="
            for (var i = 0; i < args.length; i++) {
                if (args[i].indexOf(prefix) === 0)
                    return args[i].substring(prefix.length)
            }
            return visualOutputDir
        }

        function visualSlug(value) {
            var s = String(value || "section").toLowerCase()
            s = s.replace(/[^a-z0-9]+/g, "-")
            s = s.replace(/^-+|-+$/g, "")
            return s.length > 0 ? s : "section"
        }

        function visualCapture(filename, callback) {
            Qt.callLater(function() {
                window.contentItem.grabToImage(function(result) {
                    var path = visualOutputDir + "/" + filename
                    var ok = result.saveToFile(path)
                    console.log("SETTINGS_VISUAL:", ok ? "saved" : "FAILED", path)
                    Qt.callLater(callback)
                })
            })
        }

        function visualPositions(contentHeight, viewportHeight) {
            var positions = [0]
            var maxY = Math.max(0, contentHeight - viewportHeight)
            var step = Math.max(1, Math.floor(viewportHeight * 0.80))
            var y = step
            while (y < maxY) {
                positions.push(y)
                y += step
            }
            if (maxY > 0 && positions[positions.length - 1] !== maxY)
                positions.push(maxY)
            return positions
        }

        function visualCaptureList(prefix, key, list, callback) {
            var positions = visualPositions(list.contentHeight, list.height)
            function captureAt(index) {
                if (index >= positions.length) {
                    callback()
                    return
                }
                list.contentY = positions[index]
                visualCapture(prefix + "-" + visualSlug(key) + "-" + index + ".png", function() { captureAt(index + 1) })
            }
            captureAt(0)
        }

        function visualPageHasCatalogEntries(target) {
            var layout = settingsCatalog.legacyLayout || ({})
            var sourceMap = layout.sourceFileByKey || ({})
            for (var key in sourceMap) {
                if (sourceMap[key] === target)
                    return true
            }
            var pageParents = layout.pageParentTargetByKey || ({})
            for (var pageKey in pageParents) {
                if (pageParents[pageKey] === target)
                    return true
            }
            return false
        }

        function visualOpenModernRoot(category) {
            modernSettingsSearch.text = ""
            modernSettingsParent = ""
            modernSettingsExternalTarget = ""
            modernSettingsExternalTitle = ""
            modernSettingsExternalParent = ""
            if (category.catalogKind === "category") {
                modernSettingsParent = category.key
                rebuildModernSettingsItems("")
                return true
            }
            if (category.catalogKind === "page" && visualPageHasCatalogEntries(category.target)) {
                modernSettingsExternalTarget = category.target
                modernSettingsExternalTitle = category.name || qsTr("Settings")
                rebuildModernSettingsItems("")
                return true
            }
            return false
        }

        function visualCaptureModernCategories(index) {
            if (index >= modernSettingsCategories.length) {
                visualStartLegacy()
                return
            }
            var category = modernSettingsCategories[index]
            if (!visualOpenModernRoot(category)) {
                console.log("SETTINGS_VISUAL: skipping external-only modern page", category.name)
                visualCaptureModernCategories(index + 1)
                return
            }
            visualCaptureList("modern", category.name, modernItemList, function() {
                visualCaptureModernCategories(index + 1)
            })
        }

        function visualLegacyRoots() {
            var roots = []
            var children = settingsContent.children
            for (var i = 0; i < children.length; i++) {
                var child = children[i]
                if (child && child.title !== undefined && child.isOpen !== undefined)
                    roots.push(child)
            }
            return roots
        }

        function visualCloseLegacyRoots(roots) {
            for (var i = 0; i < roots.length; i++)
                roots[i].isOpen = false
        }

        function visualCaptureLegacyRoot(index, roots) {
            if (index >= roots.length) {
                console.log("SETTINGS_VISUAL: complete")
                Qt.quit()
                return
            }
            visualCloseLegacyRoots(roots)
            var item = roots[index]
            item.isOpen = true
            Qt.callLater(function() {
                var viewport = Math.max(1, settingsPane.height)
                var start = Math.max(0, item.y)
                var end = Math.max(start, item.y + item.height - viewport)
                var positions = []
                var step = Math.max(1, Math.floor(viewport * 0.80))
                var y = start
                positions.push(start)
                while (y + step < end) {
                    y += step
                    positions.push(y)
                }
                if (end > start && positions[positions.length - 1] !== end)
                    positions.push(end)
                function captureAt(segment) {
                    if (segment >= positions.length) {
                        visualCaptureLegacyRoot(index + 1, roots)
                        return
                    }
                    if (settingsPane.contentItem)
                        settingsPane.contentItem.contentY = positions[segment]
                    visualCapture("legacy-" + visualSlug(item.title) + "-" + segment + ".png", function() { captureAt(segment + 1) })
                }
                captureAt(0)
            })
        }

        function visualStartLegacy() {
            modernSettingsDrawer.close()
            legacySettingsUiEnabled = true
            if (settingsPane.contentItem)
                settingsPane.contentItem.contentY = 0
            Qt.callLater(function() {
                visualCapture("legacy-root-0.png", function() {
                    var roots = visualLegacyRoots()
                    console.log("SETTINGS_VISUAL: legacy root accordions", roots.length)
                    visualCaptureLegacyRoot(0, roots)
                })
            })
        }

        function visualStart() {
            visualOutputDir = visualResolveOutputDir()
            console.log("SETTINGS_VISUAL: output", visualOutputDir)
            openModernSettingsPreview()
            Qt.callLater(function() {
                visualCaptureList("modern", "root", modernCategoryList, function() {
                    visualCaptureModernCategories(0)
                })
            })
        }

        Timer {
            id: visualStartTimer
            interval: 900
            repeat: false
            onTriggered: settingsPane.visualStart()
        }

        Component.onCompleted: {
            window.settings_restart_to_apply = false;
            Qt.callLater(function() { settingsPane.openModernSettingsPreview() })
            if (visualTestMode)
                visualStartTimer.start()
        }
''')

workflow = Path(".github/workflows/main.yml")
replace_once(workflow,
'''      - name: Compile Linux Desktop
        run: qmake; make -j8

      - name: Archive linux-desktop binary
''',
'''      - name: Compile Linux Desktop
        run: qmake; make -j8

      - name: Capture real QML settings parity screenshots
        if: github.event_name == 'pull_request'
        run: |
          set -euo pipefail
          rm -rf settings-qml-visual-report
          mkdir -p settings-qml-visual-report/raw
          set +e
          timeout 210s ./src/qdomyos-zwift --settings-visual-test --settings-visual-output="${{ github.workspace }}/settings-qml-visual-report/raw"
          app_status=$?
          set -e
          echo "visual harness exit status: $app_status"
          count=$(find settings-qml-visual-report/raw -name '*.png' | wc -l)
          echo "runtime screenshot count: $count"
          test "$count" -gt 1
          python3 scripts/settings_visual_report.py --raw settings-qml-visual-report/raw --out settings-qml-visual-report

      - name: Upload real QML settings parity report
        if: always() && github.event_name == 'pull_request'
        uses: actions/upload-artifact@v4
        with:
          name: settings-qml-visual-report
          path: settings-qml-visual-report/
          if-no-files-found: warn
          retention-days: 14

      - name: Archive linux-desktop binary
''')

print("applied real QML settings visual parity harness")
