#!/usr/bin/env python3
"""Create and check the lot tables of a wave.

A lot table is a TSV with one row per target, cut from the shortlist by the
root and given to one agent.  The schema is:

    module  offset  size  bound_source  evidence  name  body_sha256  shared_modules  shared_placements

    module           text, the module the target belongs to
    offset           hexadecimal, module relative
    size             decimal bytes, a multiple of four
    bound_source     where the bound comes from ("<tool>:<kind>")
    evidence         why the entry is a candidate ("call", "pointer", "export")
    name             the retail symbol at that offset, when the module names it
    body_sha256      64 hex digits, the raw body bytes
    shared_modules   decimal, modules carrying the same body
    shared_placements decimal, placements carrying the same body

Usage:
    python3 tools/lot_table.py init work/lots/L01.tsv
    python3 tools/lot_table.py check work/lots/L01.tsv [work/lots/L02.tsv ...]
    python3 tools/lot_table.py merge work/E0/shortlist.tsv --per-module 40 --out work/lots
"""

from __future__ import annotations

import argparse
import csv
import re
import sys
from pathlib import Path

HEADER = ["module", "offset", "size", "bound_source", "evidence", "name",
          "body_sha256", "shared_modules", "shared_placements"]
SHA = re.compile(r"[0-9a-f]{64}")


def read_table(path: Path) -> list[dict]:
    with path.open(encoding="utf-8", newline="") as handle:
        reader = csv.DictReader((line for line in handle
                                 if not line.startswith("#")), delimiter="\t")
        if reader.fieldnames != HEADER:
            raise ValueError(f"{path.name}: header differs from the schema")
        return list(reader)


def check(paths: list[Path]) -> int:
    seen: dict[tuple[str, int], str] = {}
    problems = 0
    for path in paths:
        try:
            rows = read_table(path)
        except (OSError, ValueError) as error:
            print(f"{path.name}: {error}")
            problems += 1
            continue
        for number, row in enumerate(rows, 2):
            module = row["module"]
            try:
                offset = int(row["offset"], 0)
                size = int(row["size"])
            except ValueError:
                print(f"{path.name}:{number}: bad offset or size")
                problems += 1
                continue
            if offset < 0 or size <= 0 or size % 4:
                print(f"{path.name}:{number}: offset or size out of range")
                problems += 1
            if SHA.fullmatch(row["body_sha256"] or "") is None:
                print(f"{path.name}:{number}: body_sha256 is not 64 hex digits")
                problems += 1
            key = (module, offset)
            if key in seen:
                print(f"{path.name}:{number}: {module}:0x{offset:06X} is already "
                      f"owned by {seen[key]}")
                problems += 1
            else:
                seen[key] = path.name
        print(f"{path.name}: {len(rows)} targets")
    print(f"{len(seen)} distinct placements, {problems} problem(s)")
    return 1 if problems else 0


def merge(args: argparse.Namespace) -> int:
    with args.shortlist.open(encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader((line for line in handle
                                    if not line.startswith("#")), delimiter="\t"))
    grouped: dict[str, list[dict]] = {}
    for row in rows:
        grouped.setdefault(row["module"], []).append(row)
    out = args.out
    out.mkdir(parents=True, exist_ok=True)
    index = 1
    for module in sorted(grouped):
        entries = sorted(grouped[module],
                         key=lambda row: (-int(row["shared_placements"]),
                                          int(row["size"])))
        for start in range(0, len(entries), args.per_module):
            chunk = entries[start:start + args.per_module]
            path = out / f"L{index:02d}.tsv"
            with path.open("w", encoding="utf-8", newline="\n") as handle:
                writer = csv.writer(handle, delimiter="\t", lineterminator="\n")
                writer.writerow(HEADER)
                writer.writerows([[row["module"], row["offset"], row["size"],
                                   row["bound_source"], row["evidence"],
                                   row.get("name", ""), row["body_sha256"],
                                   row["shared_modules"],
                                   row["shared_placements"]] for row in chunk])
            print(f"{path.name}: {len(chunk)} targets ({module})")
            index += 1
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    init = commands.add_parser("init", help="create an empty lot table")
    init.add_argument("path", type=Path)
    verify = commands.add_parser("check", help="check one or more lot tables")
    verify.add_argument("paths", nargs="+", type=Path)
    cut = commands.add_parser("merge", help="cut a shortlist into lot tables")
    cut.add_argument("shortlist", type=Path)
    cut.add_argument("--per-module", type=int, default=40)
    cut.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    if args.command == "init":
        args.path.parent.mkdir(parents=True, exist_ok=True)
        args.path.write_text("\t".join(HEADER) + "\n", encoding="utf-8",
                             newline="\n")
        print(f"wrote {args.path}")
        return 0
    if args.command == "check":
        return check(args.paths)
    return merge(args)


if __name__ == "__main__":
    sys.exit(main())
