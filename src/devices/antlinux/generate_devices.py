import re
import configparser
import os
import sys

INPUT_FILE = "setting.qml.txt"
OUTPUT_FILE = "devices.ini"

def load_file_content(filename):
    if not os.path.exists(filename):
        filename = filename.replace(".txt", "")
        if not os.path.exists(filename):
            print(f"Error: Could not find {INPUT_FILE} or {filename}")
            return None
    with open(filename, 'r', encoding='utf-8') as f:
        return f.read()

def parse_combobox_logic(content, combo_id):
    devices = {}
    
    # Robust Model Pattern: Handles multiline and spaces
    model_pattern = rf'id:\s*{re.escape(combo_id)}.*?model:\s*\[\s*(.*?)\s*\]'
    model_match = re.search(model_pattern, content, re.DOTALL)
    
    if not model_match:
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
                devices[model_name] = variable
    
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

        name_match = re.search(r'text:\s*qsTr\("([^"]+)"\)', item)
        var_match = re.search(r'settings\.(\w+)\s*=\s*checked', item)
        
        if name_match and var_match:
            name = name_match.group(1)
            variable = var_match.group(1)
            
            skip_keywords = ["Incline", "Resistance", "Gain", "Offset", "Log", "Debug", "Reverse", "Invert", "Unit", "Pause", "Force", "Miles"]
            if not any(x in name for x in skip_keywords):
                devices[name] = variable

    return devices

def main():
    content = load_file_content(INPUT_FILE)
    if not content:
        sys.exit(1)

    config = configparser.ConfigParser()
    config.optionxform = str

    print("Parsing QML file...")

    bikes = parse_combobox_logic(content, "bikeModelComboBox")
    treadmills = parse_combobox_logic(content, "treadmillModelComboBox")
    ellipticals = parse_accordion_switches(content, "Elliptical Options")
    rowers = parse_accordion_switches(content, "Rower Options")

    # Manual overrides for specific Rower/Elliptical items if regex missed complex nesting
    if "Proform Sport RL" not in rowers and "proform_rower_sport_rl" in content:
        rowers["Proform Sport RL"] = "proform_rower_sport_rl"
    if "Proform 750R" not in rowers and "proform_rower_750r" in content:
        rowers["Proform 750R"] = "proform_rower_750r"

    config["Bike"] = {k: v for k, v in sorted(bikes.items())}
    config["Treadmill"] = {k: v for k, v in sorted(treadmills.items())}
    config["Elliptical"] = {k: v for k, v in sorted(ellipticals.items())}
    config["Rower"] = {k: v for k, v in sorted(rowers.items())}
    
    # Add generic flags for Rowers manually as they are often logic-based in QZ
    config["Rower"]["Concept2 (CSAFE/PM3/PM4)"] = "csafe_rower_enabled"
    config["Rower"]["FTMS Generic"] = "ftms_rower_enabled"

    with open(OUTPUT_FILE, 'w') as configfile:
        configfile.write(f"; Devices configuration generated from {INPUT_FILE}\n")
        config.write(configfile)

    print(f"Done! Generated {OUTPUT_FILE}")
    print(f"- {len(bikes)} Bikes")
    print(f"- {len(treadmills)} Treadmills")
    print(f"- {len(ellipticals)} Ellipticals")
    print(f"- {len(rowers)} Rowers")

if __name__ == "__main__":
    main()
