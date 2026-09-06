#!/bin/bash
# Script to update translation files
# This extracts new translatable strings from source code

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "QDomyos-Zwift Translation Updater"
echo "=================================="
echo ""

# Check if lupdate is available
if ! command -v lupdate &> /dev/null; then
    echo "ERROR: lupdate not found!"
    echo "Please install Qt development tools."
    echo ""
    echo "On Ubuntu/Debian: sudo apt-get install qttools5-dev-tools"
    echo "On macOS: brew install qt@5"
    echo "On Windows: Install Qt from qt.io"
    exit 1
fi

echo "Found lupdate: $(which lupdate)"
echo "Version: $(lupdate -version)"
echo ""

# Navigate to project root
cd "$PROJECT_ROOT"

# Update translation files
echo "Updating translation files..."
echo "Running: lupdate src/qdomyos-zwift.pri"
echo ""

lupdate src/qdomyos-zwift.pri

echo ""
echo "SUCCESS: Translation files updated successfully!"
echo ""
echo "Next steps:"
echo "1. Open a translation file in Qt Linguist:"
echo "   linguist src/translations/qdomyos-zwift_it.ts"
echo ""
echo "2. Or edit files manually in a text editor"
echo ""
echo "3. Compile translations:"
echo "   lrelease src/qdomyos-zwift.pri"
echo ""
echo "4. Rebuild the application:"
echo "   qmake && make"
