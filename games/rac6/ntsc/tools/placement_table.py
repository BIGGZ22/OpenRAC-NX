#!/usr/bin/env python3
"""Read the placement tables of the campaign.

A placement says *where* a proved body lives; a source says *what* it is.
The two facts are kept in two tables, because a source carried by hundreds of
placements must not repeat its hashes hundreds of times:

- ``bodies.tsv`` -- one row per placement: module, offset, size, source;
- ``body_sources.tsv`` -- one row per source: source, source_sha256_lf,
  object_sha256, profile, proof, body_sha256.

The two tables are joined by the source name, which is the name of the file
under ``src/``.
"""

from __future__ import annotations

import csv
from pathlib import Path

PLACEMENT_FIELDS = ["module", "offset", "size", "source"]
SOURCE_FIELDS = ["source", "source_sha256_lf", "object_sha256", "profile",
                 "proof", "body_sha256"]


def records(path: Path) -> list[dict[str, str]]:
    """Read a TSV, skipping the comment lines."""
    if not path.is_file():
        return []
    with path.open(encoding="utf-8", newline="") as handle:
        return list(csv.DictReader((line for line in handle
                                    if not line.startswith("#")), delimiter="\t"))


def sources(path: Path) -> dict[str, dict[str, str]]:
    """Read the source table, by source name."""
    return {row["source"]: row for row in records(path) if row.get("source")}


def join(placement_table: Path, source_table: Path) -> list[dict[str, str]]:
    """Return the full rows: the placement, with the facts of its source."""
    known = sources(source_table)
    rows: list[dict[str, str]] = []
    for row in records(placement_table):
        source = known.get(row.get("source", ""), {})
        merged = dict(row)
        for field in SOURCE_FIELDS:
            if field != "source":
                merged[field] = source.get(field, "")
        rows.append(merged)
    return rows
