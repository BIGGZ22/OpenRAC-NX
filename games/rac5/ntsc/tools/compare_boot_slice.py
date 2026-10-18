#!/usr/bin/env python3
"""Compare a linked candidate's .text slice to the pinned boot executable.

This is a local witness check, not a complete-program match or progress gate.
Neither executable nor instruction bytes are printed.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from elf import inspect_elf

ROOT = Path(__file__).resolve().parent.parent


def text_slice(path: Path, address: int, size: int) -> bytes:
    if size <= 0 or address < 0:
        raise ValueError("address must be nonnegative and size must be positive")
    report = inspect_elf(path)
    section = next((entry for entry in report["sections"]
                    if entry["name"] == ".text" and entry["flags"] & 4), None)
    if section is None:
        raise ValueError(f"{path}: missing executable .text section")
    start = int(str(section["virtual_address"]), 16)
    if address < start or address + size > start + int(section["size"]):
        raise ValueError(f"{path}: requested slice lies outside executable .text")
    offset = int(section["file_offset"]) + address - start
    return path.read_bytes()[offset:offset + size]


def compare(boot: Path, candidate: Path, address: int, size: int,
            expected_boot_sha256: str | None = None) -> tuple[bool, int, str]:
    if expected_boot_sha256 is not None:
        actual = hashlib.sha256(boot.read_bytes()).hexdigest()
        if actual != expected_boot_sha256:
            raise ValueError("boot executable differs from pinned USA target")
    reference = text_slice(boot, address, size)
    rebuilt = text_slice(candidate, address, size)
    equal_count = sum(left == right for left, right in zip(reference, rebuilt))
    return rebuilt == reference, equal_count, hashlib.sha256(rebuilt).hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("boot", type=Path)
    parser.add_argument("candidate", type=Path)
    parser.add_argument("address", type=lambda value: int(value, 0))
    parser.add_argument("size", type=lambda value: int(value, 0))
    args = parser.parse_args()
    expected = json.loads((ROOT / "config/target.json").read_text(encoding="utf-8"))[
        "boot_sha256"]
    equal, count, digest = compare(args.boot, args.candidate, args.address,
                                   args.size, expected)
    print(f"boot witness 0x{args.address:08X}: {count}/{args.size} bytes equal; "
          f"candidate SHA-256 {digest}; {'exact slice' if equal else 'different slice'}")
    if not equal:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
