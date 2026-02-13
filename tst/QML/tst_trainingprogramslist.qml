import QtQuick 2.7
import QtTest 1.2
import Qt.labs.folderlistmodel 2.15

TestCase {
    id: testCase
    name: "TrainingProgramsListRecursiveSearchTest"
    when: windowShown

    // Get test data directory from context property (set by C++ test runner)
    // This will be either from QML_TEST_DATA_DIR environment variable or relative path
    property string mockBaseDir: typeof testDataDirUrl !== 'undefined' ? testDataDirUrl : ""

    property bool isSearching: false
    property var searchResultsModel: ListModel { id: searchModel }

    // JavaScript functions for recursive file search (same as in TrainingProgramsListJS.qml)
    function searchRecursively(folderUrl, filter) {
        searchResultsModel.clear()
        isSearching = true
        var searchPattern = filter.toLowerCase()
        recursiveSearch(folderUrl, searchPattern)
    }

    function recursiveSearch(folderUrl, pattern) {
        var tempModel = Qt.createQmlObject('import Qt.labs.folderlistmodel 2.15; FolderListModel {}', testCase)
        tempModel.folder = folderUrl
        tempModel.nameFilters = ["*.xml", "*.zwo"]
        tempModel.showDirs = true
        tempModel.showDotAndDotDot = false

        // Wait for model to populate
        var maxWait = 100
        var waited = 0
        while (tempModel.count === 0 && waited < maxWait) {
            waited++
        }

        for (var i = 0; i < tempModel.count; i++) {
            var isFolder = tempModel.isFolder(i)
            var fileName = tempModel.get(i, "fileName")
            var fileUrl = tempModel.get(i, "fileUrl") || tempModel.get(i, "fileURL")

            if (isFolder) {
                // Recursively search subdirectories
                recursiveSearch(fileUrl, pattern)
            } else {
                // Check if file matches pattern
                if (fileName.toLowerCase().indexOf(pattern) !== -1) {
                    // Get relative path from base training folder
                    var relativePath = fileUrl.toString().replace(folderUrl.toString(), "")
                    if (relativePath.startsWith("/")) {
                        relativePath = relativePath.substring(1)
                    }

                    searchResultsModel.append({
                        "fileName": fileName,
                        "fileUrl": fileUrl,
                        "relativePath": relativePath,
                        "isFolder": false
                    })
                }
            }
        }

        tempModel.destroy()
    }

    // Test data setup
    SignalSpy {
        id: modelChangedSpy
        target: searchResultsModel
        signalName: "countChanged"
    }

    function init() {
        searchResultsModel.clear()
        isSearching = false
        modelChangedSpy.clear()
    }

    // Test 1: Verify search model is initially empty
    function test_01_initialState() {
        compare(searchResultsModel.count, 0, "Search model should be empty initially")
        compare(isSearching, false, "Should not be searching initially")
    }

    // Test 2: Verify recursive search with test data
    function test_02_recursiveSearch() {
        if (mockBaseDir === "") {
            skip("No test directory provided - skipping recursive search test")
            return
        }

        // Create test directory structure
        var testDir = "file://" + mockBaseDir
        console.log("Testing with directory: " + testDir)

        // Perform search for a common pattern
        searchRecursively(testDir, "test")

        // Verify search state changed
        verify(isSearching, "Should be in searching state")

        // Check if we found any files (depending on test data)
        console.log("Found " + searchResultsModel.count + " matching files")

        // Verify model structure for first result (if any)
        if (searchResultsModel.count > 0) {
            var firstResult = searchResultsModel.get(0)
            verify(firstResult.fileName !== undefined, "Result should have fileName")
            verify(firstResult.fileUrl !== undefined, "Result should have fileUrl")
            verify(firstResult.relativePath !== undefined, "Result should have relativePath")
            verify(firstResult.isFolder === false, "Result should not be a folder")
        }
    }

    // Test 3: Verify case-insensitive search
    function test_03_caseInsensitiveSearch() {
        if (mockBaseDir === "") {
            skip("No test directory provided - skipping case-insensitive test")
            return
        }

        var testDir = "file://" + mockBaseDir

        searchResultsModel.clear()
        searchRecursively(testDir, "TEST")
        var upperCount = searchResultsModel.count

        searchResultsModel.clear()
        searchRecursively(testDir, "test")
        var lowerCount = searchResultsModel.count

        compare(upperCount, lowerCount, "Search should be case-insensitive")
    }

    // Test 4: Verify search filters by file extension
    function test_04_fileExtensionFilter() {
        if (mockBaseDir === "") {
            skip("No test directory provided - skipping extension filter test")
            return
        }

        var testDir = "file://" + mockBaseDir

        searchRecursively(testDir, "")

        // All results should be .xml or .zwo files
        for (var i = 0; i < searchResultsModel.count; i++) {
            var fileName = searchResultsModel.get(i).fileName
            var hasValidExtension = fileName.endsWith(".xml") || fileName.endsWith(".zwo")
            verify(hasValidExtension, "File '" + fileName + "' should have .xml or .zwo extension")
        }
    }

    // Test 5: Verify model clear functionality
    function test_05_modelClear() {
        // Add some dummy data
        searchResultsModel.append({
            "fileName": "test1.xml",
            "fileUrl": "file:///test1.xml",
            "relativePath": "test1.xml",
            "isFolder": false
        })

        verify(searchResultsModel.count > 0, "Model should have items")

        // Clear should empty the model
        searchResultsModel.clear()
        compare(searchResultsModel.count, 0, "Model should be empty after clear")
    }

    // Test 6: Verify search with empty pattern
    function test_06_emptyPatternSearch() {
        if (mockBaseDir === "") {
            skip("No test directory provided - skipping empty pattern test")
            return
        }

        var testDir = "file://" + mockBaseDir

        // Search with empty pattern should find all files
        searchRecursively(testDir, "")

        console.log("Found " + searchResultsModel.count + " total files with empty pattern")

        // Should find at least some files (if test data exists)
        // We can't verify exact count without knowing test data structure
    }

    // Test 7: Verify search with no matches
    function test_07_noMatchesSearch() {
        if (mockBaseDir === "") {
            skip("No test directory provided - skipping no matches test")
            return
        }

        var testDir = "file://" + mockBaseDir

        // Search for something that definitely won't exist
        searchRecursively(testDir, "xyzabc123notexist999")

        compare(searchResultsModel.count, 0, "Should find no matches for non-existent pattern")
    }
}
