#!/bin/bash

# Translate multiple language files using ollama-translate.py

set -e  # Exit on error

echo "Starting translations with gemma4:e4b model..."

python3 tools/ollama-translate.py --ts-file src/translations/qdomyos-zwift_de.ts --language German --model gemma4:e4b && \
echo "✓ German completed"

python3 tools/ollama-translate.py --ts-file src/translations/qdomyos-zwift_fr.ts --language French --model gemma4:e4b && \
echo "✓ French completed"

python3 tools/ollama-translate.py --ts-file src/translations/qdomyos-zwift_es.ts --language Spanish --model gemma4:e4b && \
echo "✓ Spanish completed"

python3 tools/ollama-translate.py --ts-file src/translations/qdomyos-zwift_pt.ts --language Portuguese --model gemma4:e4b && \
echo "✓ Portuguese completed"

python3 tools/ollama-translate.py --ts-file src/translations/qdomyos-zwift_zh_CN.ts --language "Simplified Chinese" --model gemma4:e4b && \
echo "✓ Simplified Chinese completed"

python3 tools/ollama-translate.py --ts-file src/translations/qdomyos-zwift_ja.ts --language Japanese --model gemma4:e4b && \
echo "✓ Japanese completed"

python3 tools/ollama-translate.py --ts-file src/translations/qdomyos-zwift_it.ts --language Italian --model gemma4:e4b && \
echo "✓ Italian completed"

echo "All translations completed!"
