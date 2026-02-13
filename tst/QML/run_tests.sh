#!/bin/bash
# Script to build and run Qt Quick Tests

set -e

echo "Building Qt Quick Tests..."
cd "$(dirname "$0")"

qmake
make

echo ""
echo "Running Qt Quick Tests..."

# Get absolute path to test data and export as environment variable
export QML_TEST_DATA_DIR="$PWD/test-data/training"
echo "Test data directory: $QML_TEST_DATA_DIR"

# Check if running in a display environment
if [ -z "$DISPLAY" ]; then
    echo "No display detected, using xvfb-run..."
    xvfb-run -a ./qml-tests
else
    echo "Display detected, running tests normally..."
    ./qml-tests
fi

echo ""
echo "Tests completed successfully!"
