#!/usr/bin/env python3
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
qml_path = root / 'src/settings.qml'
cat_path = root / 'src/settings-catalog.json'
parity_path = root / 'scripts/settings_parity.py'

qml = qml_path.read_text(encoding='utf-8')
cat = json.loads(cat_path.read_text(encoding='utf-8'))
parity = parity_path.read_text(encoding='utf-8')

# 1. App Language is a real legacy ComboBox.  Preserve display labels separately
# from the stored language codes.
m = re.search(r'property var appLanguageOptions:\s*\[(.*?)\n\s*\]', qml, re.S)
if not m:
    raise SystemExit('appLanguageOptions block not found')
opts = re.findall(r'\{\s*label:\s*qsTr\("([^"]+)"\),\s*value:\s*"([^"]+)"\s*\}', m.group(1))
if len(opts) < 2:
    raise SystemExit('Could not parse appLanguageOptions')
labels = [label for label, _ in opts]
values = [value for _, value in opts]
entry = next((e for e in cat.get('settings', []) if e.get('key') == 'app_language'), None)
if not entry:
    raise SystemExit('app_language missing from catalog')
entry['control'] = 'select'
entry['options'] = {'values': values, 'labels': labels}

# 2. Modern ComboBox models display labels while writes keep the stored values.
if 'function optionLabels(entry)' not in qml:
    needle = '\n        function optionIndex(entry) {'
    helper = '''\n        function optionLabels(entry) {\n            var values = optionValues(entry)\n            if (entry.options && entry.options.labels && entry.options.labels.length === values.length)\n                return entry.options.labels\n            return values\n        }\n\n        function optionIndex(entry) {'''
    if needle not in qml:
        raise SystemExit('optionIndex insertion point not found')
    qml = qml.replace(needle, helper, 1)
qml = qml.replace('model: visible ? settingsPane.optionValues(entry) : []',
                  'model: visible ? settingsPane.optionLabels(entry) : []')
qml = qml.replace('var selectedValue = currentValue',
                  'var selectedValue = settingsPane.optionValues(entry)[currentIndex]')

# 3. Refine semantic equivalence. Legacy TextField vs modern numeric TextField is
# not a UI mismatch; custom AccordionCheckElement is a switch-like control.
if '"AccordionCheckElement": "switch",' not in parity:
    parity = parity.replace('    "CheckBox": "switch",\n',
                            '    "CheckBox": "switch",\n    "AccordionCheckElement": "switch",\n')
old = 'status = "unknown" if lv is None else ("ok" if lv == mv or (lv == "slider" and mv == "number") else "mismatch")'
new = '''\n            compatible = (\n                lv == mv\n                or ({lv, mv} <= {"text", "number"})\n                or (lv == "slider" and mv in ("number", "text"))\n            ) if lv is not None else False\n            status = "unknown" if lv is None else ("ok" if compatible else "mismatch")'''.strip('\n')
if old not in parity:
    raise SystemExit('parity status expression not found')
parity = parity.replace(old, new, 1)

qml_path.write_text(qml, encoding='utf-8')
cat_path.write_text(json.dumps(cat, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
parity_path.write_text(parity, encoding='utf-8')
print(f'Updated app_language with {len(values)} language choices')
