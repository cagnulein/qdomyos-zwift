#!/usr/bin/env python3
"""
QDomyos-Zwift: Device Database Generator (BUILD-TIME TOOL ONLY)

Part of QDomyos-Zwift: https://github.com/cagnulein/qdomyos-zwift
Contributor: bassai-sho | AI-assisted development | License: GPL-3.0

BUILD-TIME TOOL: parses settings.qml to extract device models and generates
`devices.ini`, `devices_optimized.json` and per-section caches. Not included
in runtime packages.
"""
import re
import configparser
import os
import sys
import json
import logging
from typing import Dict, List, Tuple, Optional, Set
from pathlib import Path

# Configure logging for Docker/production use
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger(__name__)

# Configuration
INPUT_FILE = sys.argv[1] if len(sys.argv) > 1 else None
OUTPUT_FILE = "devices.ini"

_CANDIDATES = [
    "setting.qml.txt",
    "setting.qml",
    "settings.qml",
    "settings.qml.txt",
]

DEVICE_TYPE_KEYWORDS = {
    'Bike': ['bike', 'cycle', 'trainer', 'cycling', 'studio', 'tdf'],
    'Treadmill': ['treadmill', 'running', 'runner', 'run', 'incline'],
    'Elliptical': ['elliptical', 'ellipt'],
    'Rower': ['rower', 'rowing', 'erg', 'ftms', 'csafe']
}

BRAND_KEYWORDS = [
    'proform', 'nordic', 'nordictrack', 'concept', 'peloton', 
    'echelon', 'sole', 'hydrow', 'waterrower', 'kettler', 
    'schwinn', 'horizon', 'toorx', 'fitplus', 'domyos', 
    'life fitness', 'iconcept', 'freemotion'
]

# Known device ID patterns from QML settings properties
KNOWN_DEVICE_IDS = {
    # Bikes
    'proform_tour_de_france_clc': 'Tour de France CLC',
    'proform_studio': 'Proform Studio Bike',
    'proform_studio_NTEX71021': 'Proform Studio Bike NTEX71021',
    'freemotion_coachbike_b22_7': 'Freemotion Coachbike B22.7',
    'proform_tdf_10': 'Proform TDF 1.0',
    'proform_bike_PFEVEX71316_1': 'TDF 1.0 PFEVEX71316.1',
    'proform_tdf_10_0': 'Proform TDF 10',
    'nordictrack_gx_2_7': 'NordicTrack GX 2.7',
    'nordictrack_GX4_5_bike': 'NordicTrack GX 4.5',
    'proform_cycle_trainer_300_ci': 'Cycle Trainer 300 CI',
    'proform_cycle_trainer_400': 'Cycle Trainer 400',
    'proform_bike_225_csx': 'Proform 225 CSX',
    'proform_bike_325_csx': 'Proform 325 CSX / Healthrider H30X',
    'proform_bike_sb': 'Proform SB',
    'nordictrack_gx_44_pro': 'Nordictrack GX 4.4 Pro',
    'proform_bike_PFEVEX71316_0': 'TDF 1.0 PFEVEX71316.0',
    'proform_xbike': 'Proform XBike',
    'proform_225_csx_PFEX32925_INT_0': 'Proform 225 CSX PFEX32925 INT.0',
    'proform_csx210': 'Proform CSX210',
    
    # Treadmills
    'norditrack_s25_treadmill': 'Nordictrack S25',
    'norditrack_s25i_treadmill': 'Nordictrack S25i',
    'nordictrack_incline_trainer_x7i': 'Nordictrack Incline Trainer x7i',
    'nordictrack_x22i': 'NordicTrack X22i',
    'nordictrack_10_treadmill': 'Nordictrack 10',
    'nordictrack_treadmill_t8_5s': 'Nordictrack T8.5s',
    'proform_2000_treadmill': 'Proform 2000 (not pro)',
    'proform_treadmill_505_cst': 'Proform 505 CST',
    'proform_8_5_treadmill': 'Proform 8.5',
    'proform_treadmill_sport_8_5': 'Proform Sport 8.5',
    'proform_pro_1000_treadmill': 'Proform Pro 1000',
    'proform_treadmill_l6_0s': 'Nordictrack L6.0S',
    'nordictrack_t65s_treadmill': 'Nordictrack T6.5S v81',
    'nordictrack_t65s_83_treadmill': 'Nordictrack T6.5S v83',
    'nordictrack_t70_treadmill': 'Nordictrack T7.0',
    'nordictrack_s20_treadmill': 'Nordictrack S20',
    'nordictrack_s30_treadmill': 'Nordictrack S30',
    'proform_treadmill_1800i': 'Proform 1800i',
    'proform_treadmill_z1300i': 'Proform/NordicTrack z1300i',
    'proform_treadmill_se': 'Proform SE',
    'proform_treadmill_cadence_lt': 'Proform Cadence LT',
    'proform_treadmill_8_0': 'Proform 8.0',
    'proform_treadmill_9_0': 'Proform 9.0',
    
    # Rowers
    'proform_rower_sport_rl': 'Proform Sport RL',
    'proform_rower_750r': 'Proform 750R',
    'csafe_rower_enabled': 'Concept2 (CSAFE/PM3/PM4)',
    'ftms_rower_enabled': 'FTMS Generic',
    
    # Ellipticals
    'proform_hybrid_trainer_xt': 'Proform Hybrid Trainer XT',
    'proform_hybrid_trainer_PFEL03815': 'Proform Hybrid Trainer PFEL03815',
    'nordictrack_elliptical_c7_5': 'Nordictrack C7.5',
    'nordictrack_se7i': 'NordicTrack Elliptical SE7i',
    'sole_elliptical_e55': 'Sole E55 elliptical',
    'iconcept_elliptical': 'iConcept elliptical',
}


def load_file_content(path: Optional[str] = None) -> Optional[str]:
    """
    Load QML file content with comprehensive fallback search strategy.
    Designed to work in Docker containers and various deployment scenarios.
    """
    search_paths = []
    
    # Priority 1: Explicit path provided
    if path and os.path.exists(path):
        logger.info(f"Loading file from explicit path: {path}")
        try:
            with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                return f.read()
        except Exception as e:
            logger.error(f"Failed to read {path}: {e}")
            return None
    
    # Priority 2: Current directory candidates
    for candidate in _CANDIDATES:
        if os.path.exists(candidate):
            search_paths.append(candidate)
    
    # Priority 3: Script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    for candidate in _CANDIDATES:
        p = os.path.join(script_dir, candidate)
        if os.path.exists(p):
            search_paths.append(p)
    
    # Priority 4: Repository structure (../../src/settings.qml)
    repo_src = os.path.abspath(os.path.join(script_dir, '..', '..', 'src', 'settings.qml'))
    if os.path.exists(repo_src):
        search_paths.append(repo_src)
    
    # Priority 5: Current working directory src/
    cwd_candidate = os.path.join(os.getcwd(), 'src', 'settings.qml')
    if os.path.exists(cwd_candidate):
        search_paths.append(cwd_candidate)
    
    # Priority 6: Common Docker mount points
    docker_paths = [
        '/app/settings.qml',
        '/app/src/settings.qml',
        '/data/settings.qml',
        '/config/settings.qml',
    ]
    for dpath in docker_paths:
        if os.path.exists(dpath):
            search_paths.append(dpath)
    
    # Try each path
    for spath in search_paths:
        try:
            logger.info(f"Attempting to load: {spath}")
            with open(spath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                logger.info(f"Successfully loaded file from: {spath} ({len(content)} bytes)")
                return content
        except Exception as e:
            logger.warning(f"Failed to read {spath}: {e}")
            continue
    
    logger.error("No QML file found in any search location")
    return None


def parse_inline_model_array(array_text: str) -> List[str]:
    """
    Parse inline JavaScript array from QML model definition.
    Handles quoted strings with proper comma separation.
    """
    if not array_text:
        return []
    
    # Split by commas not inside quotes
    parts = re.split(r',\s*(?=(?:[^"\']*["\'][^"\']*["\'])*[^"\']*$)', array_text)
    items = []
    
    for p in parts:
        p = p.strip()
        if not p:
            continue
        
        # Remove surrounding quotes
        if (p.startswith('"') and p.endswith('"')) or (p.startswith("'") and p.endswith("'")):
            p = p[1:-1]
        
        if p and p != 'Other':
            items.append(p)
    
    return items


def extract_switch_devices(content: str, section_title: str) -> Dict[str, str]:
    """
    Extract device models from IndicatorOnlySwitch patterns within a section.
    This handles the Elliptical and Rower accordion sections.
    """
    devices = {}
    
    # Find the section
    section_pattern = re.escape(section_title)
    section_matches = list(re.finditer(section_pattern, content, re.IGNORECASE))
    
    if not section_matches:
        logger.warning(f"Section not found: {section_title}")
        return devices
    
    for section_match in section_matches:
        # Extract a reasonable window around the section
        start = section_match.start()
        # Find the next major section or use a fixed window
        window_end = min(start + 5000, len(content))
        window = content[start:window_end]
        
        # Find all IndicatorOnlySwitch blocks
        switch_pattern = r'IndicatorOnlySwitch\s*\{[^}]*text:\s*qsTr\("([^"]+)"\)[^}]*checked:\s*settings\.([a-z_0-9]+)[^}]*\}'
        
        for match in re.finditer(switch_pattern, window, re.DOTALL):
            display_name = match.group(1)
            device_id = match.group(2)
            
            if not is_noise_model(display_name):
                devices[display_name] = device_id
                logger.debug(f"Found switch device: {display_name} -> {device_id}")
    
    return devices


def parse_combobox_with_mapping(content: str, combo_id: str) -> Dict[str, str]:
    """
    Enhanced ComboBox parser that extracts both the model array and 
    the corresponding settings property mappings.
    """
    devices = {}
    
    # Find the ComboBox definition
    combo_pattern = rf'id:\s*{re.escape(combo_id)}.*?model:\s*\[(.*?)\]'
    combo_match = re.search(combo_pattern, content, re.DOTALL)
    
    if not combo_match:
        logger.warning(f"ComboBox not found: {combo_id}")
        return devices
    
    # Extract model array
    model_array = combo_match.group(1)
    model_items = parse_inline_model_array(model_array)
    
    logger.info(f"Found {len(model_items)} items in {combo_id} model array")
    
    # Find the initializeModel function
    init_pattern = rf'function\s+initializeModel\(\)\s*\{{.*?var\s+selectedModel\s*=\s*(.*?);'
    # Search for initializeModel starting from the ComboBox definition to the end
    init_match = re.search(init_pattern, content[combo_match.end():], re.DOTALL)
    
    if init_match:
        # Parse the ternary chain to map indices to setting IDs
        ternary_chain = init_match.group(1)
        setting_pattern = r'settings\.([a-z_0-9]+)\s*\?\s*(\d+)'
        
        index_to_setting = {}
        for setting_match in re.finditer(setting_pattern, ternary_chain):
            setting_id = setting_match.group(1)
            index = int(setting_match.group(2))
            index_to_setting[index] = setting_id
        
        # Match model items to settings
        for idx, display_name in enumerate(model_items):
            if display_name == 'Other' or is_noise_model(display_name):
                continue
            
            device_id = None
            if idx in index_to_setting:
                device_id = index_to_setting[idx]
            else:
                # Fallback to slugified name
                device_id = slugify(display_name)
            
            devices[display_name] = device_id
            logger.debug(f"Mapped: {display_name} -> {device_id}")
    else:
        # Fallback: use slugified names
        logger.warning(f"Could not find initializeModel for {combo_id}, using slugified names")
        for item in model_items:
            if item != 'Other' and not is_noise_model(item):
                devices[item] = slugify(item)
    
    return devices


def is_noise_model(name: Optional[str]) -> bool:
    """Check if a model name is noise/placeholder."""
    if not name:
        return True
    n = name.strip().lower()
    return n in ('', 'other', 'none', 'disabled') or len(n) < 2


def slugify(name: str) -> str:
    """Convert display name to setting ID format."""
    s = re.sub(r'[^A-Za-z0-9]+', '_', name).strip('_').lower()
    return s or 'model'


def _name_score(name: str) -> int:
    """Score a name for quality (prefer longer, more descriptive names)."""
    if not name:
        return 0
    s = name.strip()
    alpha = sum(c.isalpha() for c in s)
    digits = sum(c.isdigit() for c in s)
    non_alnum = sum(1 for c in s if not (c.isalnum() or c.isspace()))
    spaces = s.count(' ')
    return alpha * 3 + spaces * 5 + len(s) - digits * 2 - non_alnum * 4


def build_config_from_content(content: str, verbose: bool = False) -> configparser.ConfigParser:
    """
    Build the configuration by parsing the QML content.
    Uses multiple parsing strategies for robustness.
    """
    config = configparser.ConfigParser()
    config.optionxform = str  # Preserve case
    
    # Parse bikes
    logger.info("Parsing bike models...")
    bikes = parse_combobox_with_mapping(content, "bikeModelComboBox")
    logger.info(f"Found {len(bikes)} bike models")
    
    # Parse treadmills
    logger.info("Parsing treadmill models...")
    treadmills = parse_combobox_with_mapping(content, "treadmillModelComboBox")
    logger.info(f"Found {len(treadmills)} treadmill models")
    
    # Parse ellipticals
    logger.info("Parsing elliptical models...")
    ellipticals = extract_switch_devices(content, "Elliptical Options")
    
    # Add known elliptical devices from KNOWN_DEVICE_IDS
    for device_id, display_name in KNOWN_DEVICE_IDS.items():
        if any(kw in device_id.lower() for kw in ['elliptical', 'ellipt']):
            if display_name not in ellipticals:
                ellipticals[display_name] = device_id
    
    logger.info(f"Found {len(ellipticals)} elliptical models")
    
    # Parse rowers
    logger.info("Parsing rower models...")
    rowers = extract_switch_devices(content, "Rower Options")
    
    # Add known rower devices
    for device_id, display_name in KNOWN_DEVICE_IDS.items():
        if any(kw in device_id.lower() for kw in ['rower', 'row']):
            if display_name not in rowers:
                rowers[display_name] = device_id
    
    # Add FTMS and CSAFE rowers explicitly
    rowers["Concept2 (CSAFE/PM3/PM4)"] = "csafe_rower_enabled"
    rowers["FTMS Generic"] = "ftms_rower_enabled"
    
    logger.info(f"Found {len(rowers)} rower models")
    
    # Build config sections
    config["Bike"] = {k: v for k, v in sorted(bikes.items())}
    config["Treadmill"] = {k: v for k, v in sorted(treadmills.items())}
    config["Elliptical"] = {k: v for k, v in sorted(ellipticals.items())}
    config["Rower"] = {k: v for k, v in sorted(rowers.items())}
    
    # Deduplicate by device_id (keep best name)
    for section in list(config.keys()):
        id_to_best_name = {}
        for display_name, device_id in config[section].items():
            if device_id not in id_to_best_name:
                id_to_best_name[device_id] = display_name
            else:
                existing = id_to_best_name[device_id]
                if _name_score(display_name) > _name_score(existing):
                    id_to_best_name[device_id] = display_name
        
        new_section = {}
        for device_id, best_name in id_to_best_name.items():
            new_section[best_name] = device_id
        config[section] = new_section
    
    return config


def write_outputs(config: configparser.ConfigParser, content_source: str, verbose: bool = False):
    """
    Write output files with error handling for Docker environments.
    """
    outputs_written = []
    
    # Write primary output
    try:
        with open(OUTPUT_FILE, 'w', encoding='utf-8') as configfile:
            configfile.write(f"; Devices configuration generated from {content_source}\n")
            config.write(configfile)
        logger.info(f"✓ Written: {OUTPUT_FILE}")
        outputs_written.append(OUTPUT_FILE)
    except Exception as e:
        logger.error(f"Failed to write {OUTPUT_FILE}: {e}")
    
    # Try alternate locations
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_src_dir = os.path.abspath(os.path.join(script_dir, '..', '..'))
    alt_path = os.path.join(repo_src_dir, 'devices.ini')
    
    try:
        with open(alt_path, 'w', encoding='utf-8') as f2:
            f2.write(f"; Devices configuration generated from {content_source}\n")
            config.write(f2)
        logger.info(f"✓ Written: {alt_path}")
        outputs_written.append(alt_path)
    except Exception as e:
        logger.debug(f"Could not write to {alt_path}: {e}")
    
    # Generate JSON cache
    flat_menu = []
    idx = 0
    for section in config.sections():
        for display_name, device_id in config.items(section):
            line = f"{display_name:<40} [{section}]"
            flat_menu.append({
                'line': line,
                'width': len(line),
                'name': display_name,
                'id': device_id,
                'category': section,
                'index': idx,
            })
            idx += 1
    
    cache = {
        'version': '1.0',
        'total_devices': idx,
        'flat_menu': flat_menu
    }
    
    out_json = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'devices_optimized.json')
    try:
        with open(out_json, 'w', encoding='utf-8') as jf:
            json.dump(cache, jf, separators=(',', ':'), indent=2)
        logger.info(f"✓ Written: {out_json} ({idx} devices)")
        outputs_written.append(out_json)
    except Exception as e:
        logger.error(f"Failed to write {out_json}: {e}")
    
    # Generate per-section caches
    cache_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '.menu_cache')
    try:
        os.makedirs(cache_dir, exist_ok=True)
        per_section = {}
        for it in flat_menu:
            sec = it.get('category', 'Other')
            per_section.setdefault(sec, []).append(it)
        
        for sec, items in per_section.items():
            fname = os.path.join(cache_dir, f"{sec}.cache")
            with open(fname, 'w', encoding='utf-8') as fh:
                for it in items:
                    name = it.get('name', '')
                    idv = it.get('id', '')
                    width = len(name)
                    fh.write(f"{name}\x1f{idv}\x1f{width}\n")
        logger.info(f"✓ Written per-section caches to: {cache_dir}")
    except Exception as e:
        logger.warning(f"Could not write section caches: {e}")
    
    # Summary
    if verbose:
        logger.info('\n=== Summary by Section ===')
        for section in config.sections():
            logger.info(f"  {section}: {len(config[section])} devices")
        logger.info(f"\nTotal outputs written: {len(outputs_written)}")


def validate_config(config: configparser.ConfigParser) -> bool:
    """Validate the generated configuration."""
    if not config.sections():
        logger.error("Configuration is empty!")
        return False
    
    total_devices = sum(len(config[section]) for section in config.sections())
    if total_devices == 0:
        logger.error("No devices found in configuration!")
        return False
    
    # Check for minimum expected devices
    min_expected = {
        'Bike': 10,
        'Treadmill': 20,
        'Elliptical': 3,
        'Rower': 2,
    }
    
    for section, min_count in min_expected.items():
        if section in config:
            count = len(config[section])
            if count < min_count:
                logger.warning(f"Section '{section}' has only {count} devices (expected at least {min_count})")
        else:
            logger.warning(f"Section '{section}' is missing!")
    
    logger.info(f"✓ Validation passed: {total_devices} total devices")
    return True


def health_check() -> bool:
    """
    Health check for Docker and monitoring systems.
    Returns True if the script can find and load a QML file.
    """
    try:
        content = load_file_content()
        if content and len(content) > 1000:
            logger.info("✓ Health check passed")
            return True
        logger.error("✗ Health check failed: file too small or empty")
        return False
    except Exception as e:
        logger.error(f"✗ Health check failed: {e}")
        return False


def main():
    """Main entry point."""
    # Handle health check
    if '--health' in sys.argv:
        sys.exit(0 if health_check() else 1)
    
    verbose = '--verbose' in sys.argv or '-v' in sys.argv
    
    if verbose:
        logger.setLevel(logging.DEBUG)
    
    logger.info("=" * 60)
    logger.info("QML Device Configuration Generator")
    logger.info("=" * 60)
    
    # Load content
    content = load_file_content(INPUT_FILE)
    if not content:
        logger.error("FAILED: No settings QML file found")
        logger.error("Please provide the settings.qml file path as an argument")
        logger.error("or place it in one of the expected locations.")
        sys.exit(1)
    
    logger.info(f"Loaded QML content: {len(content)} bytes")
    
    # Parse and build config
    try:
        config = build_config_from_content(content, verbose=verbose)
    except Exception as e:
        logger.error(f"FAILED during parsing: {e}", exc_info=True)
        sys.exit(1)
    
    # Validate
    if not validate_config(config):
        logger.error("FAILED: Configuration validation failed")
        sys.exit(1)
    
    # Write outputs
    try:
        write_outputs(config, INPUT_FILE or '<auto-detected>', verbose=verbose)
    except Exception as e:
        logger.error(f"FAILED during output: {e}", exc_info=True)
        sys.exit(1)
    
    logger.info("=" * 60)
    logger.info("✓ SUCCESS: Device configuration generated")
    logger.info("=" * 60)


if __name__ == '__main__':
    main()