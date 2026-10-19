#!/usr/bin/env python3
"""Run the pinned complete-boot C gate with private inputs.

This checks compiler/object identities and the linked retail load image for the
explicitly accepted boot placements. It does not identify the original compiler
or publish a ByteMatch report.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
from pathlib import Path

from boot_c_campaign import REPLACEMENTS, accepted_replacements
from boot_promotion_audit import (catalogue_rows, input_fingerprint,
                                  verified_linker_alignment)
from compile_boot_trial import PROFILE, add_extra_arguments, compile_objects
from elf import inspect_elf

ROOT = Path(__file__).resolve().parent.parent
EXPORT = ROOT / "build/boot-compiled-objects"
REPORT = ROOT / "build/boot-trial-gate/report.json"


def witness_count() -> int:
    """Linked C witness bodies, from the catalogue boot_witness_boundaries writes."""
    return len(catalogue_rows())


def accepted_matches() -> list[dict]:
    """One catalogued body per accepted source, filling its owned range exactly.

    The counted bytes come from each source's owned placement, so an accepted
    source that covers only a prefix or ends in something other than a normal
    return keeps no acceptance. Verified linker alignment stays uncounted.
    """
    rows = catalogue_rows()
    matches = []
    for owner in accepted_replacements():
        span = [row for row in rows if owner.start <= row["start"] < owner.end]
        if (len(span) != 1
                or span[0]["start"] != owner.start
                or not verified_linker_alignment(span[0], owner.end)
                or span[0]["exit"] != "jr_ra"):
            raise ValueError("accepted body requires its complete C substitution, "
                             f"not assembly fallback: {owner.name}")
        matches.append({"owner": owner.name, "symbol": span[0]["symbol"],
                        "body_bytes": span[0]["body_bytes"]})
    return matches


def accepted_body_bytes() -> int:
    return sum(row["body_bytes"] for row in accepted_matches())


def run_tool(name: str, *arguments: str) -> None:
    script = ROOT / "tools" / name
    subprocess.run([sys.executable, str(script), *arguments], cwd=ROOT,
                   check=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("boot", type=Path)
    parser.add_argument("--cpp", type=Path, required=True)
    parser.add_argument("--cc1", type=Path, required=True)
    parser.add_argument("--as", dest="gnu_as", type=Path, required=True)
    parser.add_argument("--assembler", type=Path, required=True,
                        help="local SN PS2 EE assembler")
    parser.add_argument("--linker", type=Path, required=True,
                        help="local SN PS2 EE linker")
    parser.add_argument("--output-dir", type=Path, required=True,
                        help="native Linux directory for compiler intermediates")
    add_extra_arguments(parser)
    args = parser.parse_args()

    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({"gate": "boot_trial", "status": "incomplete",
                                  "counted_matches": 0}, indent=2) + "\n",
                      encoding="utf-8", newline="\n")
    compile_objects(args.cpp, args.cc1, args.gnu_as, args.output_dir, EXPORT,
                    dict(args.extra_bin), dict(args.extra_stage),
                    dict(args.as_bin))
    boot = args.boot.resolve()
    run_tool("boot_symbols.py", "code-intervals", str(boot))
    run_tool("boot_dvp.py", "catalogue", str(boot))
    run_tool("boot_c_campaign.py", str(boot), "--objects-dir", str(EXPORT),
             "--assembler", str(args.assembler.resolve()),
             "--linker", str(args.linker.resolve()))
    linked = ROOT / "build/boot-c-campaign/boot_multi.elf"
    run_tool("boot_witness_boundaries.py", str(boot), str(linked))
    matches = accepted_matches()
    accepted_bytes = sum(row["body_bytes"] for row in matches)
    run_tool("boot_elf_pack.py", "catalogue", str(boot))
    run_tool("boot_elf_pack.py", "build", str(boot), "--linked", str(linked))
    packed = ROOT / "build/boot-elf-pack/boot_exact.elf"

    reference = inspect_elf(boot)
    candidate = inspect_elf(linked)
    complete = inspect_elf(packed)
    program = next(item for item in reference["programs"] if item["type"] == 1)
    raw = linked.read_bytes()[program["file_offset"]:
                              program["file_offset"] + program["file_size"]]
    profile = json.loads(PROFILE.read_text(encoding="utf-8"))
    report = {
        "gate": "boot_trial",
        "status": "passed_boot_only",
        "profile": profile["id"],
        "boot_sha256": reference["sha256"],
        "candidate_objects": len(REPLACEMENTS),
        "candidate_functions": witness_count(),
        "promotion_input_sha256": input_fingerprint(),
        "entry": candidate["entry"],
        "raw_load_size": program["file_size"],
        "raw_load_sha256": hashlib.sha256(raw).hexdigest(),
        "raw_load_and_header_exact": candidate["programs"] == reference["programs"],
        "whole_elf_exact": complete["sha256"] == reference["sha256"],
        "whole_elf_size": complete["file_size"],
        "whole_elf_sha256": complete["sha256"],
        "counted_matches": len(matches),
        "counted_bytes": accepted_bytes,
        "accepted_functions": [row["symbol"] for row in matches],
        "note": "full ELF metadata was regenerated from the pinned numeric catalogue; non-code loaded data came from retail; original compiler identity and full gameplay remain unverified",
    }
    if not report["raw_load_and_header_exact"] or not report["whole_elf_exact"]:
        raise ValueError("gate report does not reflect the measured complete boot")
    REPORT.write_text(json.dumps(report, indent=2) + "\n",
                      encoding="utf-8", newline="\n")
    print(f"boot gate passed: {len(REPLACEMENTS)} exact objects, "
          f"{witness_count()} C witnesses, {program['file_size']} exact raw "
          f"load bytes and complete ELF exact; {len(matches)} boot-only matches "
          f"({accepted_bytes} bytes)")


if __name__ == "__main__":
    main()
