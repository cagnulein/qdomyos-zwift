#!/usr/bin/env python3
"""Check how the settings pages use SettingsNumberField / SettingsFormatField.

The fields only check the typed value; the row around them has to use it. For every such field:
- the OK button of its row is enabled by `<id>.valid` and writes `<id>.value` (number) or
  `<id>.text` (format field), never the raw text of a number field;
- Enter (`onAccepted`) writes `value`, and only `if (valid)` when the field has a range;
- the default value of the setting it shows passes the field's own rule (sign, decimals, range,
  format), so a fresh install never shows a red field.
And no number setting (real/int/double) is left in a plain TextField.

Usage: python3 tools/check_settings_fields.py   (from the repository root)
"""
import re
import sys

PAGES = ["src/settings.qml", "src/settings-treadmill-inclination-override.qml", "src/settings-tts.qml"]

# The same rules as SettingsFormatField.qml (time is checked only when the text is the setting itself)
FORMATS = {
    "ip": r"^$|^((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)$",
    "host": r"^$|^[A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?(\.[A-Za-z0-9]([A-Za-z0-9-]*[A-Za-z0-9])?)*(:\d{1,5})?$",
    "heightCm": r"^\d{2,3}(\.\d*)?$",
    "time": r"^\d{1,2}:[0-5]?\d:[0-5]?\d$",
}

# Rows where Enter and OK differ on purpose, with the reason
ENTER_UNLIKE_OK = {
    # OK also says "Invalid format!" for a height it cannot parse: Enter shows no toast, and the
    # heightFtIn format of the field lets no such height through
    "heightTextField",
}

errors = []


def error(path, line, msg):
    errors.append("%s:%d: %s" % (path, line, msg))


def block_end(lines, start):
    """Index of the closing brace of the QML object opened on lines[start]."""
    depth = 0
    for i in range(start, len(lines)):
        code = re.sub(r"//.*", "", lines[i])
        code = re.sub(r'"(\\.|[^"\\])*"', '""', code)
        depth += code.count("{") - code.count("}")
        if depth == 0:
            return i
    raise ValueError("unclosed block at line %d" % (start + 1))


def props_of(text):
    """Settings properties: name -> (type, default literal)."""
    out = {}
    for m in re.finditer(r"^\s*property\s+(real|int|double|string|bool)\s+(\w+)\s*:\s*(.*?)\s*(//.*)?$",
                         text, re.M):
        out[m.group(2)] = (m.group(1), m.group(3))
    return out


def number(literal):
    try:
        return float(int(literal, 0)) if re.match(r"^-?0[xX]", literal) else float(literal)
    except ValueError:
        return None


def handler(lines, start, end, name):
    """The code of `name:` (onAccepted, onClicked) at the top level of lines[start:end], also
    when it spans lines ({ ... })."""
    depth = 0
    for k in range(start, end):
        code = re.sub(r'"(\\.|[^"\\])*"', '""', re.sub(r"//.*", "", lines[k]))
        if depth == 0:
            m = re.match(r"^\s*%s:\s*(.*)$" % name, lines[k])
            if m:
                text, d = [m.group(1)], code.count("{") - code.count("}")
                while d > 0 and k + 1 < end:
                    k += 1
                    text.append(lines[k])
                    c = re.sub(r'"(\\.|[^"\\])*"', '""', re.sub(r"//.*", "", lines[k]))
                    d += c.count("{") - c.count("}")
                return "\n".join(text)
        depth += code.count("{") - code.count("}")
    return None


def actions(code, fid):
    """What a handler does, comparable between Enter and OK: its statements without the toast
    and the `if (valid)` guard (the OK button is enabled by valid), with `<id>.value` read as
    `value`."""
    code = re.sub(r"//.*", "", code)
    code = re.sub(r'toast\.show\(qsTr\("(\\.|[^"\\])*"\)\)', "", code)
    code = re.sub(r"\bif\s*\(valid\)", "", code)
    code = re.sub(r"\b%s\.(value|text)\b" % re.escape(fid), r"\1", code)
    code = re.sub(r"\s+", "", code).strip("{}")
    return sorted(s for s in code.split(";") if s.strip("{}"))


def check_page(path):
    text = open(path, encoding="utf-8").read()
    lines = text.split("\n")
    props = props_of(text)
    count = 0
    for i, line in enumerate(lines):
        kind = re.match(r"^\s*(SettingsNumberField|SettingsFormatField|TextField)\s*\{", line)
        if not kind:
            continue
        kind = kind.group(1)
        end = block_end(lines, i)
        body = lines[i + 1:end]
        get = lambda name: next((re.sub(r"^\s*%s:\s*" % name, "", x).strip()
                                 for x in body if re.match(r"^\s*%s:" % name, x)), None)
        fid = get("id")
        shown = get("text") or ""
        keys = re.findall(r"settings\.(\w+)", shown)
        types = {props[k][0] for k in keys if k in props}

        # Enter does what the OK button of the row does (it shows no toast, so a difference is
        # not seen): the same value, the same conversion, the same side effects
        accepted = handler(lines, i + 1, end, "onAccepted")
        if accepted is not None and fid and fid not in ENTER_UNLIKE_OK:
            for j in range(end + 1, min(end + 40, len(lines))):
                if re.match(r"^\s*(SettingsNumberField|SettingsFormatField|TextField)\s*\{", lines[j]):
                    break
                # the OK button: a Cancel next to a field does something else on purpose
                if re.match(r"^\s*Button\s*\{", lines[j]) and \
                        any(re.match(r'^\s*text:\s*qsTr\("OK"\)', x) for x in lines[j + 1:block_end(lines, j)]):
                    clicked = handler(lines, j + 1, block_end(lines, j), "onClicked")
                    if clicked is not None and actions(accepted, fid) != actions(clicked, fid):
                        error(path, i + 1, "%s: Enter does not do what the OK button does:\n    Enter: %s\n    OK:    %s"
                              % (fid, "; ".join(actions(accepted, fid)), "; ".join(actions(clicked, fid))))
                    break

        if kind == "TextField":
            if types & {"real", "int", "double"}:
                error(path, i + 1, "number setting %s in a plain TextField" % ", ".join(keys))
            continue
        count += 1
        if not fid:
            error(path, i + 1, "%s without an id" % kind)
            continue

        is_number = kind == "SettingsNumberField"
        has_range = get("minimum") is not None or get("maximum") is not None

        # Enter
        accepted = get("onAccepted")
        if accepted and not accepted.startswith("{"):
            if is_number and re.search(r"=\s*text\b", accepted):
                error(path, i + 1, "%s: onAccepted writes text, not value" % fid)
            if has_range and "if (valid)" not in accepted:
                error(path, i + 1, "%s: onAccepted saves without 'if (valid)' though the field has a range" % fid)

        # The OK button of the row: the first Button after the field, before the next field
        ok = None
        for j in range(end + 1, min(end + 40, len(lines))):
            if re.match(r"^\s*(SettingsNumberField|SettingsFormatField|TextField)\s*\{", lines[j]):
                break
            if re.match(r"^\s*Button\s*\{", lines[j]):
                ok = (j, lines[j + 1:block_end(lines, j)])
                break
        if ok is None:
            error(path, i + 1, "%s: no OK button after the field" % fid)
        else:
            j, button = ok
            joined = "\n".join(button)
            enabled = next((x for x in button if re.match(r"^\s*enabled:", x)), "")
            if not re.search(r"\b%s\.valid\b" % fid, enabled):
                error(path, j + 1, "%s: OK button not enabled by %s.valid" % (fid, fid))
            if is_number:
                if re.search(r"\b%s\.text\b" % fid, joined):
                    error(path, j + 1, "%s: OK button reads .text of a number field (use .value)" % fid)
                if not re.search(r"\b%s\.value\b" % fid, joined):
                    error(path, j + 1, "%s: OK button does not write %s.value" % (fid, fid))

        # The default value of a setting shown as is
        simple = re.match(r"^settings\.(\w+)$", shown)
        if not simple or simple.group(1) not in props:
            continue
        typ, literal = props[simple.group(1)]
        if is_number:
            value = number(literal)
            if value is None:
                continue
            if value < 0 and get("signed") != "true":
                error(path, i + 1, "%s: default %s is negative but the field is not signed" % (fid, literal))
            if get("decimals") == "0" and value != int(value):
                error(path, i + 1, "%s: default %s has decimals but the field takes none" % (fid, literal))
            lo, hi = number(get("minimum") or "x"), number(get("maximum") or "x")
            if (lo is not None and value < lo) or (hi is not None and value > hi):
                error(path, i + 1, "%s: default %s is outside %s..%s" % (fid, literal, lo, hi))
        else:
            fmt = (get("format") or '"time"').strip('"')
            rule = FORMATS.get(fmt)
            if rule and typ == "string" and not re.match(rule, literal.strip('"')):
                error(path, i + 1, "%s: default %s does not match format %s" % (fid, literal, fmt))
    return count


total = sum(check_page(p) for p in PAGES)
for e in errors:
    print("ERROR: " + e)
print("%d fields checked, %d errors" % (total, len(errors)))
sys.exit(1 if errors else 0)
