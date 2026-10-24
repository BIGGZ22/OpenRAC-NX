"""Split the placement tables into a placement table and a source table.

The campaign's tables grew: one row per placement repeated the hashes of the
source hundreds of times, and the repository guard refuses a tracked file over
its size limit.  The split keeps the hashes where they belong -- with the
source -- and leaves the placement table small.

Usage:
    python3 tools/split_tables.py config/levels/bodies.tsv config/levels/body_sources.tsv
"""

from __future__ import annotations

import csv
import sys
from pathlib import Path

PLACEMENT_FIELDS = ["module", "offset", "size", "source"]
SOURCE_FIELDS = ["source", "source_sha256_lf", "object_sha256", "profile",
                 "proof", "body_sha256"]
PLACEMENT_HEADER = [
    "# Placement table of the proved bodies; one row per placement.",
    "# The facts of the source are in body_sources.tsv.",
    "# proof=body: the compiled body equals the retail body at that",
    "# placement.  Promotion to a program-level match waits for the",
    "# complete-program gate.",
]
SOURCE_HEADER = [
    "# The proved sources, one row each; the placements that carry them",
    "# are in bodies.tsv.",
]


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    table = Path(sys.argv[1])
    target = Path(sys.argv[2])
    with table.open(encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader((line for line in handle
                                    if not line.startswith("#")), delimiter="\t"))
    placements: list[list[str]] = []
    sources: dict[str, list[str]] = {}
    for row in rows:
        placements.append([row.get(field, "") for field in PLACEMENT_FIELDS])
        name = row.get("source", "")
        if name and name not in sources:
            sources[name] = [row.get(field, "") for field in SOURCE_FIELDS]
    lines = PLACEMENT_HEADER + ["\t".join(PLACEMENT_FIELDS)]
    for row in placements:
        lines.append("\t".join(row))
    table.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    lines = SOURCE_HEADER + ["\t".join(SOURCE_FIELDS)]
    for name in sorted(sources):
        lines.append("\t".join(sources[name]))
    target.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print(f"{table.name}: {len(placements)} placement(s) -> {target.name}: "
          f"{len(sources)} source(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
