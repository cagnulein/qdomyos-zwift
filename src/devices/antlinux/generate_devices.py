import re
import configparser
import os
import sys
import json

# Accept an optional input file path as argv[1], otherwise try common filenames
INPUT_FILE = sys.argv[1] if len(sys.argv) > 1 else None
OUTPUT_FILE = "devices.ini"

_CANDIDATES = [
    "setting.qml.txt",
    "setting.qml",
    "settings.qml",
    "settings.qml.txt",
]

# Heuristic keyword lists used to infer sections for inline model arrays
DEVICE_TYPE_KEYWORDS = {
    'Bike': ['bike', 'cycle', 'trainer', 'cycling', 'studio', 'tdf'],
    'Treadmill': ['treadmill', 'running', 'runner', 'run', 'incline'],
    'Elliptical': ['elliptical', 'ellipt'],
    'Rower': ['rower', 'rowing', 'erg', 'ftms', 'csafe']
}

BRAND_KEYWORDS = ['proform', 'nordic', 'nordictrack', 'concept', 'peloton', 'echelon', 'sole', 'iconsole', 'hydrow', 'waterrower', 'kettler', 'schwinn', 'horizon', 'toorx', 'fitplus', 'domyos']

def infer_section_from_model_content(models, context_before='', context_after=''):
    """Infer section name (Bike/Treadmill/Elliptical/Rower) from model strings and nearby context.
    Returns one of the section names or None.
    """
    text = ' '.join(models).lower() + ' ' + (context_before or '').lower() + ' ' + (context_after or '').lower()

    # Quick brand presence check — prefer arrays that look like device lists
    has_brand = any(b in text for b in BRAND_KEYWORDS)

    # Score device-type keyword occurrences
    scores = {sec: 0 for sec in DEVICE_TYPE_KEYWORDS}
    for sec, keys in DEVICE_TYPE_KEYWORDS.items():
        for k in keys:
            scores[sec] += text.count(k)

    # Choose highest scoring section when brand keywords are present or score is significant
    best = max(scores, key=scores.get)
    if scores[best] > 0 and (has_brand or scores[best] >= 2):
        return best
    return None

def log_unassigned_array_context(models, position, content, window=200):
    start = max(0, position - window)
    end = min(len(content), position + window)
    ctx = content[start:end].replace('\n', ' ')
    print('\n=== UNASSIGNED MODEL ARRAY ===')
    print('Models sample:', models[:6])
    print('Context:', ctx)
    suggestion = infer_section_from_model_content(models, ctx, '')
    if suggestion:
        print('SUGGESTED SECTION:', suggestion)
    print('=' * 50)


def is_noise_model(name):
    """Return True for names that look like UI options or non-device items we should skip."""
    if not name or not name.strip():
        return True
    n = name.strip()
    nl = n.lower()
    # explicit blacklist
    if nl in ('disabled', 'other'):
        return True
    # single numbers like '1', '2', ...
    if re.fullmatch(r"\d+", nl):
        return True
    # distance/time presets like '1 mile', '5 km', '10 km', 'half marathon', 'marathon'
    if re.fullmatch(r"\d+\s*(km|m|mile|miles)", nl):
        return True
    if 'marathon' in nl or 'half marathon' in nl:
        return True
    # short UI options (two-words common) that are non-device
    if nl in ('male', 'female', 'always', 'request'):
        return True
    # generic words that are not device names
    if len(n) <= 2:
        return True
    return False

def load_file_content(filename=None):
    # If a filename is provided, prefer it. Otherwise probe candidate names.
    candidates = []
    if filename:
        candidates.append(filename)
    if INPUT_FILE:
        candidates.append(INPUT_FILE)
    candidates.extend(_CANDIDATES)

    # Search locations: script dir, parent dirs up to 3 levels, and cwd
    base_dir = os.path.dirname(os.path.abspath(__file__))
    search_dirs = [base_dir, os.path.join(base_dir, '..'), os.path.join(base_dir, '..', '..'), os.path.join(base_dir, '..', '..', '..'), os.getcwd()]

    for fn in candidates:
        if not fn:
            continue
        if os.path.isabs(fn) and os.path.exists(fn):
            with open(fn, 'r', encoding='utf-8') as f:
                print(f"Using input file: {fn}")
                return f.read()
        for d in search_dirs:
            path = os.path.abspath(os.path.join(d, fn))
            if os.path.exists(path):
                with open(path, 'r', encoding='utf-8') as f:
                    print(f"Using input file: {path}")
                    return f.read()

    print(f"Error: Could not find any of: {', '.join(candidates)} in {', '.join(search_dirs)}")
    return None

def parse_combobox_logic(content, combo_id):
    devices = {}
    
    # Robust Model Pattern: Handles multiline and spaces
    model_pattern = rf'id:\s*{re.escape(combo_id)}.*?model:\s*\[\s*(.*?)\s*\]'
    model_match = re.search(model_pattern, content, re.DOTALL)
    
    if not model_match:
        # Try to resolve model by referencing a ListModel id elsewhere
        listmodels = build_listmodel_index(content)
        # Search for "model: someId" near the combo id
        start_index = content.find(f"id: {combo_id}")
        if start_index == -1:
            start_index = content.find(combo_id)
        if start_index != -1:
            tail = content[start_index:start_index+2000]
            m = re.search(r'model\s*:\s*([A-Za-z0-9_\.]+)', tail)
            if m:
                mid = m.group(1)
                # Attempt to resolve referenced model identifier (e.g., rootItem.bluetoothDevices)
                resolved = resolve_model_identifier(content, mid, listmodels)
                if resolved:
                    for name in resolved:
                        devices[name] = name.lower().replace(' ', '_')
                    return devices
        # Fallback: try nearby ListModel parsing
        lm = parse_listmodel_near(content, combo_id)
        if lm:
            return lm
        print(f"Warning: No model definition found for {combo_id}")
        return devices

    raw_list = model_match.group(1)
    model_list = []
    # Split by comma but respect quotes
    for item in re.split(r',\s*(?=(?:[^"]*"[^"]*")*[^"]*$)', raw_list):
        clean_name = item.strip().strip('"').strip("'")
        if clean_name:
            model_list.append(clean_name)

    # Find switch block
    start_index = content.find(f"id: {combo_id}")
    if start_index == -1: return devices
    
    # We scan a reasonable chunk, looking for the switch logic
    chunk = content[start_index:start_index + 15000] 
    
    # Flexible case pattern
    case_pattern = r'case\s+(\d+)\s*:\s*settings\.(\w+)\s*=\s*true'
    matches = re.findall(case_pattern, chunk)
    
    for index_str, variable in matches:
        index = int(index_str)
        if 0 <= index < len(model_list):
            model_name = model_list[index]
            if model_name != "Other":
                if is_noise_model(model_name):
                    continue
                devices[model_name] = variable

    # Always include inline model_list entries (slugified) so new models
    # appearing in the QML are captured even when no explicit case mapping
    # exists. Prefer existing variable names from 'devices' mapping.
    def slugify(s):
        s = s.strip()
        s = s.replace('(PFTL','').replace(')','')
        s = re.sub(r"[^A-Za-z0-9]+", '_', s)
        s = re.sub(r'_+', '_', s)
        s = s.strip('_').lower()
        return s or 'unknown'

    for idx, name in enumerate(model_list):
        if name == 'Other':
            continue
        if is_noise_model(name):
            continue
        if name in devices:
            # already mapped via case -> variable
            continue
        # create a slug id, but avoid collisions by appending index if needed
        base = slugify(name)
        candidate = base
        suffix = 1
        while candidate in devices.values():
            candidate = f"{base}_{suffix}"
            suffix += 1
        devices[name] = candidate

    return devices

def parse_accordion_switches(content, section_title_keyword):
    devices = {}
    
    # Robust Title Search
    section_start_match = re.search(
        rf'title:\s*qsTr\(["\']([^"\']*{re.escape(section_title_keyword)}[^"\']*)["\']',
        content
    )
    
    if not section_start_match:
        return devices
        
    start_pos = section_start_match.start()
    
    # Robust End Finding using Brace Counting
    brace_count = 0
    in_section = False
    section_end = len(content)
    
    # Scan forward to find the block
    for i, char in enumerate(content[start_pos:], start_pos):
        if char == '{':
            brace_count += 1
            in_section = True
        elif char == '}':
            brace_count -= 1
            if in_section and brace_count == 0:
                section_end = i
                break
                
    chunk = content[start_pos:section_end]
    
    # Parse switches inside the chunk
    # We split by 'IndicatorOnlySwitch' to handle them sequentially
    items = chunk.split("IndicatorOnlySwitch")
    
    for item in items[1:]:
        # Stop if we hit nested structures that shouldn't be here
        if "AccordionElement" in item or "NewPageElement" in item:
             # Heuristic: stop if we see a new major element start
             pass 

        name_match = re.search(r'text:\s*qsTr\(["\']([^"\']+)["\']\)', item)

        # Try several patterns to find the backing settings variable:
        # 1) property binding: checked: settings.var
        # 2) onClicked assignment: settings.var = checked
        # 3) explicit reference inside onClicked or other handlers
        var_match = None
        m1 = re.search(r'checked\s*:\s*settings\.([A-Za-z0-9_]+)', item)
        m2 = re.search(r'settings\.([A-Za-z0-9_]+)\s*=\s*checked', item)
        m3 = re.search(r'onClicked\s*:\s*\{[^}]*settings\.([A-Za-z0-9_]+)\s*=\s*checked', item, re.DOTALL)
        if m1:
            var_match = m1
        elif m2:
            var_match = m2
        elif m3:
            var_match = m3

        if name_match and var_match:
            name = name_match.group(1)
            variable = var_match.group(1)
            
            skip_keywords = ["Incline", "Resistance", "Gain", "Offset", "Log", "Debug", "Reverse", "Invert", "Unit", "Pause", "Force", "Miles"]
            if not any(x in name for x in skip_keywords):
                if is_noise_model(name):
                    continue
                devices[name] = variable

    return devices


def parse_listmodel_near(content, combo_id):
    """Attempt to find a nearby ListModel for the given combo id and
    extract display strings from ListElement blocks."""
    devices = {}
    start_index = content.find(f"id: {combo_id}")
    if start_index == -1:
        # fallback: search for combo_id anywhere
        start_index = content.find(combo_id)
        if start_index == -1:
            return devices

    chunk = content[start_index:start_index + 20000]
    # find ListModel { ... }
    lm_match = re.search(r'ListModel\s*\{(.*?)\}', chunk, re.DOTALL)
    if not lm_match:
        return devices
    lm_body = lm_match.group(1)
    # find all ListElement blocks
    for m in re.finditer(r'ListElement\s*\{(.*?)\}', lm_body, re.DOTALL):
        body = m.group(1)
        # try a set of common property names
        for keyname in ('text', 'name', 'display', 'label', 'title'):
            mm = re.search(rf'{keyname}\s*[:=]\s*"([^"]+)"', body)
            if mm:
                label = mm.group(1).strip()
                # use a synthetic id if present
                valm = re.search(r'value\s*[:=]\s*"?([A-Za-z0-9_\-]+)"?', body)
                val = valm.group(1).strip() if valm else label.lower().replace(' ', '_')
                devices[label] = val
                break
    return devices


def resolve_model_identifier(content, identifier, listmodels_index=None):
    """Given an identifier used as a ComboBox model (e.g., rootItem.bluetoothDevices or myListModel),
    attempt to resolve it to a list of display strings.
    Returns list of strings or None.
    """
    if not identifier:
        return None
    # allow dotted identifiers like rootItem.bluetoothDevices -> bluetoothDevices
    key = identifier.split('.')[-1]
    if listmodels_index is None:
        listmodels_index = build_listmodel_index(content)

    # 1) check if key matches a ListModel id
    if key in listmodels_index and listmodels_index[key]:
        return [name for name, val in listmodels_index[key]]

    # 2) look for a ListModel block whose id matches key
    lm_re = re.compile(r'ListModel\b(.*?)\{(.*?)\}', re.DOTALL)
    # fallback simple search for 'ListModel { ... id: key ... }'
    m = re.search(r'ListModel\s*\{(.*?)id\s*:\s*' + re.escape(key) + r'(.*?)\}', content, re.DOTALL)
    if m:
        block = m.group(0)
        # extract ListElement strings
        items = []
        for me in re.finditer(r'ListElement\s*\{(.*?)\}', block, re.DOTALL):
            body = me.group(1)
            mm = re.search(r'(?:text|name|display|label|title)\s*[:=]\s*"([^"]+)"', body)
            if mm:
                items.append(mm.group(1).strip())
        if items:
            return items

    # 3) look for inline array assignment: key = [ "A", "B" ] or key: [ ... ]
    arrm = re.search(r'\b' + re.escape(key) + r'\s*(?:=|:)\s*\[(.*?)\]', content, re.DOTALL)
    if arrm:
        arr_body = arrm.group(1)
        return parse_inline_model_array(arr_body)

    # 4) look for occurrences like "model: rootItem.bluetoothDevices" where bluetoothDevices is populated elsewhere
    # Search for 'bluetoothDevices' followed by '=' or ':' and then an array or ListModel
    pattern = re.compile(r'\b' + re.escape(key) + r'\b.*?(?:=|:)\s*(\[|ListModel)', re.DOTALL)
    if pattern.search(content):
        # try to find the nearest array or ListModel after the keyword
        post = content[content.find(key):content.find(key)+2000]
        arr2 = re.search(r'\[(.*?)\]', post, re.DOTALL)
        if arr2:
            return parse_inline_model_array(arr2.group(1))

    return None


def build_listmodel_index(content):
    """Scan the entire content for ListModel blocks and return a mapping
    of listmodel id -> list of (display, value) tuples."""
    res = {}
    idx = 0
    import re
    L = len(content)
    while True:
        m = re.search(r'ListModel\b', content[idx:])
        if not m:
            break
        start = idx + m.start()
        ob = content.find('{', start)
        if ob == -1:
            idx = start + 8
            continue
        brace = 0
        end = ob
        for i in range(ob, L):
            if content[i] == '{':
                brace += 1
            elif content[i] == '}':
                brace -= 1
                if brace == 0:
                    end = i
                    break
        block = content[start:end+1]
        # find id inside block
        idm = re.search(r'\bid\s*:\s*([A-Za-z_][A-Za-z0-9_]*)', block)
        lid = idm.group(1) if idm else None
        items = []
        for me in re.finditer(r'ListElement\s*\{(.*?)\}', block, re.DOTALL):
            body = me.group(1)
            name = None
            for keyname in ('text', 'name', 'display', 'label', 'title'):
                mm = re.search(rf'{keyname}\s*[:=]\s*"([^"]+)"', body)
                if mm:
                    name = mm.group(1).strip()
                    break
            if not name:
                mm = re.search(r'"([^"]+)"', body)
                if mm:
                    name = mm.group(1).strip()
            if not name:
                continue
            valm = re.search(r'value\s*[:=]\s*"?([A-Za-z0-9_\-]+)"?', body)
            val = valm.group(1).strip() if valm else name.lower().replace(' ', '_')
            items.append((name, val))
        if lid and items:
            res[lid] = items
        idx = end + 1
    return res

def main():
    content = load_file_content(INPUT_FILE)
    if not content:
        sys.exit(1)

    config = configparser.ConfigParser()
    config.optionxform = str

    print("Parsing QML file...")

    verbose = '--verbose' in sys.argv or '-v' in sys.argv

    def vprint(*args, **kwargs):
        if verbose:
            print(*args, **kwargs)

    bikes = parse_combobox_logic(content, "bikeModelComboBox")
    vprint(f"Parsed bikes: {len(bikes)} items")
    treadmills = parse_combobox_logic(content, "treadmillModelComboBox")
    vprint(f"Parsed treadmills: {len(treadmills)} items")
    ellipticals = parse_accordion_switches(content, "Elliptical Options")
    vprint(f"Parsed ellipticals: {len(ellipticals)} items")
    rowers = parse_accordion_switches(content, "Rower Options")
    vprint(f"Parsed rowers: {len(rowers)} items")

    # Manual overrides for specific Rower/Elliptical items if regex missed complex nesting
    if "Proform Sport RL" not in rowers and "proform_rower_sport_rl" in content:
        rowers["Proform Sport RL"] = "proform_rower_sport_rl"
    if "Proform 750R" not in rowers and "proform_rower_750r" in content:
        rowers["Proform 750R"] = "proform_rower_750r"

    if verbose:
        # Print any nearby contexts where these manual variables appear to help diagnosis
        for var in ("proform_rower_sport_rl", "proform_rower_750r"):
            pos = content.find(var)
            if pos != -1:
                start = max(0, pos-80)
                end = min(len(content), pos+80)
                snippet = content[start:end].replace('\n', ' ')
                print(f"Found variable '{var}' near:\n...{snippet}...\n")

    config["Bike"] = {k: v for k, v in sorted(bikes.items())}
    config["Treadmill"] = {k: v for k, v in sorted(treadmills.items())}
    config["Elliptical"] = {k: v for k, v in sorted(ellipticals.items())}
    config["Rower"] = {k: v for k, v in sorted(rowers.items())}
    
    # Add generic flags for Rowers manually as they are often logic-based in QZ
    config["Rower"]["Concept2 (CSAFE/PM3/PM4)"] = "csafe_rower_enabled"
    config["Rower"]["FTMS Generic"] = "ftms_rower_enabled"

    # Deduplicate entries by device id: keep the first display name seen for each id
    def name_score(name: str) -> int:
        # Prefer longer, multi-word, alphabetic names and penalize punctuation/digits
        if not name:
            return 0
        s = name.strip()
        alpha = sum(c.isalpha() for c in s)
        digits = sum(c.isdigit() for c in s)
        non_alnum = sum(1 for c in s if not (c.isalnum() or c.isspace()))
        spaces = s.count(' ')
        # weights tuned to prefer cleaner readable names
        return alpha * 3 + spaces * 5 + len(s) - digits * 2 - non_alnum * 4

    for section in list(config.keys()):
        id_to_best_name = {}
        # iterate in original order to bias toward earlier entries when scores tie
        for display_name, device_id in config[section].items():
            if device_id not in id_to_best_name:
                id_to_best_name[device_id] = display_name
            else:
                existing = id_to_best_name[device_id]
                if name_score(display_name) > name_score(existing):
                    id_to_best_name[device_id] = display_name

        # rebuild section mapping using preferred names
        new_section = {}
        for device_id, best_name in id_to_best_name.items():
            new_section[best_name] = device_id
        config[section] = new_section

    # ------------------------------------------------------------------
    # Global scan: pick up any inline `model: [ ... ]` arrays or ListModel
    # blocks not tied to the known combobox ids. This ensures new models
    # added to settings.qml are captured automatically.
    # ------------------------------------------------------------------
    def guess_section_from_id(identifier, context_chunk=''):
        id_low = (identifier or '').lower()
        ctx = (context_chunk or '').lower()
        if 'treadmill' in id_low or 'treadmill' in ctx or 'treadmill' in id_low:
            return 'Treadmill'
        if 'bike' in id_low or 'bikes' in ctx or 'bike' in id_low or 'bike' in ctx:
            return 'Bike'
        if 'ellipt' in id_low or 'elliptical' in ctx:
            return 'Elliptical'
        if 'rower' in id_low or 'rowers' in ctx or 'rower' in id_low:
            return 'Rower'
        return None

    def parse_inline_model_array(array_text):
        # Split respecting quotes
        items = []
        for item in re.split(r',\s*(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)', array_text):
            it = item.strip().strip('"').strip("'")
            if it:
                items.append(it)
        return items

    # find all model: [ ... ] occurrences across file
    for m in re.finditer(r'model\s*:\s*\[(.*?)\]', content, re.DOTALL):
        arr_body = m.group(1)
        # search backward a small window to find an id or label to guess section
        start = max(0, m.start() - 400)
        context = content[start:m.start()]
        idm = re.search(r'id\s*:\s*([A-Za-z0-9_]+)', context)
        identifier = idm.group(1) if idm else None
        section = guess_section_from_id(identifier, context)
        items = parse_inline_model_array(arr_body)
        # If we couldn't guess a section from id/context, use heuristics
        if not section:
            inferred = infer_section_from_model_content(items, context, content[m.end():m.end()+400])
            if inferred:
                section = inferred
            else:
                if verbose:
                    log_unassigned_array_context(items, m.start(), content)
        if section:
            for name in items:
                if section not in config:
                    config[section] = {}
                if name == 'Other' or is_noise_model(name):
                    continue
                if name not in config[section]:
                    # create slug id and avoid collisions
                    slug = re.sub(r"[^A-Za-z0-9]+", '_', name).strip('_').lower()
                    base = slug
                    i = 1
                    while slug in config[section].values():
                        slug = f"{base}_{i}"; i += 1
                    config[section][name] = slug

    # Also include any ListModel entries found globally (using build_listmodel_index)
    lm_index = build_listmodel_index(content)
    for lid, items in lm_index.items():
        # find where lid is referenced to guess section
        refpos = content.find(lid)
        section = None
        if refpos != -1:
            start = max(0, refpos - 400)
            context = content[start:refpos]
            section = guess_section_from_id(lid, context)
        if not section:
            # try to infer from nearby words
            section = 'Bike' if 'bike' in lid.lower() else None
        if not section:
            continue
        if section not in config:
            config[section] = {}
        for name, val in items:
            if is_noise_model(name):
                continue
            if name not in config[section]:
                # avoid collisions
                candidate = val
                base = candidate
                i = 1
                while candidate in config[section].values():
                    candidate = f"{base}_{i}"; i += 1
                config[section][name] = candidate

    with open(OUTPUT_FILE, 'w') as configfile:
        configfile.write(f"; Devices configuration generated from {INPUT_FILE}\n")
        config.write(configfile)

    # Also write a copy into the repository's `src` folder (helpful when the script
    # is run from repo root vs its own directory). This ensures `src/devices.ini`
    # is updated.
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_src_dir = os.path.abspath(os.path.join(script_dir, '..', '..'))
    alt_path = os.path.join(repo_src_dir, 'devices.ini')
    try:
        with open(alt_path, 'w') as f2:
            f2.write(f"; Devices configuration generated from {INPUT_FILE}\n")
            config.write(f2)
        print(f"Wrote additional devices.ini: {alt_path}")
    except Exception:
        pass

    print(f"Done! Generated {OUTPUT_FILE}")
    print(f"- {len(bikes)} Bikes")
    print(f"- {len(treadmills)} Treadmills")
    print(f"- {len(ellipticals)} Ellipticals")
    print(f"- {len(rowers)} Rowers")

    # Write optimized JSON cache
    flat_menu = []
    idx = 0
    for section in config.sections():
        for display_name, device_id in config.items(section):
            line = f"{display_name:<40} [{section}]"
            width = len(line)
            flat_menu.append({
                'line': line,
                'width': width,
                'name': display_name,
                'id': device_id,
                'category': section,
                'index': idx,
            })
            idx += 1

    cache = {'version': '1.0', 'total_devices': idx, 'flat_menu': flat_menu}
    out_json = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'devices_optimized.json')
    with open(out_json, 'w', encoding='utf-8') as jf:
        json.dump(cache, jf, separators=(',', ':'))
    print(f"Wrote optimized JSON cache: {out_json} ({idx} items)")
    # Also write per-section flat cache files for fast shell consumption
    cache_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '.menu_cache')
    try:
        os.makedirs(cache_dir, exist_ok=True)
        # Build per-section lists
        per_section = {}
        for it in flat_menu:
            sec = it.get('category', 'Other')
            per_section.setdefault(sec, []).append(it)

        for sec, items in per_section.items():
            fname = os.path.join(cache_dir, f"{sec}.cache")
            with open(fname, 'w', encoding='utf-8') as fh:
                for it in items:
                    # store as: name <US> id <US> width (width is name-length for fast rendering)
                    name = it.get('name','')
                    idv = it.get('id','')
                    width = len(name)
                    fh.write(f"{name}\x1f{idv}\x1f{width}\n")
        print(f"Wrote per-section menu caches to: {cache_dir}")
    except Exception:
        pass
    if verbose:
        print('\nSummary by section:')
        for section in config.sections():
            print(f"- {section}: {len(config[section])} entries")

        # Report any model arrays we found but couldn't assign a section for
        unassigned = []
        for m in re.finditer(r'model\s*:\s*\[(.*?)\]', content, re.DOTALL):
            start = m.start()
            # if no nearby id or hint that maps to a known section, note it
            ctx = content[max(0, start-120):start]
            if not re.search(r'(bike|treadmill|ellipt|rower)', ctx, re.IGNORECASE):
                arr = m.group(1).strip().split('\n')[0][:120]
                unassigned.append(arr)
        if unassigned:
            print('\nUnassigned model arrays (sample):')
            for s in unassigned[:10]:
                print(f"  {s.strip()}")

if __name__ == "__main__":
    main()
