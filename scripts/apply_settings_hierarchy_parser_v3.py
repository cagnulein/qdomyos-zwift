#!/usr/bin/env python3
from pathlib import Path
p = Path('scripts/settings_hierarchy.py')
text = p.read_text(encoding='utf-8')
old = 'ACC_RE = re.compile(r"^\\s*AccordionElement\\s*\\{")'
new = 'ACC_RE = re.compile(r"^\\s*(?:AccordionElement|StaticAccordionElement|AccordionCheckElement)\\s*\\{")'
if old in text:
    text = text.replace(old, new, 1)
elif new not in text:
    raise RuntimeError('hierarchy accordion regex not found')
p.write_text(text, encoding='utf-8')
print('Expanded hierarchy parser to all legacy accordion types')
