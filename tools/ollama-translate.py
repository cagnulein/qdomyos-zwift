#!/usr/bin/env python3
import argparse
import json
import re
import time
import urllib.request
from pathlib import Path

from lxml import etree as ET


BRANDS = [
    "QZ",
    "QZ Fitness",
    "qdomyos-zwift",
    "qDomyos-Zwift",
    "qDoymos-Zwift",
    "Zwift",
    "Peloton",
    "Garmin",
    "Strava",
    "Wahoo",
    "KICKR",
    "FTMS",
    "Bluetooth",
    "BLE",
    "ANT+",
    "GPX",
    "FIT",
    "JSON",
    "Intervals.icu",
    "Apple Watch",
    "Wear OS",
]


BRAND_SET = set(BRANDS)
UNCHANGED_TEXTS = BRAND_SET | {
    "OK",
    "Watt",
}

# Domain-specific glossary for fitness/cycling terms.
# Keys are English source terms (case-sensitive), values are the preferred translation.
# Add entries here when the LLM picks the wrong word for a domain-specific concept.
GLOSSARY: dict[str, dict[str, str]] = {
    "Italian": {
        "Gears": "Marce",
        "Gear": "Marcia",
        "Gear +": "Marcia +",
        "Gear -": "Marcia -",
        "Gears +": "Marce +",
        "Gears -": "Marce -",
        "Cadence": "Cadenza",
        "Resistance": "Resistenza",
        "Inclination": "Inclinazione",
        "Incline": "Pendenza",
        "Peloton Resistance": "Resistenza Peloton",
        "Watts": "Watt",
        "Lap": "Giro",
        "Stride": "Passo",
        "Step Count": "Contapassi",
        "Ramp": "Rampa",
    },
    "French": {
        "Gears": "Vitesses",
        "Gear": "Vitesse",
        "Gear +": "Vitesse +",
        "Gear -": "Vitesse -",
        "Gears +": "Vitesses +",
        "Gears -": "Vitesses -",
        "Cadence": "Cadence",
        "Resistance": "Résistance",
        "Inclination": "Inclinaison",
        "Incline": "Pente",
        "Lap": "Tour",
    },
    "Spanish": {
        "Gears": "Marchas",
        "Gear": "Marcha",
        "Gear +": "Marcha +",
        "Gear -": "Marcha -",
        "Gears +": "Marchas +",
        "Gears -": "Marchas -",
        "Cadence": "Cadencia",
        "Resistance": "Resistencia",
        "Inclination": "Inclinación",
        "Incline": "Pendiente",
        "Lap": "Vuelta",
    },
    "German": {
        "Gears": "Gänge",
        "Gear": "Gang",
        "Gear +": "Gang +",
        "Gear -": "Gang -",
        "Gears +": "Gänge +",
        "Gears -": "Gänge -",
        "Cadence": "Kadenz",
        "Resistance": "Widerstand",
        "Inclination": "Neigung",
        "Incline": "Steigung",
        "Lap": "Runde",
    },
    "Portuguese": {
        "Gears": "Marchas",
        "Gear": "Marcha",
        "Gear +": "Marcha +",
        "Gear -": "Marcha -",
        "Gears +": "Marchas +",
        "Gears -": "Marchas -",
        "Cadence": "Cadência",
        "Resistance": "Resistência",
        "Inclination": "Inclinação",
        "Incline": "Inclinação",
        "Lap": "Volta",
    },
}


def should_skip_source(text: str) -> bool:
    s = text.strip()

    if not s:
        return True

    # Numbers, symbols, time values, percentages.
    # Examples: "-", "+", "0", "0.0", "00:00:00", "50%", "70.0"
    if re.fullmatch(r"[-+0-9:.% ]+", s):
        return True

    # Pure brand/protocol/app names should remain unchanged.
    if s.rstrip(":") in UNCHANGED_TEXTS:
        return True

    # File filters / technical patterns are often better left unchanged.
    # Example: Train Program (*.xml *.gpx)
    if re.search(r"\*\.[a-zA-Z0-9]+", s):
        return True

    return False


def normalize_for_compare(text: str) -> str:
    text = text.replace("&", "")
    text = re.sub(r"\s+", " ", text.strip())
    return text.casefold()


def has_letters(text: str) -> bool:
    return any(ch.isalpha() for ch in text)


def is_same_as_source(source: str, translation: str) -> bool:
    return normalize_for_compare(source) == normalize_for_compare(translation)


def count_accelerators(text: str) -> int:
    # Qt uses & as a keyboard accelerator marker. Escaped && means a literal &.
    count = 0
    i = 0
    while i < len(text):
        if text[i] != "&":
            i += 1
            continue
        if i + 1 < len(text) and text[i + 1] == "&":
            i += 2
            continue
        count += 1
        i += 1
    return count


def build_glossary_section(target_language: str) -> str:
    lang_glossary = GLOSSARY.get(target_language, {})
    if not lang_glossary:
        return ""
    lines = [f'- "{en}" → "{tr}"' for en, tr in lang_glossary.items()]
    return (
        "\nDomain glossary (fitness/cycling context — use these translations exactly):\n"
        + "\n".join(lines)
        + "\n"
    )


def call_ollama(
    model: str,
    source_text: str,
    target_language: str,
    timeout: int = 180,
) -> str:
    glossary_section = build_glossary_section(target_language)

    prompt = f"""You are translating UI strings for QZ, a Qt/QML fitness app.

Translate from English to {target_language}.

Rules:
- Return only the translation.
- Do not add quotes.
- Do not add explanations.
- Preserve placeholders exactly: %1, %2, %3, %4, %5, %n.
- Preserve escaped characters exactly: \\n, \\t, \\r.
- Preserve line breaks when the source has line breaks.
- Preserve HTML/XML tags exactly.
- Preserve XML/HTML entities exactly when present: &amp;, &lt;, &gt;, &quot;, &apos;.
- Preserve Qt keyboard accelerators marked by &: &Save may become &Salva or Sal&va, but must keep one & marker.
- Preserve leading and trailing spaces.
- Do not translate brand/product/protocol names: {", ".join(BRANDS)}.
- Keep the translation short and suitable for a mobile app UI.
- If the source text is already a brand name, protocol name, number, symbol, or file pattern, return it unchanged.
{glossary_section}
Source text:
{source_text}
"""

    payload = {
        "model": model,
        "prompt": prompt,
        "stream": False,
        "options": {
            "temperature": 0.1,
            "top_p": 0.9,
        },
    }

    req = urllib.request.Request(
        "http://localhost:11434/api/generate",
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST",
    )

    with urllib.request.urlopen(req, timeout=timeout) as resp:
        data = json.loads(resp.read().decode("utf-8"))

    out = data.get("response", "")

    # Strip only external whitespace added by the model, then restore leading/trailing
    # spaces according to source later in preserve_spacing().
    out = out.strip()

    # Remove accidental wrapping quotes.
    if len(out) >= 2 and (
        (out[0] == out[-1] == '"')
        or (out[0] == out[-1] == "'")
        or (out[0] == "“" and out[-1] == "”")
    ):
        out = out[1:-1]

    return out


def extract_tokens(text: str) -> list[str]:
    if not text:
        return []

    tokens = []

    # Qt placeholders.
    tokens += re.findall(r"%\d+|%n", text)

    # Escaped characters.
    tokens += re.findall(r"\\n|\\t|\\r", text)

    # HTML/XML entities. In parsed XML, entities such as &lt; may already
    # become literal < in source text depending on parser behavior, but this
    # still helps for entities that remain in text.
    tokens += re.findall(r"&amp;|&lt;|&gt;|&quot;|&apos;", text)

    # Very simple HTML/XML tags in already-decoded text.
    tokens += re.findall(r"</?[^>]+>", text)

    return tokens


def preserve_spacing(source: str, translation: str) -> str:
    # Preserve leading spaces.
    leading = len(source) - len(source.lstrip(" "))
    trailing = len(source) - len(source.rstrip(" "))

    translation = translation.strip()

    if leading:
        translation = (" " * leading) + translation

    if trailing:
        translation = translation + (" " * trailing)

    return translation


def validate_translation(source: str, translation: str) -> list[str]:
    errors = []

    if not translation.strip():
        errors.append("empty translation")

    if (
        has_letters(source)
        and not should_skip_source(source)
        and is_same_as_source(source, translation)
    ):
        errors.append("translation is identical to source")

    # Check placeholders, escaped chars and tags.
    for token in extract_tokens(source):
        if token not in translation:
            errors.append(f"missing token {token}")

    source_accelerators = count_accelerators(source)
    translation_accelerators = count_accelerators(translation)
    if source_accelerators != translation_accelerators:
        errors.append(
            f"accelerator count changed: source={source_accelerators}, translation={translation_accelerators}"
        )

    # Preserve line break count approximately.
    if source.count("\n") != translation.count("\n"):
        errors.append(
            f"line break count changed: source={source.count(chr(10))}, translation={translation.count(chr(10))}"
        )

    # Preserve leading/trailing spaces.
    if source.startswith(" ") and not translation.startswith(" "):
        errors.append("missing leading space")

    if source.endswith(" ") and not translation.endswith(" "):
        errors.append("missing trailing space")

    return errors


def existing_translation_needs_work(source: str, translation: str) -> bool:
    if not translation.strip():
        return True

    return bool(validate_translation(source, translation))


def finalize_valid_translations(root, keep_unfinished: bool) -> int:
    if keep_unfinished:
        return 0

    finalized = 0

    for message in root.findall(".//message"):
        source_el = message.find("source")
        translation_el = message.find("translation")

        if source_el is None or translation_el is None:
            continue

        if translation_el.get("type") != "unfinished":
            continue

        source_text = source_el.text or ""
        translation_text = translation_el.text or ""

        if should_skip_source(source_text):
            if not translation_text.strip():
                translation_el.text = source_text
            elif translation_text.strip() != source_text.strip():
                continue
        elif existing_translation_needs_work(source_text, translation_text):
            continue

        translation_el.attrib.pop("type", None)
        finalized += 1

    return finalized


def get_translation_element(message):
    translation_el = message.find("translation")

    if translation_el is None:
        translation_el = ET.SubElement(message, "translation")
        translation_el.set("type", "unfinished")

    return translation_el


def translate_ts_file(
    ts_file: Path,
    target_language: str,
    model: str,
    max_entries: int,
    sleep_seconds: float,
    dry_run: bool,
    overwrite_unfinished_with_text: bool,
    keep_unfinished: bool,
):
    parser = ET.XMLParser(
        remove_blank_text=False,
        resolve_entities=False,
        strip_cdata=False,
    )

    tree = ET.parse(str(ts_file), parser)
    root = tree.getroot()

    translated = 0
    finalized = 0
    skipped = 0
    failed = 0

    for message in root.findall(".//message"):
        source_el = message.find("source")
        if source_el is None:
            skipped += 1
            continue

        source_text = source_el.text or ""

        translation_el = get_translation_element(message)
        current_translation = translation_el.text or ""
        is_unfinished = translation_el.get("type") == "unfinished"

        if should_skip_source(source_text):
            if is_unfinished:
                if current_translation.strip() != source_text:
                    translation_el.text = source_text
                    translated += 1
                if not keep_unfinished:
                    translation_el.attrib.pop("type", None)
                    finalized += 1
            else:
                skipped += 1
            if max_entries and translated >= max_entries:
                break
            continue

        if not is_unfinished:
            skipped += 1
            continue

        # Translate only:
        # - empty unfinished translations
        # - unfinished translations that fail validation
        # - optionally unfinished translations that already contain text
        needs_work = existing_translation_needs_work(source_text, current_translation)
        if (
            current_translation.strip()
            and not overwrite_unfinished_with_text
            and not needs_work
        ):
            if not keep_unfinished:
                translation_el.attrib.pop("type", None)
                finalized += 1
            else:
                skipped += 1
            continue

        print(f"\n[{translated + 1}] Source:")
        print(source_text)

        try:
            result = call_ollama(
                model=model,
                source_text=source_text,
                target_language=target_language,
            )

            result = preserve_spacing(source_text, result)

            errors = validate_translation(source_text, result)

            if errors:
                print("  FAILED validation:")
                for err in errors:
                    print(f"   - {err}")
                print(f"  Model output: {result!r}")
                failed += 1
                continue

            translation_el.text = result

            if keep_unfinished:
                translation_el.set("type", "unfinished")
            else:
                translation_el.attrib.pop("type", None)

            print("  Translation:")
            print(result)

            translated += 1

            if sleep_seconds:
                time.sleep(sleep_seconds)

            if max_entries and translated >= max_entries:
                break

        except Exception as e:
            print(f"  ERROR: {e}")
            failed += 1

    finalized += finalize_valid_translations(root, keep_unfinished)

    if dry_run:
        print("\nDry run: no file written")
    else:
        tree.write(
            str(ts_file),
            encoding="utf-8",
            xml_declaration=True,
            doctype="<!DOCTYPE TS>",
            pretty_print=False,
        )
        print(f"\nWritten: {ts_file}")

    print("\nSummary")
    print(f"Translated: {translated}")
    print(f"Finalized:  {finalized}")
    print(f"Skipped:    {skipped}")
    print(f"Failed:     {failed}")


def main():
    parser = argparse.ArgumentParser(
        description="Pre-fill Qt .ts translations using local Ollama."
    )

    parser.add_argument(
        "--ts-file",
        required=True,
        help="Qt .ts file to translate, e.g. src/translations/qdomyos-zwift_it.ts",
    )

    parser.add_argument(
        "--language",
        required=True,
        help="Target language name, e.g. Italian, French, German, Spanish",
    )

    parser.add_argument(
        "--model",
        default="gemma4:e4b",
        help="Ollama model name, e.g. gemma4:e2b, gemma4:e4b, qwen3:4b",
    )

    parser.add_argument(
        "--max-entries",
        type=int,
        default=0,
        help="Max entries to translate. Use 0 for all.",
    )

    parser.add_argument(
        "--sleep",
        type=float,
        default=0.0,
        help="Sleep seconds between Ollama requests.",
    )

    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Do not write changes.",
    )

    parser.add_argument(
        "--overwrite-unfinished-with-text",
        action="store_true",
        help="Also overwrite valid unfinished translations that already contain text.",
    )

    parser.add_argument(
        "--keep-unfinished",
        action="store_true",
        help="Keep type=\"unfinished\" after writing machine translations.",
    )

    args = parser.parse_args()

    translate_ts_file(
        ts_file=Path(args.ts_file),
        target_language=args.language,
        model=args.model,
        max_entries=args.max_entries,
        sleep_seconds=args.sleep,
        dry_run=args.dry_run,
        overwrite_unfinished_with_text=args.overwrite_unfinished_with_text,
        keep_unfinished=args.keep_unfinished,
    )


if __name__ == "__main__":
    main()
