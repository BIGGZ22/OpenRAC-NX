"""Publish a clearly labelled, provisional objdiff v2 ByteMatch report.

Only the accepted C numerator is treated as matched. The level denominator is
the public optimistic masked-shape scenario and must be revised as the role
and boundary audits resolve its false equivalences.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from boot_trial_gate import accepted_body_bytes

ROOT = Path(__file__).resolve().parents[1]
STATUS = ROOT / "progress/status.json"
SHAPE = ROOT / "config/levels/shape_scenario_summary.json"
OUTPUT = ROOT / "build/bytematch/SCUS_976.15/report.json"


def _measures(total: int, matched: int, units: int = 1) -> dict:
    if total <= 0 or not 0 <= matched <= total:
        raise ValueError("invalid progress byte counts")
    percent = 100 * matched / total
    return {
        "total_code": str(total),
        "matched_code": str(matched),
        "matched_code_percent": percent,
        "fuzzy_match_percent": percent,
        "total_units": units,
    }


def report(status: dict, shape: dict) -> dict:
    if (status.get("format") != "rac5-scoped-progress-v1"
            or status.get("target") != "SCUS_976.15"
            or status.get("status") != "partial_verified"
            or status.get("overall_percent") is not None
            or status.get("bytematch_objdiff_ready") is not False):
        raise ValueError("expected the scoped, provisional RAC5 snapshot")
    if (shape.get("status") != "optimistic_masked_shapes_only"
            or shape.get("module_count") != 20
            or shape.get("bytematch_ready") is not False
            or shape.get("overall_percent") is not None):
        raise ValueError("expected the provisional 20-level shape audit")
    level_total = shape["masked_max_per_module_scenario_bytes"]
    if (status["optimistic_shape_scenario"]["masked_max_per_module_bytes"]
            != level_total
            or status["levels"]["accepted_c_bytes_per_unique_source"] != 80
            or status["levels"]["accepted_c_bytes_by_placement"] != 1600
            or len(status["levels"]["placements"]) != 21):
        raise ValueError("progress and shape audit disagree")
    boot_total = status["boot"]["code_bytes"]
    boot_matched = status["boot"]["accepted_c_bytes"]
    frontend = next(row for row in status["levels"]["placements"]
                    if row["module"] == "FRONTEND.REL")
    frontend_total = frontend["candidate_code_bytes_by_placement"]
    frontend_matched = frontend["accepted_c_bytes"]
    if (frontend_matched != status["levels"]["accepted_frontend_c_bytes"]
            or status["levels"]["accepted_frontend_functions"] !=
            ["func_00002F80"]):
        raise ValueError("frontend C scope changed")
    level_matched = status["levels"]["accepted_c_bytes_per_unique_source"]
    # Linker alignment following a complete C body earns no matching credit.
    boot_accepted = accepted_body_bytes()
    if boot_total != 498900 or boot_matched != boot_accepted:
        raise ValueError("accepted boot scope changed")
    if (shape["masked_max_per_module_bytes"]
            + shape["unrepresented_bytes_kept_unique"] != level_total):
        raise ValueError("optimistic level accounting does not close")

    boot = _measures(boot_total, boot_matched)
    frontend_measures = _measures(frontend_total, frontend_matched)
    levels = _measures(level_total, level_matched)
    units = [
        {"name": "Boot .text (measured scope)", "measures": boot,
         "metadata": {"progress_categories": ["boot_measured"]}},
        {"name": "Frontend (verified C; candidate code interval)",
         "measures": frontend_measures,
         "metadata": {"progress_categories": ["frontend_candidate"]}},
        {"name": "Levels (optimistic masked-shape estimate)",
         "measures": levels,
         "metadata": {"progress_categories": ["levels_optimistic"]}},
    ]
    return {
        "version": 2,
        "measures": _measures(boot_total + frontend_total + level_total,
                              boot_matched + frontend_matched + level_matched,
                              units=3),
        "units": units,
        "categories": [
            {"id": "boot_measured", "name": "Boot .text (measured scope)",
             "measures": boot},
            {"id": "frontend_candidate",
             "name": "Frontend (verified C; candidate code interval)",
             "measures": frontend_measures},
            {"id": "levels_optimistic",
             "name": "Levels (optimistic masked-shape estimate)",
             "measures": levels},
        ],
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    if args.write == args.check:
        parser.error("choose exactly one of --write or --check")
    payload = report(json.loads(STATUS.read_text(encoding="utf-8")),
                     json.loads(SHAPE.read_text(encoding="utf-8")))
    content = json.dumps(payload, indent=2) + "\n"
    if args.write:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(content, encoding="utf-8")
    elif not OUTPUT.is_file() or OUTPUT.read_text(encoding="utf-8") != content:
        raise SystemExit("ByteMatch estimate is missing or stale")
    print(f"ByteMatch provisional estimate: {payload['measures']['matched_code']} "
          f"/ {payload['measures']['total_code']} code bytes")


if __name__ == "__main__":
    main()
