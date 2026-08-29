#!/usr/bin/env python3
"""
Auto-translate unfinished Qt TS entries using a free translation endpoint.

Default provider: MyMemory (no API key required, rate-limited).
"""

import argparse
import json
import re
import sys
import time
from pathlib import Path
from typing import Dict, Tuple
from urllib.parse import quote_plus
from urllib.request import Request, urlopen
import xml.etree.ElementTree as ET


PLACEHOLDER_RE = re.compile(r"%\d+|%n|%L\d+")
ONLY_SYMBOLS_RE = re.compile(r"^[\s\d\W_]+$")


def protect_placeholders(text: str) -> Tuple[str, Dict[str, str]]:
    placeholders: Dict[str, str] = {}

    def repl(match: re.Match) -> str:
        token = f"__PH_{len(placeholders)}__"
        placeholders[token] = match.group(0)
        return token

    protected = PLACEHOLDER_RE.sub(repl, text)
    return protected, placeholders


def restore_placeholders(text: str, placeholders: Dict[str, str]) -> str:
    for token, value in placeholders.items():
        text = text.replace(token, value)
    return text


def mymemory_translate(text: str, src_lang: str, dst_lang: str, timeout: int = 20) -> str:
    url = (
        "https://api.mymemory.translated.net/get"
        f"?q={quote_plus(text)}&langpair={quote_plus(src_lang)}|{quote_plus(dst_lang)}"
    )
    req = Request(url, headers={"User-Agent": "qdomyos-zwift-i18n-bot/1.0"})
    with urlopen(req, timeout=timeout) as resp:
        payload = json.loads(resp.read().decode("utf-8", errors="replace"))
    return payload.get("responseData", {}).get("translatedText", "") or ""


def normalize_lang_for_provider(lang: str) -> str:
    # MyMemory expects generic codes for best compatibility.
    return lang.split("_", 1)[0]


def should_skip_source(source: str) -> bool:
    if not source.strip():
        return True
    if ONLY_SYMBOLS_RE.match(source):
        return True
    return False


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--ts-file", required=True, help="Path to target .ts file")
    parser.add_argument("--source-lang", default="en")
    parser.add_argument("--target-lang", required=True, help="Target language code (e.g. it, fr, de, es)")
    parser.add_argument("--sleep", type=float, default=0.25, help="Sleep between API calls")
    parser.add_argument("--max-entries", type=int, default=0, help="0 = no limit")
    parser.add_argument("--cache-file", default="", help="Optional cache json path")
    parser.add_argument("--progress-every", type=int, default=200, help="Print progress every N processed entries")
    args = parser.parse_args()

    ts_path = Path(args.ts_file)
    if not ts_path.exists():
        print(f"ERROR: file not found: {ts_path}")
        return 2

    cache_path = Path(args.cache_file) if args.cache_file else None
    cache: Dict[str, str] = {}
    if cache_path and cache_path.exists():
        cache = json.loads(cache_path.read_text(encoding="utf-8"))

    tree = ET.parse(ts_path)
    root = tree.getroot()

    src_lang = normalize_lang_for_provider(args.source_lang)
    dst_lang = normalize_lang_for_provider(args.target_lang)

    translated = 0
    skipped = 0
    failed = 0
    processed = 0

    for msg in root.findall(".//message"):
        src_el = msg.find("source")
        tr_el = msg.find("translation")
        if src_el is None or tr_el is None:
            continue

        unfinished = tr_el.attrib.get("type") == "unfinished" or not (tr_el.text or "").strip()
        if not unfinished:
            continue

        source = src_el.text or ""
        processed += 1
        if should_skip_source(source):
            skipped += 1
            if args.progress_every > 0 and processed % args.progress_every == 0:
                print(
                    f"progress file={ts_path.name} processed={processed} translated={translated} "
                    f"skipped={skipped} failed={failed}",
                    flush=True,
                )
            continue

        if args.max_entries and translated >= args.max_entries:
            break

        protected, placeholders = protect_placeholders(source)

        if protected in cache:
            out = cache[protected]
        else:
            try:
                out = mymemory_translate(protected, src_lang, dst_lang)
                if out:
                    cache[protected] = out
                time.sleep(args.sleep)
            except Exception:
                failed += 1
                if args.progress_every > 0 and processed % args.progress_every == 0:
                    print(
                        f"progress file={ts_path.name} processed={processed} translated={translated} "
                        f"skipped={skipped} failed={failed}",
                        flush=True,
                    )
                continue

        out = restore_placeholders(out, placeholders).strip()
        if not out or out == source:
            failed += 1
            if args.progress_every > 0 and processed % args.progress_every == 0:
                print(
                    f"progress file={ts_path.name} processed={processed} translated={translated} "
                    f"skipped={skipped} failed={failed}",
                    flush=True,
                )
            continue

        tr_el.text = out
        tr_el.attrib.pop("type", None)
        translated += 1
        if args.progress_every > 0 and processed % args.progress_every == 0:
            print(
                f"progress file={ts_path.name} processed={processed} translated={translated} "
                f"skipped={skipped} failed={failed}",
                flush=True,
            )

    tree.write(ts_path, encoding="utf-8", xml_declaration=True)

    if cache_path:
        cache_path.parent.mkdir(parents=True, exist_ok=True)
        cache_path.write_text(json.dumps(cache, ensure_ascii=False, indent=2), encoding="utf-8")

    print(
        f"done file={ts_path} translated={translated} skipped={skipped} failed={failed} "
        f"target={args.target_lang}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
