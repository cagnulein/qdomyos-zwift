#!/bin/bash
# Script to build and run Qt Quick Tests

set -e

echo "Building Qt Quick Tests..."
cd "$(dirname "$0")"

qmake
make

echo ""
echo "Running Qt Quick Tests..."

# Get absolute path to test data
TEST_DATA_DIR="$PWD/test-data/training"

# Check if running in a display environment
if [ -z "$DISPLAY" ]; then
    echo "No display detected, using xvfb-run..."
    xvfb-run -a ./qml-tests "$TEST_DATA_DIR"
else
    echo "Display detected, running tests normally..."
    ./qml-tests "$TEST_DATA_DIR"
fi

echo ""
echo "Tests completed successfully!"
