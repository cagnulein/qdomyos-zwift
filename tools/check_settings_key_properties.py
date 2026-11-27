#!/usr/bin/env python3
"""Check that every setting key added by a pull request is also a property of the persistent
Settings { id: settings } block in src/settings.qml.

A key that lives only in C++ (src/qzsettings.cpp) corrupts the settings file on iOS, so a new
QZSettings key needs a property named as the key (the text of its QStringLiteral, which is not
always the name of the constant) in settings.qml.

Only the keys added against the base ref are checked: keys already in the base are left alone.
The default values (QZSettings::default_*) are not keys. The properties are read the same way as
check_settings_property_order.py reads them.

Usage: python3 tools/check_settings_key_properties.py <base-git-ref>   (from the repository root)
"""
import subprocess
import sys

from check_settings_property_order import settings_properties

QZSETTINGS_PATH = "src/qzsettings.cpp"
SETTINGS_PATH = "src/settings.qml"
KEY_START = "const QString QZSettings::"


def read_base_file(base_ref, path):
    try:
        return subprocess.check_output(["git", "show", base_ref + ":" + path], text=True, encoding="utf-8")
    except subprocess.CalledProcessError as exc:
        raise RuntimeError("Unable to read %s from base ref %s" % (path, base_ref)) from exc


def setting_keys(text):
    """{key: line} of the keys defined in qzsettings.cpp, for example
    `const QString QZSettings::ftp_port = QStringLiteral("ftp_port");` gives "ftp_port".
    A definition may go on over the next lines up to its `;`."""
    keys = {}
    lines = text.splitlines()
    i = 0
    while i < len(lines):
        if not lines[i].lstrip().startswith(KEY_START):
            i += 1
            continue
        first = i + 1
        statement = lines[i]
        while ";" not in statement and i + 1 < len(lines):
            i += 1
            statement += " " + lines[i].strip()
        i += 1

        name = statement.split(KEY_START, 1)[1].split("=", 1)[0].strip()
        if name.startswith("default_"):
            continue
        value = statement.split("=", 1)[1]
        if 'QStringLiteral("' not in value:
            raise RuntimeError("%s:%d: cannot read the key of QZSettings::%s" % (QZSETTINGS_PATH, first, name))
        keys[value.split('QStringLiteral("', 1)[1].split('"', 1)[0]] = first
    return keys


def main():
    if len(sys.argv) != 2:
        print("Usage: %s <base-git-ref>" % sys.argv[0], file=sys.stderr)
        return 2
    base_ref = sys.argv[1]

    with open(QZSETTINGS_PATH, encoding="utf-8") as f:
        current_keys = setting_keys(f.read())
    base_keys = setting_keys(read_base_file(base_ref, QZSETTINGS_PATH))
    with open(SETTINGS_PATH, encoding="utf-8") as f:
        properties = set(settings_properties(f.read()))

    added = [key for key in current_keys if key not in base_keys]
    missing = [key for key in added if key not in properties]

    if missing:
        print("ERROR: every new QZSettings key needs a property named as the key in the "
              "Settings { id: settings } block of src/settings.qml (appended after all the existing "
              "properties), otherwise the settings file gets corrupted on iOS.", file=sys.stderr)
        for key in missing:
            print("  %s:%d: %s" % (QZSETTINGS_PATH, current_keys[key], key), file=sys.stderr)
        return 1

    if added:
        print("OK: the new settings keys are properties of src/settings.qml:")
        for key in added:
            print("  + %s" % key)
    else:
        print("OK: no new settings keys")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, RuntimeError) as exc:
        print("ERROR: %s" % exc, file=sys.stderr)
        sys.exit(2)
