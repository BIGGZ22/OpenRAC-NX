#!/usr/bin/env python3
"""Print the progress snapshot of the campaign, from the tracked catalogues.

The snapshot never publishes a percentage and never mixes the three counts:
**placements** (how many addresses a proved body covers), **sources** (how
many authored files carry them) and **bytes** (how many retail bytes the
proved bodies cover, counted by placement).  A family of small accessors can
add many placements and few bytes; both facts are true, and they are
reported apart.

Usage:
    python3 tools/progress.py
    python3 tools/progress.py --json
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

from placement_table import join  # noqa: E402
BODIES = ROOT / "config/levels/bodies.tsv"
BODY_SOURCES = ROOT / "config/levels/body_sources.tsv"
BOOT_BODIES = ROOT / "config/boot/bodies.tsv"
BOOT_BODY_SOURCES = ROOT / "config/boot/body_sources.tsv"
BASELINE = ROOT / "config/levels/assembly_baseline.tsv"
INTERVALS = ROOT / "config/levels/code_intervals.tsv"
MODULES = ROOT / "config/levels/modules.tsv"
BOOT_SUMMARY = ROOT / "config/boot/summary.json"


def records(path: Path) -> list[dict[str, str]]:
    if not path.is_file():
        return []
    with path.open(encoding="utf-8", newline="") as handle:
        return list(csv.DictReader((line for line in handle
                                    if not line.startswith("#")), delimiter="\t"))


def snapshot() -> dict:
    """Build the snapshot from the catalogues."""
    bodies = join(BODIES, BODY_SOURCES)
    boot_bodies = join(BOOT_BODIES, BOOT_BODY_SOURCES)
    baseline = records(BASELINE)
    intervals = records(INTERVALS)
    boot_summary = json.loads(BOOT_SUMMARY.read_text(encoding="utf-8")) \
        if BOOT_SUMMARY.is_file() else {}

    placements = len(bodies) + len(boot_bodies)
    sources = len({row["source"] for row in bodies}
                  | {row["source"] for row in boot_bodies})
    bytes_by_placement = sum(int(row["size"]) for row in bodies + boot_bodies)
    bytes_distinct = 0
    seen: dict[tuple[str, str], int] = {}
    for row in bodies + boot_bodies:
        key = (row["source"], row["module"])
        if key in seen:
            continue
        seen[key] = int(row["size"])
        bytes_distinct += int(row["size"])

    per_module: dict[str, dict[str, int]] = {}
    for row in bodies + boot_bodies:
        entry = per_module.setdefault(row["module"], {"placements": 0, "bytes": 0})
        entry["placements"] += 1
        entry["bytes"] += int(row["size"])

    return {
        "status": "body_proved_program_gate_pending",
        "module_count": len(records(MODULES)),
        "boot_candidate_bytes": int(boot_summary.get("bounded_candidate_body_bytes", 0)),
        "boot_candidates": int(boot_summary.get("entry_intervals", 0)),
        "assembly_baseline_modules": sum(1 for row in baseline if row.get("exact") == "yes"),
        "assembly_baseline_bytes": sum(int(row["interval_bytes"]) for row in baseline),
        "candidate_code_bytes_by_placement": sum(int(row["interval_bytes"])
                                                 for row in intervals),
        "placements": placements,
        "sources": sources,
        "bytes_by_placement": bytes_by_placement,
        "bytes_by_source_and_module": bytes_distinct,
        "per_module": per_module,
        "overall_percent": None,
        "percent_note": ("No percentage is published: the denominator would be "
                         "a provisional candidate count, and the program-level "
                         "gate has not run yet."),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    report = snapshot()
    if args.json:
        print(json.dumps(report, indent=2))
        return 0
    print(f"modules                : {report['module_count']}")
    print(f"boot candidates        : {report['boot_candidates']} intervals, "
          f"{report['boot_candidate_bytes']} bounded body bytes")
    print(f"assembly baseline      : {report['assembly_baseline_modules']} module(s), "
          f"{report['assembly_baseline_bytes']} bytes rebuilt exactly")
    print(f"candidate code bytes   : {report['candidate_code_bytes_by_placement']} "
          "by placement")
    print(f"proved placements      : {report['placements']}")
    print(f"proved sources         : {report['sources']}")
    print(f"proved bytes           : {report['bytes_by_placement']} by placement, "
          f"{report['bytes_by_source_and_module']} by source and module")
    for module, entry in sorted(report["per_module"].items()):
        print(f"  {module:16s} {entry['placements']:4d} placement(s), "
              f"{entry['bytes']:6d} byte(s)")
    print(f"no percentage is published ({report['percent_note']})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
