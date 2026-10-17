#!/usr/bin/env python3
"""Check how the settings pages use SettingsNumberField / SettingsFormatField.

The fields only check the typed value; the row around them has to use it. For every such field:
- the field shows the setting by `savedText:`, not `text:` (it knows then whether the text was
  changed - `dirty`);
- the OK button of its row is enabled by `<id>.valid` and `<id>.dirty` and writes `<id>.value`
  (number) or `<id>.text` (format field), never the raw text of a number field;
- Enter (`onAccepted`) writes `value`, and only `if (valid)` when the field has a range;
- the default value of the setting it shows passes the field's own rule (sign, decimals, range,
  format), so a fresh install never shows a red field.
And no number setting (real/int/double) is left in a plain TextField.

Usage: python3 tools/check_settings_fields.py   (from the repository root)
"""
import sys

PAGES = ["src/settings.qml", "src/settings-treadmill-inclination-override.qml", "src/settings-tts.qml"]

FIELDS = ["SettingsNumberField", "SettingsFormatField", "TextField"]
NUMBER_TYPES = {"real", "int", "double"}
PROPERTY_TYPES = NUMBER_TYPES | {"string", "bool"}
DIGITS = "0123456789"

# Rows where Enter and OK differ on purpose, with the reason
ENTER_UNLIKE_OK = {
    # OK also says "Invalid format!" for a height it cannot parse: Enter shows no toast, and the
    # heightFtIn format of the field lets no such height through
    "heightTextField",
}

errors = []


def error(path, line, msg):
    errors.append("%s:%d: %s" % (path, line, msg))


# Small text helpers

def is_word_char(c):
    return c.isalnum() or c == "_"


def is_digits(text, least, most):
    """`text` is `least` to `most` digits 0-9."""
    return least <= len(text) <= most and all(c in DIGITS for c in text)


def string_end(text, start):
    """Index just after the string literal opened by the quote at text[start], or -1 when it is
    not closed."""
    i = start + 1
    while i < len(text):
        c = text[i]
        if c == '"':
            return i + 1
        if c == "\\":
            if i + 1 >= len(text) or text[i + 1] == "\n":
                return -1
            i += 2
        else:
            i += 1
    return -1


def without_strings(text):
    """`text` with every string literal emptied to "", so braces in a text are not counted."""
    out, i = [], 0
    while i < len(text):
        end = string_end(text, i) if text[i] == '"' else -1
        if end < 0:
            out.append(text[i])
            i += 1
        else:
            out.append('""')
            i = end
    return "".join(out)


def without_comment(line):
    """The line up to a `//` comment."""
    return line.split("//", 1)[0]


def code_of(line):
    """The code of a line, without the comment and the texts."""
    return without_strings(without_comment(line))


def find_word(text, word, start=0):
    """Index of `word` in `text` not glued to letters, digits or '_' on either side, or -1."""
    i = text.find(word, start)
    while i >= 0:
        before = text[i - 1] if i > 0 else " "
        after = text[i + len(word)] if i + len(word) < len(text) else " "
        if not is_word_char(before) and not is_word_char(after):
            return i
        i = text.find(word, i + 1)
    return -1


def replace_word(text, word, new):
    """`text` with every `word` (as find_word sees it) replaced by `new`."""
    i = find_word(text, word)
    while i >= 0:
        text = text[:i] + new + text[i + len(word):]
        i = find_word(text, word, i + len(new))
    return text


def opens(line, names):
    """Which of `names` the line opens as a QML object (`Name {`), or None."""
    s = line.lstrip()
    for name in names:
        if s.startswith(name) and s[len(name):].lstrip().startswith("{"):
            return name
    return None


def after_key(line, key):
    """The text after `key:` when the line starts with it, else None."""
    s = line.lstrip()
    if s.startswith(key + ":"):
        return s[len(key) + 1:]
    return None


def setting_names(text):
    """The names after every `settings.` in the text."""
    names, i = [], text.find("settings.")
    while i >= 0:
        j = k = i + len("settings.")
        while k < len(text) and is_word_char(text[k]):
            k += 1
        if k > j:
            names.append(text[j:k])
        i = text.find("settings.", k)
    return names


# The same rules as SettingsFormatField.qml (time is checked only when the text is the setting itself)

def is_ip(text):
    """Empty, or four numbers 0-255 (no leading zeros) joined by dots."""
    if text == "":
        return True
    parts = text.split(".")
    return len(parts) == 4 and all(
        is_digits(p, 1, 3) and (p == "0" or p[0] != "0") and int(p) <= 255 for p in parts)


def is_host(text):
    """Empty, or names of letters, digits and '-' (not at the ends) joined by dots, with an
    optional :port of up to 5 digits."""
    if text == "":
        return True
    host, colon, port = text.partition(":")
    if colon and not is_digits(port, 1, 5):
        return False
    for label in host.split("."):
        if label == "" or label[0] == "-" or label[-1] == "-":
            return False
        if not all(c.isascii() and (c.isalnum() or c == "-") for c in label):
            return False
    return True


def is_height_cm(text):
    """2 or 3 digits, optionally a dot and more digits."""
    whole, _, fraction = text.partition(".")
    return is_digits(whole, 2, 3) and all(c in DIGITS for c in fraction)


def is_time(text):
    """h:m:s: hours of 1-2 digits, minutes and seconds of 1-2 digits up to 59."""
    parts = text.split(":")
    if len(parts) != 3 or not is_digits(parts[0], 1, 2):
        return False
    return all(is_digits(p, 1, 2) and (len(p) == 1 or p[0] in "012345") for p in parts[1:])


FORMATS = {"ip": is_ip, "host": is_host, "heightCm": is_height_cm, "time": is_time}


def block_end(lines, start):
    """Index of the closing brace of the QML object opened on lines[start]."""
    depth = 0
    for i in range(start, len(lines)):
        code = code_of(lines[i])
        depth += code.count("{") - code.count("}")
        if depth == 0:
            return i
    raise ValueError("unclosed block at line %d" % (start + 1))


def props_of(text):
    """Settings properties: name -> (type, default literal)."""
    out = {}
    for line in text.split("\n"):
        head, colon, default = line.partition(":")
        words = head.split()
        if colon and len(words) == 3 and words[0] == "property" and words[1] in PROPERTY_TYPES \
                and all(is_word_char(c) for c in words[2]):
            out[words[2]] = (words[1], without_comment(default).strip())
    return out


def number(literal):
    try:
        unsigned = literal[1:] if literal.startswith("-") else literal
        if unsigned[:2] in ("0x", "0X"):
            return float(int(literal, 0))
        return float(literal)
    except ValueError:
        return None


def handler(lines, start, end, name):
    """The code of `name:` (onAccepted, onClicked) at the top level of lines[start:end], also
    when it spans lines ({ ... })."""
    depth = 0
    for k in range(start, end):
        code = code_of(lines[k])
        if depth == 0:
            first = after_key(lines[k], name)
            if first is not None:
                text, d = [first.lstrip()], code.count("{") - code.count("}")
                while d > 0 and k + 1 < end:
                    k += 1
                    text.append(lines[k])
                    c = code_of(lines[k])
                    d += c.count("{") - c.count("}")
                return "\n".join(text)
        depth += code.count("{") - code.count("}")
    return None


def without_toasts(code):
    """`code` without `toast.show(qsTr("..."))` calls."""
    start = 'toast.show(qsTr('
    i = code.find(start)
    while i >= 0:
        quote = i + len(start)
        end = string_end(code, quote) if code[quote:quote + 1] == '"' else -1
        if end >= 0 and code[end:end + 2] == "))":
            code = code[:i] + code[end + 2:]
            i = code.find(start, i)
        else:
            i = code.find(start, i + 1)
    return code


def without_valid_guard(code):
    """`code` without `if (valid)` (any spaces before the bracket)."""
    i = find_word(code, "if")
    while i >= 0:
        j = i + 2
        while j < len(code) and code[j].isspace():
            j += 1
        if code.startswith("(valid)", j):
            code = code[:i] + code[j + len("(valid)"):]
            i = find_word(code, "if", i)
        else:
            i = find_word(code, "if", i + 1)
    return code


def actions(code, fid):
    """What a handler does, comparable between Enter and OK: its statements without the toast
    and the `if (valid)` guard (the OK button is enabled by valid), with `<id>.value` read as
    `value`."""
    code = "\n".join(without_comment(line) for line in code.split("\n"))
    code = without_toasts(code)
    code = without_valid_guard(code)
    code = replace_word(code, fid + ".value", "value")
    code = replace_word(code, fid + ".text", "text")
    code = "".join(code.split()).strip("{}")
    return sorted(s for s in code.split(";") if s.strip("{}"))


def writes_text(code):
    """`code` assigns `text` (`= text`)."""
    i = find_word(code, "text")
    while i >= 0:
        if code[:i].rstrip().endswith("="):
            return True
        i = find_word(code, "text", i + 1)
    return False


def check_page(path):
    text = open(path, encoding="utf-8").read()
    lines = text.split("\n")
    props = props_of(text)
    count = 0
    for i, line in enumerate(lines):
        kind = opens(line, FIELDS)
        if not kind:
            continue
        end = block_end(lines, i)
        body = lines[i + 1:end]
        get = lambda name: next((after_key(x, name).strip() for x in body if after_key(x, name) is not None), None)
        fid = get("id")
        shown = (get("text") if kind == "TextField" else get("savedText")) or ""
        keys = setting_names(shown)
        types = {props[k][0] for k in keys if k in props}

        # Enter does what the OK button of the row does (it shows no toast, so a difference is
        # not seen): the same value, the same conversion, the same side effects
        accepted = handler(lines, i + 1, end, "onAccepted")
        if accepted is not None and fid and fid not in ENTER_UNLIKE_OK:
            for j in range(end + 1, min(end + 40, len(lines))):
                if opens(lines[j], FIELDS):
                    break
                # the OK button: a Cancel next to a field does something else on purpose
                if opens(lines[j], ["Button"]) and \
                        any((after_key(x, "text") or "").lstrip().startswith('qsTr("OK")')
                            for x in lines[j + 1:block_end(lines, j)]):
                    clicked = handler(lines, j + 1, block_end(lines, j), "onClicked")
                    if clicked is not None and actions(accepted, fid) != actions(clicked, fid):
                        error(path, i + 1, "%s: Enter does not do what the OK button does:\n    Enter: %s\n    OK:    %s"
                              % (fid, "; ".join(actions(accepted, fid)), "; ".join(actions(clicked, fid))))
                    break

        if kind == "TextField":
            if types & NUMBER_TYPES:
                error(path, i + 1, "number setting %s in a plain TextField" % ", ".join(keys))
            continue
        count += 1
        if not fid:
            error(path, i + 1, "%s without an id" % kind)
            continue

        is_number = kind == "SettingsNumberField"
        has_range = get("minimum") is not None or get("maximum") is not None

        if get("text") is not None:
            error(path, i + 1, "%s: shows the setting by text:, use savedText: (the OK button "
                  "is enabled by dirty)" % fid)
        elif not shown:
            error(path, i + 1, "%s: no savedText:" % fid)

        # Enter
        accepted = get("onAccepted")
        if accepted and not accepted.startswith("{"):
            if is_number and writes_text(accepted):
                error(path, i + 1, "%s: onAccepted writes text, not value" % fid)
            if has_range and "if (valid)" not in accepted:
                error(path, i + 1, "%s: onAccepted saves without 'if (valid)' though the field has a range" % fid)

        # The OK button of the row: the first Button after the field, before the next field
        ok = None
        for j in range(end + 1, min(end + 40, len(lines))):
            if opens(lines[j], FIELDS):
                break
            if opens(lines[j], ["Button"]):
                ok = (j, lines[j + 1:block_end(lines, j)])
                break
        if ok is None:
            error(path, i + 1, "%s: no OK button after the field" % fid)
        else:
            j, button = ok
            joined = "\n".join(button)
            enabled = next((x for x in button if after_key(x, "enabled") is not None), "")
            if find_word(enabled, fid + ".valid") < 0:
                error(path, j + 1, "%s: OK button not enabled by %s.valid" % (fid, fid))
            if find_word(enabled, fid + ".dirty") < 0:
                error(path, j + 1, "%s: OK button not enabled by %s.dirty" % (fid, fid))
            if is_number:
                if find_word(joined, fid + ".text") >= 0:
                    error(path, j + 1, "%s: OK button reads .text of a number field (use .value)" % fid)
                if find_word(joined, fid + ".value") < 0:
                    error(path, j + 1, "%s: OK button does not write %s.value" % (fid, fid))

        # The default value of a setting shown as is
        name = shown[len("settings."):] if shown.startswith("settings.") else ""
        if not name or not all(is_word_char(c) for c in name) or name not in props:
            continue
        typ, literal = props[name]
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
            if rule and typ == "string" and not rule(literal.strip('"')):
                error(path, i + 1, "%s: default %s does not match format %s" % (fid, literal, fmt))
    return count


total = sum(check_page(p) for p in PAGES)
for e in errors:
    print("ERROR: " + e)
print("%d fields checked, %d errors" % (total, len(errors)))
sys.exit(1 if errors else 0)
