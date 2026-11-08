#!/usr/bin/env python3
"""Validate an externally supplied Kettler C12 SecureData.txt (not redistributed)."""

import argparse
import hashlib
import re
from pathlib import Path

SHA256 = "70d3eeec6c510e132383fa69b2466e28dec0c89065ab0481a33d191e6281b3a3"
ROW = re.compile(rb"([0-9A-Fa-f]{4}):([0-9A-Fa-f]{32})")


def validate(data: bytes) -> None:
    if hashlib.sha256(data).hexdigest() != SHA256:
        raise ValueError("SHA-256 does not match the verified MyHomeFIT 2.3.5 asset")
    rows = data.splitlines()
    if len(rows) != 65536:
        raise ValueError(f"expected 65536 rows, found {len(rows)}")
    unique_responses = set()
    for index, row in enumerate(rows):
        match = ROW.fullmatch(row)
        if not match or int(match.group(1), 16) != index:
            raise ValueError(f"invalid or missing record at index {index:04X}")
        response = bytes.fromhex(match.group(2).decode("ascii"))
        if len(response) != 16:
            raise ValueError(f"invalid response length at index {index:04X}")
        unique_responses.add(response)
    if len(unique_responses) != 65536:
        raise ValueError("duplicate security responses")
    if rows[0x3E7C] != b"3E7C:4E0E079DE0E7BA9474AAAB503C5F4CC3":
        raise ValueError("Kinomap C12 test vector mismatch")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("file", type=Path, help="path to your private SecureData.txt")
    args = parser.parse_args()
    validate(args.file.read_bytes())
    print("OK: verified SHA-256, all 65536 entries, and Kinomap C12 vector")
