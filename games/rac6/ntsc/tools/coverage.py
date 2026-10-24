#!/usr/bin/env python3
"""Measure the campaign's coverage: how much of the program comes from C.

A body of a module is **covered** when a C source has been proved for that
raw body at that placement.  The coverage is therefore measured in bodies and
in bytes, per module, and it is the number the complete-program gate needs:
the gate can only pass when every bounded candidate of every module is
covered by a C source.

The tool prints the remaining work ranked by leverage -- the bodies whose
raw form is carried by the most placements -- because those are the ones
worth translating next.

Usage:
    python3 tools/coverage.py --bounds <private dir> \\
        --tables config/levels/bodies.tsv [config/boot/bodies.tsv] [--json]
    python3 tools/coverage.py --bounds <private dir> --tables ... --complete
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

from placement_table import records, sources  # noqa: E402



def body_hasher(modules: Path, bounds: Path):
    """Return a function that hashes the retail bytes of a candidate body.

    The level modules are read at a file offset; the boot's offsets are
    virtual addresses, resolved through the section table of the
    executable.  The bounds catalogue records a hash of its own, but the
    boot's rows were hashed at the wrong place, so the tools hash the
    bodies themselves.
    """
    import hashlib

    from elf import inspect_elf

    cache: dict[str, bytes] = {}
    boot_path = modules / "modules/SCUS_976.23"
    boot_data = b""
    boot_virtual = 0
    boot_file = 0
    if boot_path.is_file():
        boot_data = boot_path.read_bytes()
        info = inspect_elf(boot_path)
        text = next(item for item in info["sections"] if item["name"] == ".text")
        boot_virtual = int(text["virtual_address"], 16)
        boot_file = int(text["file_offset"])

    def data_of(module: str) -> bytes:
        if module not in cache:
            path = modules / module
            cache[module] = path.read_bytes() if path.is_file() else b""
        return cache[module]

    def hash_of(module: str, row: dict[str, str]) -> str:
        data = data_of(module)
        if not data:
            return row.get("raw_sha256", "")
        start = int(row["start"], 16)
        size = int(row.get("body_bytes") or 0)
        if module.startswith("SCUS"):
            at = boot_file + (start - boot_virtual)
        else:
            at = start
        return hashlib.sha256(data[at:at + size]).hexdigest()

    return hash_of

def records(path: Path) -> list[dict[str, str]]:
    if not path.is_file():
        return []
    with path.open(encoding="utf-8", newline="") as handle:
        return list(csv.DictReader((line for line in handle
                                    if not line.startswith("#")), delimiter="\t"))


def bounds_rows(path: Path) -> list[dict[str, str]]:
    if not path.is_file():
        return []
    lines = [line for line in path.read_text(encoding="utf-8").splitlines()
             if line and not line.startswith("#")]
    header = lines[0].split("\t")
    return [dict(zip(header, line.split("\t"))) for line in lines[1:]]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bounds", type=Path, required=True)
    parser.add_argument("--tables", type=Path, nargs="+", required=True,
                        help="the placement tables")
    parser.add_argument("--sources", type=Path, nargs="+", required=True,
                        help="the source tables that go with them")
    parser.add_argument("--complete", action="store_true",
                        help="exit non-zero when a module is not fully covered")
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--top", type=int, default=10,
                        help="remaining bodies printed per module")
    args = parser.parse_args()

    # The raw body hashes that a C source has proved, per module, and the
    # global set: a body proved somewhere can be re-proved at the other
    # placements that carry it, so the *work* left is counted globally.
    proved: dict[str, set[str]] = defaultdict(set)
    global_proved: set[str] = set()
    for placement, source in zip(args.tables, args.sources):
        known = sources(source)
        for row in records(placement):
            facts = known.get(row.get("source", ""), {})
            if facts.get("body_sha256"):
                proved[row["module"]].add(facts["body_sha256"])
                global_proved.add(facts["body_sha256"])

    hash_of = body_hasher(args.bounds.parent / "modules", args.bounds)
    report: dict[str, object] = {"modules": {}, "complete": True}
    total_bodies = total_covered = total_bytes = total_covered_bytes = 0
    remaining_families: dict[str, int] = defaultdict(int)
    remaining_bytes: dict[str, int] = defaultdict(int)
    for path in sorted(args.bounds.glob("*.tsv")):
        if path.name == "boot.tsv":
            continue
        module = path.stem + ".REL"
        rows = [row for row in bounds_rows(path)
                if row.get("status") == "bounded_candidate"]
        if not rows:
            continue
        covered = [row for row in rows
                   if hash_of(module, row) in proved.get(module, set())]
        missing = [row for row in rows
                   if hash_of(module, row) not in proved.get(module, set())]
        for row in missing:
            digest = hash_of(module, row) or f"{module}:{row['start']}"
            if digest not in global_proved:
                remaining_families[digest] += 1
                remaining_bytes[digest] += int(row["body_bytes"])
        bytes_covered = sum(int(row["body_bytes"]) for row in covered)
        bytes_total = sum(int(row["body_bytes"]) for row in rows)
        report["modules"][module] = {
            "bodies": len(rows), "bodies_covered": len(covered),
            "bytes": bytes_total, "bytes_covered": bytes_covered,
            "complete": not missing,
        }
        total_bodies += len(rows)
        total_covered += len(covered)
        total_bytes += bytes_total
        total_covered_bytes += bytes_covered
        if missing:
            report["complete"] = False

    report["totals"] = {
        "bodies": total_bodies, "bodies_covered": total_covered,
        "bytes": total_bytes, "bytes_covered": total_covered_bytes,
        "distinct_bodies_left": len(remaining_families),
    }
    report["largest_families_left"] = [
        {"placements": count, "bytes": remaining_bytes[digest]}
        for digest, count in sorted(remaining_families.items(),
                                    key=lambda item: -item[1])[:12]]

    if args.json:
        print(json.dumps(report, indent=2))
        return 0 if (report["complete"] or not args.complete) else 1

    for module, entry in sorted(report["modules"].items()):
        mark = "complete" if entry["complete"] else "partial "
        print(f"  {module:16s} {mark} {entry['bodies_covered']:6d}/"
              f"{entry['bodies']:6d} bodies, {entry['bytes_covered']:9d}/"
              f"{entry['bytes']:9d} bytes")

    # The boot has its own bounds.
    boot_rows = [row for row in bounds_rows(args.bounds / "boot.tsv")
                 if row.get("status") == "bounded_candidate"]
    if boot_rows:
        covered = [row for row in boot_rows
                   if row.get("raw_sha256") in proved.get("SCUS_976.23", set())]
        for row in boot_rows:
            digest = row.get("raw_sha256") or f"boot:{row['start']}"
            if digest not in global_proved:
                remaining_families[digest] += 1
                remaining_bytes[digest] += int(row["body_bytes"])
        bytes_covered = sum(int(row["body_bytes"]) for row in covered)
        bytes_total = sum(int(row["body_bytes"]) for row in boot_rows)
        print(f"  {'SCUS_976.23':16s} {'complete' if len(covered) == len(boot_rows) else 'partial '}"
              f" {len(covered):6d}/{len(boot_rows):6d} bodies, "
              f"{bytes_covered:9d}/{bytes_total:9d} bytes")
        report["modules"]["SCUS_976.23"] = {
            "bodies": len(boot_rows), "bodies_covered": len(covered),
            "bytes": bytes_total, "bytes_covered": bytes_covered,
            "complete": len(covered) == len(boot_rows),
        }
        if len(covered) != len(boot_rows):
            report["complete"] = False

    totals = report["totals"]
    print(f"  {'TOTAL':16s} {totals['bodies_covered']:6d}/{totals['bodies']:6d} "
          f"bodies, {totals['bytes_covered']:9d}/{totals['bytes']:9d} bytes")
    print(f"  {totals['distinct_bodies_left']:6d} distinct bodies still to "
          "translate (a family counts once)")
    print("  the largest families left (placements, bytes):")
    for family in report["largest_families_left"][:args.top]:
        print(f"    {family['placements']:6d} placement(s), "
              f"{family['bytes']:8d} byte(s)")
    print(f"  the complete-program gate {'PASSES' if report['complete'] else 'DOES NOT PASS YET'}")
    return 0 if (report["complete"] or not args.complete) else 1


if __name__ == "__main__":
    raise SystemExit(main())
