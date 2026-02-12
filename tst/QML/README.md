# Qt Quick Tests for QDomyos-Zwift

This directory contains Qt Quick Tests for testing QML components.

## Test Structure

- `qml_test_runner.cpp` - C++ test runner for Qt Quick Tests
- `tst_trainingprogramslist.qml` - Tests for TrainingProgramsListJS recursive search functionality
- `test-data/` - Test data directory with sample training files in various subfolders

## Building and Running Tests

### Build:
```bash
cd tst/QML
qmake
make
```

### Run:
```bash
# Run with test data directory
./qml-tests tst/QML/test-data/training

# Or with Xvfb for headless CI environments
xvfb-run -a ./qml-tests tst/QML/test-data/training
```

## Test Coverage

### tst_trainingprogramslist.qml
Tests the recursive search functionality for training programs:
- Initial state verification
- Recursive search through subdirectories
- Case-insensitive search
- File extension filtering (.xml, .zwo)
- Model clear functionality
- Empty pattern search (find all files)
- No matches search

## Test Data Structure

```
test-data/training/
├── test1.xml                      # Root level test file
├── sample.zwo                     # Root level file without "test" keyword
├── subfolder1/
│   └── test2.zwo                  # First level subfolder
└── subfolder2/
    └── nested/
        └── test3.xml              # Nested subfolder
```

This structure tests:
- Files in root directory
- Files in first-level subdirectories
- Files in nested subdirectories
- Different file extensions (.xml and .zwo)
- Different search patterns
