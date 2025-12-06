#!/usr/bin/env python3
# -----------------------------------------------------------------------------
# QDomyos-Zwift: ANT+ Virtual Footpod Feature
# Build Utility: Python to C++ Header Converter
#
# Part of QDomyos-Zwift project: https://github.com/cagnulein/qdomyos-zwift
# Contributor(s): bassai-sho
# Development assisted by AI analysis tools
# Licensed under GPL-3.0 - see project repository for full license
#
# This is a build-time utility script. It takes a Python script as input
# and converts its content into a C++ string literal inside a header file (.h),
# allowing the Python code to be embedded directly into the final binary.
# -----------------------------------------------------------------------------

import sys
import textwrap

def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_py_file> <output_h_file>")
        return 1

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            py_code = f.read()
    except FileNotFoundError:
        print(f"Error: Input file not found at {input_file}")
        return 1

    # Simple minification
    minified_code = "\n".join(line for line in py_code.splitlines() if line.strip() and not line.strip().startswith('#'))
    
    # Escape for C++ string literal
    cpp_string = minified_code.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n')

    header_content = textwrap.dedent(f'''\
    #ifndef ANT_FOOTPOD_SCRIPT_H
    #define ANT_FOOTPOD_SCRIPT_H

    #include <string>

    const std::string ant_footpod_script = "{cpp_string}";

    #endif // ANT_FOOTPOD_SCRIPT_H
    ''')

    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(header_content)

    return 0

if __name__ == "__main__":
    sys.exit(main())