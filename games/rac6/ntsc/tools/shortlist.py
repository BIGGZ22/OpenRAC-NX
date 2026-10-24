#!/usr/bin/env python3
"""Build the shortlist: the candidate work queue of a wave.

The shortlist is cut from the bounded candidates of the private workspace:
for every module, the rows of the bounds table whose status is
``bounded_candidate`` (a real ``jr $ra`` ends the body) and whose evidence
names a call.  Bodies are then ranked by **leverage**: how many placements of
the whole disc carry the same raw body.  One authored source can satisfy
several placements, so a shared family is worth more than a rare large body.

The shortlist is a private artifact; this tool only reads the private kit and
writes where it is told.  Its schema is the one ``tools/lot_table.py`` reads.

Usage:
    python3 tools/shortlist.py --modules <dir> --bounds <dir> --out <file>
                                [--max-size 600] [--per-module 40]
"""

from __future__ import annotations

import argparse
import csv
import hashlib
from collections import defaultdict
from pathlib import Path

FIELDS = ["module", "offset", "size", "bound_source", "evidence", "name",
          "body_sha256", "shared_modules", "shared_placements"]

# Size buckets, so that a wave spans several body sizes instead of filling up
# with eight-byte stubs.
BUCKETS = [(8, 32), (32, 64), (64, 128), (128, 256), (256, 400), (400, 601)]


def read_bounds(path: Path) -> list[dict]:
    lines = [line for line in path.read_text(encoding="utf-8").splitlines()
             if line and not line.startswith("#")]
    header = lines[0].split("\t")
    rows = []
    for line in lines[1:]:
        values = dict(zip(header, line.split("\t")))
        rows.append({
            "start": int(values["start"], 16),
            "limit": int(values["limit"], 16),
            "body_end": int(values["body_end"], 16) if values["body_end"] else None,
            "evidence": values["evidence"],
            "status": values["status"],
            "name": values.get("name", ""),
            "body_bytes": int(values["body_bytes"]),
            "raw_sha256": values.get("raw_sha256", ""),
        })
    return rows


def pick_stratified(rows: list[dict], take: int) -> list[dict]:
    """Round-robin over the size buckets, smallest first inside each bucket."""
    buckets = [[row for row in rows if low <= row["body_bytes"] < high]
               for low, high in BUCKETS]
    chosen: list[dict] = []
    cursors = [0] * len(buckets)
    while len(chosen) < take:
        progressed = False
        for index, bucket in enumerate(buckets):
            if len(chosen) >= take:
                break
            if cursors[index] < len(bucket):
                chosen.append(bucket[cursors[index]])
                cursors[index] += 1
                progressed = True
        if not progressed:
            break
    return sorted(chosen, key=lambda row: (row["body_bytes"], row["start"]))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--modules", type=Path, required=True,
                        help="directory of the extracted SNR2 modules")
    parser.add_argument("--bounds", type=Path, required=True,
                        help="directory of the per-module bounds tables")
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--max-size", type=int, default=600)
    parser.add_argument("--min-size", type=int, default=8)
    parser.add_argument("--per-module", type=int, default=40)
    parser.add_argument("--evidence", default="call",
                        help="evidence the entry must carry (default: call)")
    parser.add_argument("--exclude", type=Path, nargs="*", default=None,
                        help="placement tables: the candidates whose raw body "
                             "is already proved are left out of the queue")
    args = parser.parse_args()

    proved: set[str] = set()
    for table in args.exclude or []:
        if not table.is_file():
            continue
        with table.open(encoding="utf-8", newline="") as handle:
            for row in csv.DictReader((line for line in handle
                                       if not line.startswith("#")), delimiter="\t"):
                if row.get("body_sha256"):
                    proved.add(row["body_sha256"])

    modules = sorted(path.name for path in args.modules.glob("*.REL"))
    if not modules:
        raise SystemExit(f"no module found in {args.modules}")

    pools: dict[str, list[dict]] = {}
    for module in modules:
        table = args.bounds / (module.replace(".REL", "") + ".tsv")
        if not table.is_file():
            continue
        data = (args.modules / module).read_bytes()
        pool = []
        for row in read_bounds(table):
            if row["status"] != "bounded_candidate":
                continue
            if row.get("raw_sha256") and row["raw_sha256"] in proved:
                continue
            if args.evidence not in row["evidence"]:
                continue
            if not args.min_size <= row["body_bytes"] <= args.max_size:
                continue
            body = data[row["start"]:row["body_end"]]
            pool.append({**row, "module": module,
                         "sha256": hashlib.sha256(body).hexdigest()})
        pools[module] = pool

    # Leverage over the complete eligible set: identical raw bodies, counted
    # per placement and per module.
    placements: dict[str, int] = defaultdict(int)
    modules_of: dict[str, set[str]] = defaultdict(set)
    for module, rows in pools.items():
        for row in rows:
            placements[row["sha256"]] += 1
            modules_of[row["sha256"]].add(module)

    selected: list[dict] = []
    for module in modules:
        rows = sorted(pools.get(module, []), key=lambda row: (row["body_bytes"],
                                                               row["start"]))
        seen: set[str] = set()
        unique = []
        for row in rows:
            if row["sha256"] in seen:
                continue
            seen.add(row["sha256"])
            unique.append(row)
        selected.extend(pick_stratified(unique, args.per_module))

    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write("# Shortlist of the wave; bound source = "
                     "tools/function_inventory.py (bounded_candidate).\n")
        handle.write("# shared_* = the same raw body occurs at that many "
                     "placements of the disc.\n")
        writer = csv.DictWriter(handle, fieldnames=FIELDS, delimiter="\t",
                                lineterminator="\n")
        writer.writeheader()
        for row in selected:
            writer.writerow({
                "module": row["module"],
                "offset": f"0x{row['start']:06X}",
                "size": row["body_bytes"],
                "bound_source": "function_inventory:bounded_candidate",
                "evidence": row["evidence"],
                "name": row["name"],
                "body_sha256": row["sha256"],
                "shared_modules": len(modules_of[row["sha256"]]),
                "shared_placements": placements[row["sha256"]],
            })

    counts: dict[str, int] = defaultdict(int)
    for row in selected:
        counts[row["module"]] += 1
    print(f"{len(selected)} targets over {len(counts)} modules -> {args.out}")
    if proved:
        print(f"  {len(proved)} raw bodies are already proved and were left out")
    shared = sum(1 for row in selected if placements[row["sha256"]] > 1)
    print(f"  {shared} of {len(selected)} selected bodies also occur at "
          "another placement")
    for module in modules:
        if not counts.get(module):
            continue
        sizes = sorted(row["body_bytes"] for row in selected
                       if row["module"] == module)
        print(f"  {module:16s} {counts[module]:3d} targets  "
              f"sizes {sizes[0]:4d}..{sizes[-1]:4d}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
