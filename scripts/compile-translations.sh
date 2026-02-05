#!/bin/bash
# Script to compile translation files (.ts → .qm)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "QDomyos-Zwift Translation Compiler"
echo "==================================="
echo ""

# Check if lrelease is available
if ! command -v lrelease &> /dev/null; then
    echo "ERROR: lrelease not found!"
    echo "Please install Qt development tools."
    echo ""
    echo "On Ubuntu/Debian: sudo apt-get install qttools5-dev-tools"
    echo "On macOS: brew install qt@5"
    echo "On Windows: Install Qt from qt.io"
    exit 1
fi

echo "Found lrelease: $(which lrelease)"
echo "Version: $(lrelease -version)"
echo ""

# Navigate to project root
cd "$PROJECT_ROOT/src"

# Compile translation files
echo "Compiling translation files..."
echo ""

for ts_file in *.ts; do
    if [ -f "$ts_file" ]; then
        qm_file="${ts_file%.ts}_it.qm"
        echo "Compiling: $ts_file → $qm_file"
        lrelease "$ts_file" -qm "$qm_file"

        if [ $? -eq 0 ]; then
            echo "✓ $qm_file created successfully"
        else
            echo "✗ Error compiling $ts_file"
            exit 1
        fi
    fi
done

echo ""
echo "✓ All translation files compiled successfully!"
echo ""
echo "Next steps:"
echo "1. Rebuild the application to embed translations:"
echo "   qmake && make"
echo ""
echo "2. Test with Italian locale:"
echo "   LC_ALL=it_IT.UTF-8 ./qdomyos-zwift"
